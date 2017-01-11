#include <click/config.h>
#include "castor_merkle_flow.hh"
#include "merkle_tree.hh"

CLICK_DECLS

CastorMerkleFlow::CastorMerkleFlow(size_t size, const NodeId& dst, CastorFlowTable* flowtable, const Crypto* crypto) : pos(0), flowtable(flowtable) {
	Hash *aauths = new Hash[size];
	Hash *pids = new Hash[size];
	Nonce n;
	crypto->random(n);
	Buffer<32> key(crypto->getSharedKey(dst)->data());
	// Generate aauths from n
	crypto->stream(aauths->data(), size * sizeof(Hash), n.data(), key.data());
	for (unsigned int i = 0; i < size; i++) {
		crypto->hash(pids[i], aauths[i]);
	}
	MerkleTree *tree = new MerkleTree(pids, size, *crypto);

	assert(!flowtable->has(tree->root()));

	// insert new flow table entry
	CastorFlowEntry& entry = flowtable->get(tree->root());
	entry.set_tree(tree);
	entry.n = n;
	entry.aauths = aauths;
	entry.pids = pids;

	fid = entry.fid();
}

PacketLabel CastorMerkleFlow::freshLabel() {
	assert(isAlive());
	CastorFlowEntry& entry = flowtable->get(fid);
	PacketLabel label(
			pos,
			entry.height(),
			entry.fid(),
			entry.pids[pos],
			entry.n
	);
	pos++;
	return label;
}

const FlowId &CastorMerkleFlow::getFlowId() const {
	return fid;
}

bool CastorMerkleFlow::isAlive() const {
	return flowtable->has(fid) &&
	       pos < flowtable->get(fid).size();
}

CLICK_ENDDECLS
ELEMENT_REQUIRES(MerkleTree)
ELEMENT_PROVIDES(CastorMerkleFlow)
