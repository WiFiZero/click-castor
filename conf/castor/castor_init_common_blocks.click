/****************************
 * Initialize common blocks *
 ****************************/

sam::SAManagement(fake);
crypto::Crypto(sam);
flowmanager :: CastorFlowManager($flowSize, crypto);

groupmap :: CastorXcastDestinationMap;

neighbors :: Neighbors($neighborTimeout, $neighborsEnable);
history :: CastorHistory;
routingtable :: CastorRoutingTable($updateDelta);
flowtable :: CastorFlowTable(crypto);
timeouttable :: CastorTimeoutTable(INIT $initTo, MIN $minTo, MAX $maxTo);
ratelimits :: CastorRateLimitTable(INIT $initRate, MIN $minRate, MAX $maxRate);

castorclassifier :: CastorClassifier(fake, neighbors)
ratelimiter :: CastorRateLimiter(ratelimits, $bucketSize);

fromextdev -> castorclassifier;
ethout :: OutputEth($broadcastJitter) -> toextdev;
fromhostdev -> fromhost :: FromHost(fake);
tohost :: ToHost() -> tohostdev;
