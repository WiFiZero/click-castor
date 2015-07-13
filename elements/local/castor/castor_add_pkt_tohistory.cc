#include <click/config.h>
#include <click/confparse.hh>
#include "castor_add_pkt_tohistory.hh"
#include "castor.hh"

CLICK_DECLS

int CastorAddPktToHistory::configure(Vector<String> &conf, ErrorHandler *errh) {
	return cp_va_kparse(conf, this, errh,
			"CastorHistory", cpkP+cpkM, cpElementCast, "CastorHistory", &history,
			cpEnd);
}

void CastorAddPktToHistory::push(int, Packet *p){
	CastorPkt& pkt = (CastorPkt&) *p->data();

#ifdef CASTOR_CONTINUOUS_FLOW
	history->addPkt(pkt.pid, pkt.fid, pkt.nfauth, CastorPacket::src_ip_anno(p), p->dst_ip_anno(), pkt.dst, p->timestamp_anno());
#else
	history->addPkt(pkt.pid, pkt.fid, NextFlowAuth(), CastorPacket::src_ip_anno(p), p->dst_ip_anno(), pkt.dst, p->timestamp_anno());
#endif
	output(0).push(p);
}

CLICK_ENDDECLS
EXPORT_ELEMENT(CastorAddPktToHistory)
