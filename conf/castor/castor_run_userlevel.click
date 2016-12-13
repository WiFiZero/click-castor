// I/O interfaces for userlevel drivers
define(
	$EthDev wlan0,
	$HostDev tun0,
);

AddressInfo(fake $EthDev);

tun :: KernelTun(fake:ip/16, HEADROOM $headroom, DEVNAME $HostDev);
tun -> fromhostdev :: { input -> output };
tohostdev :: { input -> output; } -> tun;
fromextdev :: FromDevice($EthDev, SNAPLEN 4096, PROMISC true, SNIFFER false)
toextdev :: ToDevice($EthDev);

/**
 * Dummy class, does nothing as we do not need artificial jitter on 'real' devices
 */
elementclass BroadcastJitter {
	$broadcastJitter |

	input -> output;
}

// Finally wire all blocks
require(
	library castor_socket.click,
	library castor_init_blocks.click,
	library castor_wiring.click,
);
