require(
	library castor_settings.click,
	library castor_io.click,
);

/**
 * Handle packets destined for local host
 *
 * Input(0): IP packet
 * Input(1): Other packet
 */
elementclass ToHost {
	$myHostDev |
	
	hostdevice :: ToSimDevice($myHostDev, IP);

	input[0]
		-> CheckIPHeader2
		-> hostdevice;
		
	// Redefined without second input

}

/**
 * Handle incoming packets from local host
 */
elementclass FromHost {
	$myHostDev, $myIP |

	fromhost :: FromSimDevice($myHostDev, SNAPLEN 4096)
		-> CheckIPHeader2 // Input packets have bad IP checksum, so we don't check it (CheckIPHeader2 instead of CheckIPHeader)
		-> output;
		
}

elementclass RecordPkt {
	$map |
	
	input
		-> rec :: FloodingRecordPkt($map)
		-> output;
}

elementclass FloodingBlackhole {
	$map |
	
	input
		-> filter :: FloodingBlackholeFilter
		-> output;
		
	filter[1]
		-> rec :: FloodingRecordPkt($map)
		-> Discard;

}

elementclass HandleIPPacket {
	$map |
	
	input
		-> FloodingSetPktId
		-> rec :: FloodingRecordPkt($map)
		-> output
}

elementclass FloodingHandlePkt {
	$myIP, $crypto, $map |

	input
		-> CheckIPHeader2
		-> blackhole :: FloodingBlackhole($map)
		-> duplicateClassifier :: FloodingCheckDuplicate($crypto)
		-> destinationClassifier :: FloodingDestinationClassifier($myIP, $map)
		-> handleLocal :: RecordPkt($map)
		-> [0]output;
		
	destinationClassifier[1]
		-> forward :: RecordPkt($map)
		-> [1]output; // Forward the message
		
	duplicateClassifier[1]
		-> Discard;

}



/*************************
 * Initialize the Blocks *
 *************************/

ethin :: InputEth($EthDev, fake);
ethout :: OutputEth($EthDev, $broadcastJitter, $unicastJitter);
fromhost :: FromHost($HostDev, fake);
tohost :: ToHost($HostDev);

sam :: SAManagement(fake, netAddr, $numNodes);
crypto :: Crypto(sam);

map :: CastorXcastDestinationMap;

handleIpPacket :: HandleIPPacket(map);
handlepkt :: FloodingHandlePkt(fake, crypto, map);

arpquerier :: ARPQuerier(fake, TIMEOUT 3600, POLL_TIMEOUT 0); // Set timeout sufficiently long, so we don't introduce ARP overhead (we set entries in ns-3)

/*******************
 * Wire the Blocks *
 *******************/

ethin[1] -> ethout;			// Push new ARP Responses back to device
ethin[0] -> [1]arpquerier;	// Push incoming arp responses to querer
ethin[2]
	-> removeEthernetHeader :: Strip(14)
 	-> handlepkt; // received packets			
 
arpquerier -> ethout;	// Send Ethernet packets to output

fromhost -> handleIpPacket -> handlepkt;	// Process new generated packets
 
handlepkt[0]		-> [0]tohost;  // Deliver PKT to host
handlepkt[1]		-> arpquerier; // Forward PKT		