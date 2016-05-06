#ifndef CLICK_ADD_FORWARDER_LIST_HH
#define CLICK_ADD_FORWARDER_LIST_HH

#include <click/element.hh>

CLICK_DECLS

class AddForwarderList: public Element {
public:
	const char *class_name() const { return "AddForwarderList"; }
	const char *port_count() const { return PORTS_1_1; }
	const char *processing() const { return AGNOSTIC; }

	Packet* simple_action(Packet*);
};

CLICK_ENDDECLS

#endif
