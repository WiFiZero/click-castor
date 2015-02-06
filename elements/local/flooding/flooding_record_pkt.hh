#ifndef CLICK_FLOODING_RECORD_PKT_HH
#define CLICK_FLOODING_RECORD_PKT_HH

#include <click/element.hh>
#include <click/timestamp.hh>
#include <click/vector.hh>
#include <click/list.hh>
#include "../castor/castor.hh"
#include "../castor/castor_xcast_destination_map.hh"

CLICK_DECLS

class FloodingRecordPkt: public Element {
public:
	FloodingRecordPkt() : map(0) {
		seq_index = 0, numPids = 0, numPkts = 0, pktAccumSize = 0, broadcastDecisions = 0;
	}
		
	const char *class_name() const { return "FloodingRecordPkt"; }
	const char *port_count() const { return PORTS_1_1; }
	const char *processing() const { return PUSH; }
	int configure(Vector<String>&, ErrorHandler*);

	void push(int, Packet *);

    void add_handlers();
private:
	class Entry {
	public:
		Entry(unsigned long id, const Timestamp time = Timestamp::now()) :
			time(time) {
			this->id = id;
		};
		unsigned long id;
		const Timestamp time;
	};

	struct ListNode {
		inline ListNode() { value = 0; }
		inline ListNode(uint32_t value) { this->value = value; }
		List_member<ListNode> node;
		atomic_uint32_t value;
	};

	List<ListNode, &ListNode::node> hopcounts;

	Vector<Entry> records;
	int32_t seq_index;

	atomic_uint32_t numPids;
	atomic_uint32_t numPkts;
	atomic_uint32_t pktAccumSize;

	atomic_uint32_t broadcastDecisions;

	static String read_handler(Element*, void*);

	struct Statistics {
		enum {
			num, // number of pids recorded (same as 'numUnique' for unicast routing)
			numUnique, // number of PKTs recorded ('numUnique' <= 'num' for Xcast routing)
			size, // accumulated size of all packets (only unique PKTs counted)
			time, // Time @ which pids were received
			broadcasts, // number of broadcasts
			unicasts, // number of unicasts
			seq_entry, // returns entries one after the other
			seq_hopcount, // hopcount of the packet
		};
	};

	CastorXcastDestinationMap* map;
};

CLICK_ENDDECLS
#endif
