#ifndef CLICK_CASTOR_XCAST_TO_UNICAST_HH
#define CLICK_CASTOR_XCAST_TO_UNICAST_HH

#include <click/element.hh>
#include "castor_xcast_destination_map.hh"

CLICK_DECLS

class CastorXcastToUnicast: public Element {
public:
	const char *class_name() const { return "CastorXcastToUnicast"; }
	const char *port_count() const { return "1/2"; }
	const char *processing() const { return PUSH; }
	int configure(Vector<String>&, ErrorHandler*);

	void push(int, Packet*);
private:
	CastorXcastDestinationMap* map;
};

CLICK_ENDDECLS

#endif
