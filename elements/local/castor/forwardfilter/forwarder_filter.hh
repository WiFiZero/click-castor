#ifndef CLICK_FORWARDER_FILTER_HH
#define CLICK_FORWARDER_FILTER_HH

#include <click/element.hh>
#include "../../neighbordiscovery/neighbor_id.hh"

CLICK_DECLS

class ForwarderFilter: public Element {
public:
	const char *class_name() const { return "ForwarderFilter"; }
	const char *port_count() const { return "1/1-2"; }
	const char *processing() const { return "a/ah"; }
	int configure(Vector<String>&, ErrorHandler*);

	Packet* simple_action(Packet *);
private:
	NeighborId my_id;
};

CLICK_ENDDECLS

#endif
