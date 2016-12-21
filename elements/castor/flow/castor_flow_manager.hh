#ifndef CLICK_CASTOR_FLOW_MANAGER_HH
#define CLICK_CASTOR_FLOW_MANAGER_HH

#include <click/element.hh>
#include "castor_merkle_flow.hh"
#include "castor_flow_table.hh"
#include "../node_id.hh"
#include "../crypto/crypto.hh"

CLICK_DECLS

class CastorFlowManager : public Element {
public:
	CastorFlowManager();

	const char *class_name() const { return "CastorFlowManager"; }
	const char *port_count() const { return PORTS_0_0; }
	const char *processing() const { return AGNOSTIC; }

	int configure(Vector<String>&, ErrorHandler*);

	PacketLabel getPacketLabel(NodeId, NodeId);
	const FlowId &getCurrentFlowId(NodeId, NodeId);
private:
	unsigned int _flowsize;
	CastorFlowTable* _flowtable;
	Crypto* _crypto;
	HashTable<NodeId, HashTable<NodeId, CastorMerkleFlow*> > _flows;
	CastorMerkleFlow* createFlowIfNotExists(NodeId src, NodeId dst);

	/**
	 * Defines the type of CastorFlow to be used.
	 */
	CastorMerkleFlow* createNewFlow(const NodeId& src, const NodeId& dst);
};

CLICK_ENDDECLS

#endif
