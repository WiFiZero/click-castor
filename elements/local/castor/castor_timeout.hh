#ifndef CLICK_CASTOR_TIMEOUT_HH
#define CLICK_CASTOR_TIMEOUT_HH

#include <click/element.hh>
#include <click/hashtable.hh>
#include <click/timer.hh>
#include "castor.hh"
#include "castor_routingtable.hh"
#include "castor_history.hh"

CLICK_DECLS

// TODO Could be merged with HistoryEntry, but would require exposure of this data type
typedef struct {
	FlowId fid;
	PacketId pid;
	IPAddress routedTo;
} TimeoutEntry;

// FIXME: Make this an Element to push through, i.e. timer is started when packet is pushed through
class CastorTimeout: public Element {

public:

	CastorTimeout();
	~CastorTimeout();

	const char *class_name() const { return "CastorTimeout"; }
	const char *port_count() const { return PORTS_0_0; }
	const char *processing() const { return AGNOSTIC; }
	int configure(Vector<String>&, ErrorHandler*);
	void create_timer(Packet*);
	void run_timer(Timer*);

private:
	CastorRoutingTable* table;
	CastorHistory* history;
	HashTable<Timer*,TimeoutEntry> timers;
	int timeout;

};

CLICK_ENDDECLS
#endif /* CASTOR_TIMEOUT_HH_ */
