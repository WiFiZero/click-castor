#ifndef CLICK_CASTOR_ANNOTATE_PID_HH
#define CLICK_CASTOR_ANNOTATE_PID_HH

#include <click/element.hh>
#include "castor.hh"
#include "crypto.hh"

CLICK_DECLS

class CastorAnnotatePid: public Element {
public:
	CastorAnnotatePid() : crypto(NULL) {}

	const char *class_name() const { return "CastorAnnotatePid"; }
	const char *port_count() const { return PORTS_1_1; }
	const char *processing() const { return PUSH; }
	int configure(Vector<String>&, ErrorHandler*);

	void push(int, Packet *);
private:
	Crypto* crypto;
};

CLICK_ENDDECLS

#endif
