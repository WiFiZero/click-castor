#ifndef CLICK_CASTOR_ADD_PKT_TO_HISTORY_HH
#define CLICK_CASTOR_ADD_PKT_TO_HISTORY_HH

#include <click/element.hh>
#include "castor_history.hh"

CLICK_DECLS

class CastorAddPktToHistory: public Element {
public:
	const char *class_name() const { return "CastorAddPktToHistory"; }
	const char *port_count() const { return PORTS_1_1; }
	const char *processing() const { return AGNOSTIC; }
	int configure(Vector<String>&, ErrorHandler*);

	Packet* simple_action(Packet*);
private:
	CastorHistory* history;
};

CLICK_ENDDECLS

#endif
