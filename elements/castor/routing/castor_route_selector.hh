#ifndef CLICK_CASTOR_ROUTE_SELECTOR_HH
#define CLICK_CASTOR_ROUTE_SELECTOR_HH

#include <click/element.hh>
#include "../castor.hh"
#include "../neighbor_id.hh"

CLICK_DECLS

class CastorRouteSelector: public Element {
public:
	virtual const char *class_name() const = 0;
	virtual const char *port_count() const { return PORTS_0_0; }
	virtual const char *processing() const { return AGNOSTIC; }

	/**
	 * Select the best next hop for a given flow/subflow
	 */
	virtual NeighborId select(const Hash& flow, const NodeId& src, const NodeId& dst) = 0;
};

CLICK_ENDDECLS

#endif
