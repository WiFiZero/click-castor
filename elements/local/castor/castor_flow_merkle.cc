#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include <click/straccum.hh>
#include "castor_flow_merkle.hh"
#include "crypto.hh"
#include "tree.hh"


CLICK_DECLS

bool CastorFlowMerkle::hasFlow(Host source, Host destination) {
	HashTable<Host, Flow> * t = _flows.get_pointer(source);
	if(!t) return false;

	Flow* f = t->get_pointer(destination);
	if(!f) return false;

	return true;
}

void CastorFlowMerkle::createFlow(Host source, Host destination) {
	//Create a new Flow Object
	click_chatter("Creating a new Flow Object with %d elements", CASTOR_REAL_FLOWSIZE);
	Flow flow;
	flow.position = 0;

	//Generate some random nonces
	Vector<SValue> ack_auths = Vector<SValue>(); // The ACK authenticator
	Vector<SValue> pids 	= Vector<SValue>();	// Packet IDs

	for(int f=0;f<CASTOR_REAL_FLOWSIZE;f++){
		SValue nonce 	= _crypto->random(sizeof(ACKAuth));
		SValue pid 		= _crypto->hash(nonce);
		ack_auths.push_back(nonce);
		pids.push_back(pid);
	}

	//Build the Merkle Tree
	MerkleTree tree = MerkleTree(pids, *_crypto);

	// Create labels
	for(unsigned int i=0;i<CASTOR_REAL_FLOWSIZE;i++){
		PacketLabel lbl;
		lbl.packet_number = i;

		// Set flow id
		SValue root = tree.getRoot();
		memcpy(&lbl.flow_id, root.begin(), sizeof(FlowId));

		// Set packet id
		memcpy(&lbl.packet_id, pids.at(i).begin(), sizeof(PacketId));

		// Set flow authenticator
		Vector<SValue> siblings;
		tree.getSiblings(siblings, i);
		assert(siblings.size() == CASTOR_FLOWSIZE);
		for(int j = 0; j < siblings.size(); j++)
			memcpy(&lbl.flow_auth[j].flow, siblings.at(j).begin(), sizeof(FlowAuth));

		click_chatter("Sibling ");

		// Set unencrypted (!) ACK authenticator
		memcpy(&lbl.ack_auth,ack_auths.at(i).begin(), sizeof(ACKAuth));
		flow.labels[i] = lbl;
	}

	//Check if a hashtable for the source exists
	if(!_flows.get_pointer(source))
		_flows.set(source, HashTable<Host, Flow>());

	HashTable<Host, Flow>* fdest = _flows.get_pointer(source);
	fdest->set(destination,flow);
}

PacketLabel CastorFlowMerkle::useFlow(Host source, Host destination) {
	Flow* f = (_flows.get_pointer(source))->get_pointer(destination);
	return f->labels[f->position];
}

void CastorFlowMerkle::updateFlow(Host source, Host destination) {
	Flow* f = (_flows.get_pointer(source))->get_pointer(destination);

	f->position++;

	if(f->position >= CASTOR_REAL_FLOWSIZE){
		(_flows.get_pointer(source))->erase(destination);
	}
}

CLICK_ENDDECLS
ELEMENT_REQUIRES(TREE)
EXPORT_ELEMENT(CastorFlowMerkle)



