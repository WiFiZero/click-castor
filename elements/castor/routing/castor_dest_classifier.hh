#ifndef CLICK_CASTOR_DEST_CLASSIFIER_HH
#define CLICK_CASTOR_DEST_CLASSIFIER_HH

#include <click/element.hh>
#include "../castor.hh"

CLICK_DECLS

class CastorDestClassifier: public Element {
public:
	const char *class_name() const { return "CastorDestClassifier"; }
	const char *port_count() const { return "1/2"; }
	const char *processing() const { return PUSH; }
	int configure(Vector<String>&, ErrorHandler*);

	void push(int, Packet *);
private:
	NodeId myId;
};

CLICK_ENDDECLS

#endif
