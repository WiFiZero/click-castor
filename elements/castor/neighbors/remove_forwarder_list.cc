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

#include <click/config.h>
#include "remove_forwarder_list.hh"
#include "forwarder_list.hh"

CLICK_DECLS

Packet* RemoveForwarderList::simple_action(Packet* p) {
	p->pull(sizeof(ForwarderList));
	return p;
}

CLICK_ENDDECLS
EXPORT_ELEMENT(RemoveForwarderList)
