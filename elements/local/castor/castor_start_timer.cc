#include <click/config.h>
#include <click/args.hh>
#include <click/straccum.hh>
#include "castor_start_timer.hh"
#include "castor_xcast.hh"

CLICK_DECLS

CastorStartTimer::PidTimer::PidTimer(CastorStartTimer *element, const PacketId pid, unsigned int timeout)
: Timer(element), pid(pid) {
	initialize(element);
	schedule_after_msec(timeout);
}

int CastorStartTimer::configure(Vector<String>& conf, ErrorHandler* errh) {
	return Args(conf, this, errh)
			.read_mp("RT", ElementCastArg("CastorRoutingTable"), table)
			.read_mp("TO", ElementCastArg("CastorTimeoutTable"), toTable)
			.read_mp("HISTORY", ElementCastArg("CastorHistory"), history)
			.read_mp("RATE_LIMITS", ElementCastArg("CastorRateLimitTable"), rate_limits)
			.read_mp("ID", myId)
			.read_or_set("VERBOSE", verbose, false)
			.complete();
}

void CastorStartTimer::push(int, Packet* p) {
	// Add timer
	if(CastorPacket::isXcast(p)) {
		CastorXcastPkt header = CastorXcastPkt(p);
		// Set timer for each destination individually
		for(unsigned int i = 0; i < header.ndst(); i++) {
			const PacketId pid = header.pid(i);
			unsigned int timeout = toTable->getTimeout(header.fid(), header.dst(i), history->routedTo(pid)).value();
			new PidTimer(this, pid, timeout);
		}
	} else {
		CastorPkt& header = (CastorPkt&) *p->data();
		unsigned int timeout = toTable->getTimeout(header.fid, header.dst, history->routedTo(header.pid)).value();
		new PidTimer(this, header.pid, timeout);
	}

	output(0).push(p);
}

void CastorStartTimer::run_timer(Timer* _timer) {
	PidTimer *timer = (CastorStartTimer::PidTimer *)_timer;

	const PacketId& pid = timer->getPid();
	if (!history->hasAck(pid)) {
		adjust_estimator(pid);
		adjust_rate_limit(pid);
	}
	history->remove(pid);

	delete timer;
}

void CastorStartTimer::adjust_estimator(const PacketId& pid) {
	NeighborId routedTo = history->routedTo(pid);

	// Check whether PKT was broadcast, if yes, do nothing as we don't know who might have received it
	if (routedTo == NeighborId::make_broadcast())
		return;

	const FlowId& fid = history->getFlowId(pid);
	NodeId destination = history->getDestination(pid);

	// decrease ratings
	if(verbose) {
		StringAccum sa;
		sa << "[" << Timestamp::now() << "@" << myId << "] Timeout: no ACK received from " << routedTo.unparse();
		click_chatter(sa.c_str());
	}
	CastorEstimator& estimator = table->getEstimator(fid, destination, routedTo);
	estimator.decreaseFrist();
	estimator.decreaseAll();
}

void CastorStartTimer::adjust_rate_limit(const PacketId& pid) {
	auto& senders = history->getPktSenders(pid);
	for (auto& sender : senders) {
		// TODO should we really check this here?
		if (sender != myId) {
			rate_limits->lookup(sender).decrease();
			rate_limits->notify(sender);
		}
	}
}

CLICK_ENDDECLS
EXPORT_ELEMENT(CastorStartTimer)
