#ifndef CLICK_CASTOR_START_TIMER_HH
#define CLICK_CASTOR_START_TIMER_HH

#include <click/element.hh>
#include <click/timer.hh>
#include "castor.hh"
#include "castor_routing_table.hh"
#include "castor_timeout_table.hh"
#include "castor_history.hh"

CLICK_DECLS

class CastorStartTimer : public Element {
public:
	CastorStartTimer() : table(NULL), toTable(NULL), history(NULL), verbose(false) {};

	const char *class_name() const { return "CastorStartTimer"; }
	const char *port_count() const { return PORTS_1_1; }
	const char *processing() const { return PUSH; }
	int configure(Vector<String>&, ErrorHandler*);

	void push(int, Packet *);

private:
	void run_timer(Timer*);

	class PidTimer : public Timer {
	public:
		PidTimer(CastorStartTimer *element, const PacketId pid, unsigned int timeout);
		inline const PacketId &getPid() const { return pid; }
	private:
		const PacketId pid;
	};

	void adjust_rating(const PacketId&);

	CastorRoutingTable* table;
	CastorTimeoutTable* toTable;
	CastorHistory* history;
	NodeId myId;

	bool verbose;
};

CLICK_ENDDECLS

#endif
