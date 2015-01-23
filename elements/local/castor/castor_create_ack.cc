#include <click/config.h>
#include <click/confparse.hh>
#include <click/packet.hh>
#include <clicknet/ether.h>
#include "castor_create_ack.hh"

CLICK_DECLS

void CastorCreateAck::push(int, Packet* p) {

	Castor_ACK ack;
	ack.type = CastorType::MERKLE_ACK;
	ack.hsize = sizeof(Hash);
	ack.len = sizeof(Castor_ACK);
	ack.auth = Hash(CastorPacket::getCastorAnno(p));

	WritablePacket* q = Packet::make(sizeof(click_ether) + sizeof(click_ip), &ack, sizeof(Castor_ACK), 0);
	q->set_dst_ip_anno(CastorPacket::src_ip_anno(p)); // Set DST_ANNO to source of PKT

	output(0).push(p); // PKT -> output 0
	output(1).push(q); // ACK -> output 1

}

CLICK_ENDDECLS
EXPORT_ELEMENT(CastorCreateAck)
