#ifndef CLICK_CASTOR_UPDATE_ESTIMATES_HH
#define CLICK_CASTOR_UPDATE_ESTIMATES_HH

#include <click/element.hh>
#include "castor.hh"
#include "crypto.hh"
#include "castor_routingtable.hh"
#include "castor_history.hh"

CLICK_DECLS

class CastorUpdateEstimates: public Element {
public:
	CastorUpdateEstimates();

	const char *class_name() const { return "CastorUpdateEstimates"; }
	const char *port_count() const { return "1/3"; }
	const char *processing() const { return PUSH; }
	int configure(Vector<String>&, ErrorHandler*);

	void push(int, Packet *);
private:
	Crypto* crypto;
	CastorRoutingTable* table;
	CastorHistory* history;
};

CLICK_ENDDECLS

#endif
