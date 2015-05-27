#ifndef CLICK_CASTOR_ADD_HEADER_HH
#define CLICK_CASTOR_ADD_HEADER_HH

#include <click/element.hh>
#include "castor.hh"
#include "castor_flow_stub.hh"

CLICK_DECLS

/**
 * Add Castor header to IP packet
 * TODO: Should replace IP header instead of add
 */
class CastorAddHeader: public Element {
public:
	CastorAddHeader() : flow(NULL) {}
		
	const char *class_name() const { return "CastorAddHeader"; }
	const char *port_count() const { return PORTS_1_1; }
	const char *processing() const { return PUSH; }
	int configure(Vector<String>&, ErrorHandler*);

	void push(int, Packet*);
private:
	CastorFlowStub* flow;
};

CLICK_ENDDECLS

#endif
