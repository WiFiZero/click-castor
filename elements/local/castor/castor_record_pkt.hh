#ifndef CLICK_CASTOR_RECORD_PKT_HH
#define CLICK_CASTOR_RECORD_PKT_HH

#include <click/element.hh>
#include <click/timestamp.hh>
#include <click/list.hh>
#include "castor.hh"

CLICK_DECLS

class CastorRecordPkt: public Element {
public:
	CastorRecordPkt() : npackets(0), npids(0), size(0), nbroadcasts(0), nunicasts(0) {}
		
	const char *class_name() const { return "CastorRecordPkt"; }
	const char *port_count() const { return PORTS_1_1; }
	const char *processing() const { return PUSH; }

	virtual void push(int, Packet *);

    void add_handlers();
protected:
	struct PidTime {
		PidTime(const PacketId& pid, const Timestamp time = Timestamp::now_steady()) : pid(pid), time(time) {};
		List_member<PidTime> node;
		const PacketId pid;
		const Timestamp time;
	};

	struct UintListNode {
		inline UintListNode(size_t value) : value(value) {}
		List_member<UintListNode> node;
		size_t value;
	};

	List<PidTime, &PidTime::node> records;
	List<UintListNode, &UintListNode::node> hopcounts;
	size_t npackets;
	size_t npids;
	size_t size;
	size_t nbroadcasts;
	size_t nunicasts;

private:
	static String read_handler(Element*, void*);

	struct Statistics {
		enum {
			/* general */
			npackets, // number of packets recorded
			size, // accumulated size of all packets
			/* Castor/Xcastor PKT specific */
			npids, // number of pids recorded (is the same as 'npkts' for Castor)
			nbroadcasts, // number of recorded broadcast pids
			nunicasts, // number of recorded unicast pids
			seq_entry, // read timestamp-pid pairs sequentially
			seq_hopcount, // read individual hop counts sequentially
		};
	};

	static size_t readAndReset(size_t& value) {
		size_t tmp = value;
		value = 0;
		return tmp;
	}

};

CLICK_ENDDECLS
#endif
