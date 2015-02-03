require(
	library castor_settings.click,
	library castor_io.click,
	library castor_common.click,
	library castor_unicast.click,
	library castor_init_common_blocks.click,
);

// The flow to use
flowDB :: CastorFlowStub;
flow_merkle :: CastorFlowMerkle(flowDB, crypto);

// How to choose next hop
routeselector :: CastorRouteSelectorOriginal(routingtable, neighbors, $broadcastAdjust);

// How to handle PKTs and ACKs
handlepkt :: CastorHandlePkt(fake, routeselector, routingtable, history, crypto);
handleack :: { input -> handleCastorAck :: CastorHandleAck(fake, routingtable, history, neighbors, crypto, true) -> CastorXcastResetDstAnno(true) -> output; };

handleIpPacket :: CastorHandleMulticastToUnicastIpPacket(fake, flowDB, crypto);
removeHeader :: CastorRemoveHeader;

// Finally wire all blocks
require(
	library castor_wiring.click,
);
