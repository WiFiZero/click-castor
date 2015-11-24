#ifndef CLICK_CASTOR_HH
#define CLICK_CASTOR_HH

#include <click/packet.hh>
#include <click/packet_anno.hh>
#include "node_id.hh"
#include "hash.hh"

//#define DEBUG_ACK_SRCDST  // uncomment to add source and destination fields to ACK packets
#define DEBUG_HOPCOUNT // include (unprotected) hopcount field in packets

#define CASTOR_FLOWAUTH_ELEM                         8  // Number of flow auth elements

CLICK_DECLS

namespace CastorType { // C++11's strongly typed 'enum class' does not work, so create artificial namespace
	enum {
		PKT = 0xC0,
		ACK = 0xA0,

		DUMMY = 0x00,
		DUMMY_PKT = PKT | DUMMY,
		DUMMY_ACK = ACK | DUMMY,

		MERKLE = 0x0A,
		MERKLE_PKT = PKT | MERKLE,
		MERKLE_ACK = ACK | MERKLE,

		XCAST = 0x0C,
		XCAST_PKT = PKT | XCAST,
	};
};

typedef Hash FlowId;
typedef Hash PacketId;
typedef Hash FlowAuth[CASTOR_FLOWAUTH_ELEM];
typedef Hash AckAuth;
typedef Hash PktAuth;

/**
 * The Castor data packet header (PKT)
 */
class CastorPkt {
public:
	uint8_t 	type;
	uint8_t 	hsize;
	uint16_t 	len;
	uint8_t 	fsize;
	uint8_t 	ctype;
	uint16_t	kpkt; // the k-th packet of the current flow, necessary for flow validation (determines whether fauth[i] is left or right sibling in the Merkle tree)
	NodeId		src;
	NodeId		dst;
	NodeId		next_hop;
	FlowId	 	fid;
	PacketId 	pid;
	FlowAuth 	fauth;
	PktAuth 	pauth;
#ifdef DEBUG_HOPCOUNT
	uint8_t		hopcount;
#endif
};

/**
 * The Castor acknowledgement packet (ACK)
 */
class CastorAck {
public:
	uint8_t 	type;
	uint8_t 	hsize;
	uint16_t 	len;
#ifdef DEBUG_ACK_SRCDST
	NodeId		src;
	NodeId		dst;
#endif
	NodeId		next_hop;
	AckAuth 	auth;
};

/**
 * Utility class to handle packet types and annotations
 */
class CastorPacket {
private:
	template<typename T>
	static inline T& anno(Packet* p, uint8_t offset) {
		auto* anno_ptr = p->anno_u8();
		anno_ptr += offset;
		return reinterpret_cast<T&>(*anno_ptr);
	}

public:

	static inline uint8_t getType(const Packet* p) {
		uint8_t type = p->data()[0] & 0xF0;
		return type;
	}

	static inline NodeId& dst_id_anno(Packet* p) {
		return anno<NodeId>(p, dst_id_anno_offset);
	}

	static inline NodeId& src_id_anno(Packet* p) {
		return anno<NodeId>(p, src_id_anno_offset);
	}

	static inline void set_src_id_anno(Packet* p, const NodeId& node) {
		auto* anno_ptr = p->anno_u8();
		anno_ptr += src_id_anno_offset;
		memcpy(anno_ptr, node.data(), sizeof(NodeId));
	}

	static inline NodeId& hop_id_anno(Packet* p) {
		return anno<NodeId>(p, hop_id_anno_offset);
	}

	/**
	 * User annotation space for Castor
	 */
	static inline Hash& getCastorAnno(Packet* p) {
		uint8_t* cAnno = p->anno_u8();
		cAnno += castor_anno_offset;
		return (Hash&) *cAnno;
	}

	static inline bool isXcast(Packet* p) {
		uint8_t type = p->data()[0] & 0x0F;
		return (type == CastorType::XCAST);
	}

private:
	/** Position of annotation fields, we have a maximum of 48 bytes (!) */
	static const uint8_t dst_id_anno_offset = 0;
	static const uint8_t src_id_anno_offset = dst_id_anno_offset + sizeof(NodeId);
	static const uint8_t hop_id_anno_offset = src_id_anno_offset + sizeof(NodeId);
	static const uint8_t castor_anno_offset = hop_id_anno_offset + sizeof(NodeId);
	static const uint8_t castor_paint_offset = castor_anno_offset + sizeof(Hash);
};

CLICK_ENDDECLS
#endif
