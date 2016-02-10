#ifndef CLICK_MERKLE_TREE_HH
#define CLICK_MERKLE_TREE_HH

#include <click/element.hh>
#include <click/vector.hh>
#include "../crypto/crypto.hh"

CLICK_DECLS

class MerkleTree {
public:
	/**
	 * Creates a Merkle tree from the given values.
	 * Size of input vector needs to be a power of 2.
	 * The leave values of this tree will be the hashed values of the input vector.
	 */
	MerkleTree(const Hash[], unsigned int length, const Crypto&);
	~MerkleTree();

	/**
	 * The the root of the Merke tree
	 */
	const Hash& getRoot() const;

	/**
	 * Retrieve all sibling values that are necessary to compute the root value from leaf i
	 */
	Vector<Hash> getSiblings(int i) const;

	/**
	 * Verifies that all elements (in, siblings, and root) belong to a valid Merkle tree, i.e. verifies that
	 *
	 *      hash( ... hash(hash(in)||siblings[0])||siblings[1] ...) == root
	 *
	 * i is needed to determine whether siblings[i] is right or left sibling
	 */
	static bool isValidMerkleTree(unsigned int i, const Hash& in, const Vector<Hash>& siblings, const Hash& root, const Crypto& crypto);

private:
	class Node;

	const Node* _root;
	Node** _leaves;

	/**
	 * Find base-2 logarithm of a power-2 integer efficiently.
	 *
	 * Source: https://graphics.stanford.edu/~seander/bithacks.html#IntegerLog
	 */
	inline unsigned int log2(unsigned int pow2) {
		static const unsigned int b[] = {0xAAAAAAAA, 0xCCCCCCCC, 0xF0F0F0F0,
		                                 0xFF00FF00, 0xFFFF0000};
		unsigned int r = (pow2 & b[0]) != 0;
		r |= ((pow2 & b[4]) != 0) << 4;
		r |= ((pow2 & b[3]) != 0) << 3;
		r |= ((pow2 & b[2]) != 0) << 2;
		r |= ((pow2 & b[1]) != 0) << 1;
		return r;
	}
};

CLICK_ENDDECLS

#endif
