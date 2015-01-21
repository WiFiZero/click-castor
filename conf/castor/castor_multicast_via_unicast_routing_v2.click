define (
	$CASTOR_VERSION 2,
);

require(
	library castor_io.click,
	library castor_settings.click,
	library castor_common.click,
	library castor_unicast.click,
);

/*************************
 * Initialize the Blocks *
 *************************/

ethin :: InputEth($EthDev, fake);
ethout :: OutputEth($EthDev, $broadcastJitter, $unicastJitter);
fromhost :: FromHost($HostDev, fake, $headroom);
tohost :: ToHost($HostDev);

sam::SAManagement(fake, netAddr, $numNodes);
crypto::Crypto(sam);
flowDB :: CastorFlowStub;
flow_merkle :: CastorFlowMerkle(flowDB, crypto);
neighbors :: CastorNeighbors($neighborTimeout);
routingtable :: CastorRoutingTable($updateDelta);
history :: CastorHistory;
routeselector :: CastorRouteSelectorExperimental(routingtable, neighbors, history, $broadcastAdjust);
castorclassifier :: CastorClassifier;
handlepkt :: CastorHandlePkt(fake, routeselector, routingtable, history, crypto);
handleack :: CastorHandleAck(fake, routingtable, history, crypto, false);

handleIpPacket :: CastorHandleMulticastToUnicastIpPacket(fake, flowDB, crypto);
arpquerier :: ARPQuerier(fake, TIMEOUT 3600, POLL_TIMEOUT 0); // Set timeout sufficiently long, so we don't introduce ARP overhead (we set entries in ns-3)


/*******************
 * Wire the Blocks *
 *******************/

ethin[1] -> ethout;			// Push new ARP Responses back to device
ethin[0] -> [1]arpquerier;	// Push incoming arp responses to querer
ethin[2]
	-> cEtherFilter :: CastorEtherFilter
 	-> castorclassifier;	// Classify received packets			

cEtherFilter[1]
	-> CastorAddNeighbor(neighbors)
	-> Discard;

fromhost
	-> handleIpPacket 
	-> handlepkt;		// Process new generated packets
 
castorclassifier[0] -> handlepkt; // Process PKTs
castorclassifier[1] -> handleack; // Process ACKs
castorclassifier[2] -> [1]tohost; // Deliver non-Castor packets directly to host

handlepkt[0]		-> CastorRemoveHeader -> [0]tohost;  // Deliver PKT to host
handlepkt[1]		-> arpquerier; // Return ACK		
handlepkt[2]		-> arpquerier; // Forward PKT
handleack			-> arpquerier; // Forward ACK

arpquerier -> ethout;	// Send Ethernet packets to output

CastorBeaconGenerator($beaconingInterval, fake, $EthDev) -> ethout;
