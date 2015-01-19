#include <click/config.h>
#include <click/confparse.hh>
#include "castor_timeout.hh"
#include "castor_xcast.hh"

CLICK_DECLS

CastorTimeout::CastorTimeout() {
	timers = HashTable<Timer*,Entry>();
}

CastorTimeout::~CastorTimeout() {
}

int CastorTimeout::configure(Vector<String>& conf, ErrorHandler* errh) {
	return cp_va_kparse(conf, this, errh,
			"CastorRoutingTable", cpkP + cpkM, cpElementCast, "CastorRoutingTable", &table,
			"CastorHistory", cpkP + cpkM, cpElementCast, "CastorHistory", &history,
			"TIMEOUT", cpkP + cpkM, cpInteger, &timeout,
			"IP", cpkP + cpkM, cpIPAddress, &myIP,
			"VERBOSE", cpkP + cpkM, cpBool, &verbose,
			cpEnd);
}

void CastorTimeout::push(int, Packet* p) {
	// Add timer
	if(CastorPacket::isXcast(p)) {
		CastorXcastPkt header = CastorXcastPkt(p);
		// Set timer for each destination individually
		for(unsigned int i = 0; i < header.getNDestinations(); i++) {
			Timer* timer = new Timer(this);
			timer->initialize(this);
			timer->schedule_after_msec(timeout);

			Entry entry;
			entry.pid = header.getPid(i);
			timers.set(timer, entry);
		}
	} else {
		Castor_PKT& header = (Castor_PKT&) *p->data();

		Timer* timer = new Timer(this);
		timer->initialize(this);
		timer->schedule_after_msec(timeout);

		Entry entry;
		entry.pid = header.pid;
		timers.set(timer, entry);
	}

	output(0).push(p);
}

void CastorTimeout::run_timer(Timer* timer) {

	Entry* entry = timers.get_pointer(timer);
	if(!entry) {
		// This should not happen!
		StringAccum sa;
		sa << "[" << Timestamp::now() << "@" << myIP << "] Error: Unknown timer fired";
		click_chatter(sa.c_str());
		// delete timer
		timers.erase(timer);
		delete timer;
		return;
	}

	PacketId& pid = entry->pid;

	// Check whether ACK has been received in the meantime
	if (history->hasAck(pid)) {
		// delete timer
		timers.erase(timer);
		delete timer;
		return;
	}

	history->setExpired(pid);
	IPAddress routedTo = history->routedTo(pid);

	// Check whether PKT was broadcast, if yes, do nothing
	// XXX: Why is that?
	if (routedTo == IPAddress::make_broadcast()) {
		// delete timer
		timers.erase(timer);
		delete timer;
		return;
	}

	const FlowId& fid = history->getFlowId(pid);
	IPAddress destination = history->getDestination(pid);

	// decrease ratings
	if(verbose) {
		StringAccum sa;
		sa << "[" << Timestamp::now() << "@" << myIP << "] Timeout: no ACK received from " << routedTo.unparse();
		click_chatter(sa.c_str());
	}
	table->updateEstimates(fid, destination, routedTo, CastorRoutingTable::decrease, CastorRoutingTable::first);
	table->updateEstimates(fid, destination, routedTo, CastorRoutingTable::decrease, CastorRoutingTable::all);

	// delete timer
	timers.erase(timer);
	delete timer;

}

CLICK_ENDDECLS
EXPORT_ELEMENT(CastorTimeout)
