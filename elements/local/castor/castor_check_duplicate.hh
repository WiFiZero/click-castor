#ifndef CLICK_CASTOR_CHECK_DUPLICATE_HH
#define CLICK_CASTOR_CHECK_DUPLICATE_HH

#include <click/element.hh>
#include "castor_history.hh"

CLICK_DECLS

class CastorCheckDuplicate : public Element {
public:
	CastorCheckDuplicate() : history(NULL) {}

	const char *class_name() const	{ return "CastorCheckDuplicate"; }
	const char *port_count() const	{ return "1/4"; }
	const char *processing() const	{ return PUSH; }
	int configure(Vector<String>&, ErrorHandler*);
private:
	void push(int, Packet *);
	CastorHistory* history;
};

CLICK_ENDDECLS

#endif
