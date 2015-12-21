#include <click/config.h>
#include <click/args.hh>
#include "castor_xcast_retransmit_ack.hh"
#include "castor_xcast.hh"
#include "castor_anno.hh"

CLICK_DECLS

int CastorXcastRetransmitAck::configure(Vector<String> &conf, ErrorHandler *errh) {
	return Args(conf, this, errh)
			.read_mp("History", ElementCastArg("CastorHistory"), history)
			.complete();
}

void CastorXcastRetransmitAck::push(int, Packet *p) {
	// We already forwarded a valid ACK for this PKT, so retransmit
	CastorXcastPkt pkt = CastorXcastPkt(p);

	p = pkt.getPacket();

	for (uint8_t i = 0; i < pkt.ndst(); i++) {
		assert(history->hasAck(pkt.pid(i)));

		// Generate new ACK
		CastorAck ack;
		ack.type = CastorType::MERKLE_ACK;
		ack.hsize = sizeof(PktAuth);
		ack.len = sizeof(CastorAck);
		ack.auth = history->getAckAuth(pkt.pid(i));
#ifdef DEBUG_ACK_SRCDST
		ack.src = pkt.getSource();
		ack.dst = pkt.getDestination(i);
#endif

		WritablePacket* q = Packet::make(&ack, sizeof(CastorAck));
		CastorAnno::dst_id_anno(q) = CastorAnno::src_id_anno(p); // Unicast ACK to PKT sender
		CastorAnno::hop_id_anno(q) = CastorAnno::dst_id_anno(q);

		assert(history->hasPktFrom(pkt.pid(i), CastorAnno::dst_id_anno(q)));

		output(0).push(q);
	}

	// No longer need PKT
	p->kill();
}

CLICK_ENDDECLS
EXPORT_ELEMENT(CastorXcastRetransmitAck)
