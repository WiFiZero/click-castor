#include <click/config.h>
#include <click/args.hh>
#include <click/confparse.hh>
#include <click/straccum.hh>
#include "castor_print.hh"
#include "castor_xcast.hh"
#include <click/timestamp.hh>

CLICK_DECLS

int CastorPrint::configure(Vector<String> &conf, ErrorHandler *errh) {
	if (Args(conf, this, errh)
		.read_mp("LABEL", label)
		.read_mp("NodeId", myId)
		.read_p("VERBOSE", verbose)
		.complete() < 0)
			return -1;
	return 0;
}

void CastorPrint::push(int, Packet *p){

	StringAccum sa;
	sa << "[" << Timestamp::now() << "@" << myId << "] " << label << " ";

	uint8_t type = CastorPacket::getType(p);

	if (type == CastorType::PKT) {

		if (CastorPacket::isXcast(p)) {
			sa << (verbose ? "\n" : "") << CastorXcastPkt(p).toString(verbose).c_str();
		} else {
			CastorPkt& pkt = (CastorPkt&) *p->data();
			if(verbose) {
				sa << "\n";
				sa << "   | From: \t" << CastorPacket::src_id_anno(p) << "\n";
				sa << "   | To: \t" << p->dst_ip_anno() << "\n";
				sa << "   | Type: \tPKT (header " <<  ntohs(pkt.len) << " / payload " << (p->length() - ntohs(pkt.len)) << " bytes)\n";
				sa << "   | Flow: \t" << pkt.src << " -> " << pkt.dst << "\n";
				sa << "   | Flow ID: \t" << pkt.fid.str() << "\n";
				sa << "   | PKT ID: \t" << pkt.pid.str() << " (" << (ntohs(pkt.kpkt) + 1) << "/" << (1 << pkt.fsize) << ")\n";
				sa << "   | PKT Auth: \t" << pkt.pauth.str();
			} else {
				sa << "PKT (from " << CastorPacket::src_id_anno(p) << " to " << p->dst_ip_anno() << ", flow " << pkt.src << " -> " << pkt.dst << "): " << pkt.pid.str();
			}
		}

	} else if (type == CastorType::ACK){

		// CastorAck and CastorXcastAck are identical
		CastorAck& ack = (CastorAck&) *p->data();
		if(verbose) {
			sa << "\n";
			sa << "   | From: \t" << CastorPacket::src_id_anno(p) << "\n";
			sa << "   | To: \t" << p->dst_ip_anno() << "\n";
			sa << "   | Type: \tACK  (" <<  ack.len << " bytes)\n";
			sa << "   | ACK Auth: \t" << ack.auth.str() << "\n";
#ifdef DEBUG_ACK_SRCDST
			sa << "   | Flow: \t" << ack.src << " -> " << ack.dst << "\n";
#endif
		} else {
			sa << "ACK (from " << CastorPacket::src_id_anno(p) << " to " << p->dst_ip_anno() << "): " << ack.auth.str();
		}

	} else {
		sa << "UNKNOWN (from " << CastorPacket::src_id_anno(p) << " to " << p->dst_ip_anno() << ")";
	}

	click_chatter("%s", sa.c_str());

    output(0).push(p);
}

CLICK_ENDDECLS
EXPORT_ELEMENT(CastorPrint)
