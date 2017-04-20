/*
 * Copyright (c) 2016 Simon Schmitt
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

#ifndef CLI_HH
#define CLI_HH
#include <string>
#include <arpa/inet.h>

/*
 * The ttl is limited to 15, because the packet 
 * holds only 4 bits for the ttl.
 */
#define MAX_TTL 0xf

#define DEFAULT_IFA_NAME "wlan0"

/*
 * Defines which addresses are to be shown.
 */
enum AddressType { AT_IP, AT_MAC, AT_MAC_IP };

/*
 * Defines which routes are to be shown.
 */
enum RouteType { RT_DST, RT_NODST, RT_ALL }; 

/*
 * Defines how the routes are to be sorted.
 */
enum SortType { ST_NORMAL, ST_UP, ST_DOWN };

class CLI {
public:
	CLI() { };
	~CLI() { };

	// Parses all arguments and prints a help-message if an error occurs
	bool parse_args(int argc, char** argv);

 	// Finds the own ip address to a given interface name
	bool set_local_ip();
	
	bool get_ext();
	int get_ttl();
	float get_deadline();
	bool contains_deadline();
	float get_timeout();
	AddressType get_address_type();
	RouteType get_route_type();
	SortType get_sort_type();
	std::string get_ifa_name();
	char* get_src_ip();
	char* get_dst_ip();
private:

	// Parses all options and sets the corresponding attributes in the program.
	// Returns false if a wrong argument was given.
	bool parse_options(int argc, char** argv);

	// If this attribute is set, all invoveld nodes are considered. 
	bool ext = false;

	// Time to live
	int ttl = MAX_TTL;

	// After x sec the program stops or if -x this attribute is ignored.
	float deadline = -1;

	// Is the deadline set to a value other than -1
	bool deadline_flag = false;

	// Time to wait for a response in sec.
	float timeout = 1;

	// Defines which addresses are to be shown.
	AddressType address_type = AT_IP;

	// Defines which routes are to be shown.
	RouteType route_type = RT_DST;

	// Defines how the routes are to be sorted.
	SortType sort_type = ST_NORMAL;

	// The used network interface
	std::string ifa_name = DEFAULT_IFA_NAME;

	// The ip address of the machine that exec. traceroute.
	char src_ip[INET_ADDRSTRLEN];

	// The ip address of the destination.
	char dst_ip[INET_ADDRSTRLEN];
};

#endif
