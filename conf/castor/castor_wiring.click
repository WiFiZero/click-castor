/*******************
 * Wire the Blocks *
 *******************/

fromhost
	-> handleIpPacket
	-> SetEtherAddress(ANNO 0, ADDR fake)
	-> handlepkt;		// Process new generated packets

castorclassifier[0]
	-> ratelimiter // Rate limit PKTs
	-> handlepkt;  // Process PKTs
castorclassifier[1] -> handleack; // Process ACKs
castorclassifier[2] -> [1]tohost; // Deliver non-Castor packets directly to host

handlepkt[0]		-> removeHeader -> [0]tohost; // Deliver PKT to host
handlepkt[1]		-> DynamicEtherEncap(fake) -> [1]ethout; // Return ACK
handlepkt[2]		-> DynamicEtherEncap(fake) -> [0]ethout; // Forward PKT
handleack			-> DynamicEtherEncap(fake) -> [1]ethout; // Forward ACK

beacons :: NeighborBeaconGenerator($beaconingInterval, $EthDev, $neighborsEnable)
	-> [2]ethout;
