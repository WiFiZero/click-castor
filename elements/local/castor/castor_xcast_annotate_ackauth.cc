#include <click/config.h>
#include <click/args.hh>
#include "castor_xcast_annotate_ackauth.hh"
#include "castor_xcast.hh"
#include "castor_anno.hh"

CLICK_DECLS

int CastorXcastAnnotateAckAuth::configure(Vector<String>& conf, ErrorHandler* errh) {
	return Args(conf, this, errh)
			.read_mp("Crypto", ElementCastArg("Crypto"), crypto)
			.complete();
}

void CastorXcastAnnotateAckAuth::push(int, Packet *p) {

	CastorXcastPkt pkt = CastorXcastPkt(p->clone());

	SValue pktAuth = crypto->convert(pkt.pkt_auth());

	// Get appropriate key and decrypt encrypted ACK authenticator
	const SymmetricKey* sk = crypto->getSharedKey(pkt.src());
	if (!sk) {
		click_chatter("Could not find shared key for host %s. Discarding PKT...", pkt.dst(0).unparse().c_str());
		pkt.getPacket()->kill();
		return;
	}
	SValue ackAuth = crypto->encrypt(pktAuth, *sk);
	if (ackAuth.size() != sizeof(PktAuth)) {
		click_chatter("Cannot create ciphertext: Crypto subsystem returned wrong plaintext length. Discarding PKT...");
		pkt.getPacket()->kill();
		return;
	}

	AckAuth& authAnno = CastorAnno::hash_anno(pkt.getPacket());
	authAnno = crypto->convert(ackAuth);

	output(0).push(pkt.getPacket());

}

CLICK_ENDDECLS
EXPORT_ELEMENT(CastorXcastAnnotateAckAuth)

