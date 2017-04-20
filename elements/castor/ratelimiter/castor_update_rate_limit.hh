/*
 * Copyright (c) 2016 Milan Schmittner
 *
 * This file is part of click-castor.
 *
 * click-castor is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * click-castor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with click-castor.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CLICK_CASTOR_UPDATE_RATE_LIMIT_HH
#define CLICK_CASTOR_UPDATE_RATE_LIMIT_HH

#include <click/element.hh>
#include "castor_rate_limit_table.hh"
#include "../routing/castor_history.hh"

CLICK_DECLS

class CastorUpdateRateLimit: public Element {
public:
	const char *class_name() const { return "CastorUpdateRateLimit"; }
	const char *port_count() const { return "1/1"; }
	const char *processing() const { return AGNOSTIC; }
	int configure(Vector<String>&, ErrorHandler*);

	Packet* simple_action(Packet *);
private:
	bool enable;
	CastorRateLimitTable* rate_limits;
	CastorHistory* history;
};

CLICK_ENDDECLS

#endif
