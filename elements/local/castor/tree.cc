#include <click/config.h>
#include <click/straccum.hh>
#include "tree.hh"

CLICK_DECLS

class MerkleTree::Node {
public:
	Node(Node* parent = 0, Node* leftChild = 0,	Node* rightChild = 0) :
			parent(parent), leftChild(leftChild), rightChild(rightChild) {}
	~Node() { delete leftChild;	delete rightChild; }

	bool isRoot() { return parent == 0; }
	bool isLeaf() { return leftChild == 0 && rightChild == 0; }
	bool isLeftChild() { return !isRoot() && (parent->leftChild == this); }
	bool isRightChild() { return !isRoot() && (parent->rightChild == this); }
	Node* getSibling() {
		if(isLeftChild())
			return parent->rightChild;
		else if(isRightChild())
			return parent->leftChild;
		else
			return 0;
	}

	Node* parent;
	Node* leftChild;
	Node* rightChild;
	SValue data;
};

MerkleTree::MerkleTree(Vector<SValue>& in, Crypto& c) :	crypto(c) {

	// Verify that size of e is power of 2
	assert(in.size() && !(in.size() & (in.size() - 1)));

	_leaves = Vector<Node*>();
	Vector<Node*>* layer = 0;
	Vector<Node*>* nextlayer = new Vector<Node*>();

	// Create the leaves
	for (int i = 0; i < in.size(); i++) {
		Node* hnode = new Node();
		hnode->data = crypto.hash(in.at(i));
		_leaves.push_back(hnode);
		nextlayer->push_back(hnode);
	}

	// Build the tree
	while (nextlayer->size() > 1) {
		delete layer;
		layer = nextlayer;
		nextlayer = new Vector<Node*>();

		for (int j = 0; j < layer->size(); j += 2) {
			Node* lc = layer->at(j);
			Node* rc = layer->at(j + 1);

			// Create a new node
			Node* n = new Node(0, lc, rc);

			// Update lower layer nodes
			lc->parent = n;
			rc->parent = n;

			// Compute hash
			SValue tmp = SValue(lc->data);
			for (unsigned int k = 0; k < rc->data.size(); k++)
				tmp.push_back(rc->data.begin()[k]);
			n->data = crypto.hash(tmp);

			nextlayer->push_back(n);
		}
	}
	_root = nextlayer->at(0);
	delete nextlayer;
}

MerkleTree::~MerkleTree(){
	// Node deletes children recursively
	delete _root;
}

SValue MerkleTree::getRoot(){
	return _root->data;
}

void MerkleTree::getLeaves(Vector<SValue>& leaves){
	for(int i=0; i<_leaves.size(); i++)
		leaves.push_back(_leaves.at(i)->data);
}

void MerkleTree::getSiblings(Vector<SValue>& siblings, int id) {
	Node* node = _leaves.at(id);
	while(!node->isRoot()) {
		siblings.push_back(node->getSibling()->data);
		node = node->parent;
	}
}

String MerkleTree::toString() {
	StringAccum sa;
	sa << "Generated Merkle Tree:\n";
	for (int i = 0; i < _leaves.size(); i++) {
		Node* n = _leaves.at(i);
		do {
			sa << CastorPacket::hexToString(n->data.begin(), n->data.size()) << "->";
			n = n->parent;
		} while (n != 0);
		sa << "\n";
	}
	return sa.take_string();
}

bool MerkleTree::isValidMerkleTree(unsigned int id, const SValue& in, const Vector<SValue>& siblings, const SValue& root, const Crypto& crypto) {
	// First hash the input
	SValue current = crypto.hash(in);

	for(int s = 0; s < siblings.size(); s++) {
		SValue sibling = siblings[s];
		// Decide whether current sibling is left or right child
		if(id & (1 << s)) { // sibling is left child
			// current = hash(sibling||current)
			for(unsigned int i = 0; i < current.size(); i++)
				sibling.push_back(current[i]);
			current = crypto.hash(sibling);
		} else { // sibling is right child
			// current = hash(current||sibling)
			for(unsigned int i = 0; i < sibling.size(); i++)
				current.push_back(sibling[i]);
			current = crypto.hash(current);
		}
	}

	// verify that computed root and given root are the same
	int cmp = memcmp(current.begin(), root.begin(), root.size());
	return cmp == 0;
}

CLICK_ENDDECLS
ELEMENT_PROVIDES(TREE)
