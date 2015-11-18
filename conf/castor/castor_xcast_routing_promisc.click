require(
	library castor_settings.click,
	library castor_io.click,
	library castor_common.click,
	library castor_xcast.click,
	library castor_init_common_blocks.click
);

// How to choose next hop
routeselector :: CastorRouteSelectorOriginal(routingtable, neighbors, $broadcastAdjust);

// How to handle PKTs and ACKs
handlepkt :: {
	input
		-> handleXcastPkt :: CastorHandleXcastPkt(fake, routeselector, routingtable, timeouttable, history, crypto)[0,1]
		=> [0,1]output;
	handleXcastPkt[2] -> CastorResetDstAnno -> [2]output;
};
handleack :: { input -> handleXcastAck :: CastorHandleAck(fake, routingtable, timeouttable, ratelimits, history, neighbors, crypto, true) -> CastorResetDstAnno -> output; };

handleIpPacket :: CastorHandleMulticastIpPacket(fake, flowmanager, crypto);
removeHeader :: CastorXcastRemoveHeader;

// Finally wire all blocks
require(
	library castor_wiring.click,
);
