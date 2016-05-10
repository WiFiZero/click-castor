#ifndef CLICK_REPLAY_STORE_HH
#define CLICK_REPLAY_STORE_HH

#include <click/element.hh>
#include <click/hashtable.hh>
#include <click/timer.hh>
#include "../castor.hh"

CLICK_DECLS

class ReplayStore : public Element {
public:
	const char *class_name() const { return "ReplayStore"; }
	const char *port_count() const { return "0/2"; }
	const char *processing() const { return PUSH; }
	int configure(Vector<String>&, ErrorHandler*);

	void add_pkt(const Hash& id, Packet* p);
	void add_ack(const Hash& id, Packet* p);

	void add_handlers();
private:
	class ReplayTimer : public Timer {
	public:
		ReplayTimer() : Timer(), pkt(NULL), ack(NULL), replays_left(0), timeout(0), jitter(0) {}
		ReplayTimer(ReplayStore *element, const Hash& id, Packet* p, unsigned int replays, unsigned int timeout, unsigned int jitter = 0) : Timer(element), id(id), pkt(p), ack(NULL), replays_left(replays), timeout(timeout), jitter(jitter) {
			initialize(element);
			schedule_after_msec(interval());
		}
		inline unsigned int interval() {
			return timeout + randsign() * randjitter();
		}
		Hash id;
		Packet* pkt;
		Packet* ack;
		unsigned replays_left;
	private:
		unsigned int timeout;
		unsigned int jitter;
		inline int randjitter() {
			return (double) click_random() / CLICK_RAND_MAX * jitter;
		}
		inline int randsign() {
			return (click_random() > CLICK_RAND_MAX / 2) ? 1 : -1;
		}
	};

	HashTable<Hash, ReplayTimer*> table;

	void run_timer(Timer*);

	unsigned int interval; // After which time (in ms) to issue the retransmission
	unsigned int jitter;
	unsigned int replays_max;
	bool enable;

	static int write_handler(const String &, Element *, void *, ErrorHandler *);
};

CLICK_ENDDECLS
#endif
