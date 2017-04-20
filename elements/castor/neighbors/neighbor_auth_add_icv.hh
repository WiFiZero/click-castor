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

#ifndef CLICK_NEIGHBOR_AUTH_ADD_ICV_HH
#define CLICK_NEIGHBOR_AUTH_ADD_ICV_HH

#include <click/element.hh>
#include "neighbors.hh"
#include "../crypto/crypto.hh"

CLICK_DECLS

class NeighborAuthAddICV : public Element {
public:
	const char *class_name() const { return "NeighborAuthAddICV"; }
	const char *port_count() const { return PORTS_1_1X2; }
	const char *processing() const { return AGNOSTIC; }
	int configure(Vector<String>&, ErrorHandler*);

	Packet* simple_action(Packet *);
private:
	Neighbors* neighbors;
	Crypto* crypto;
	bool enable;
};

CLICK_ENDDECLS

#endif
