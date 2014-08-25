/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

// - All nodes are Click based.
// - The single ethernet interface that each node
//   uses is named 'eth0' in the Click file.
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-click-routing.h"
#include "ns3/click-internet-stack-helper.h"
#include "ns3/random-variable-stream.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/config.h"

using namespace ns3;

#ifndef NS3_CLICK
#define NS3_CLICK
#endif

NS_LOG_COMPONENT_DEFINE("NsclickCastor");

#ifdef NS3_CLICK

uint32_t phyTx = 0;

void
PhyTx(Ptr<const Packet> p)
{
	phyTx += p->GetSize();
}

std::string readStringStat(Ptr<Ipv4ClickRouting> clickRouter, std::string what, std::string where) {
	return clickRouter->ReadHandler(where, what);
}

unsigned int readIntStat(Ptr<Ipv4ClickRouting> clickRouter, std::string what, std::string where) {
	std::string result = readStringStat(clickRouter, what, where);
	unsigned int i;
	sscanf(result.c_str(), "%d", &i);
	return i;
}

unsigned int readPidCount(Ptr<Ipv4ClickRouting> clickRouter, std::string where) {
	return readIntStat(clickRouter, "num", where);
}

unsigned int readPktCount(Ptr<Ipv4ClickRouting> clickRouter, std::string where) {
	return readIntStat(clickRouter, "numUnique", where);
}

unsigned int readAccumPktSize(Ptr<Ipv4ClickRouting> clickRouter, std::string where) {
	return readIntStat(clickRouter, "size", where);
}

unsigned int readBroadcasts(Ptr<Ipv4ClickRouting> clickRouter, std::string where) {
	return readIntStat(clickRouter, "broadcasts", where);
}

unsigned int readUnicasts(Ptr<Ipv4ClickRouting> clickRouter, std::string where) {
	return readIntStat(clickRouter, "unicasts", where);
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

bool readPidTimestamp(Ptr<Ipv4ClickRouting> clickRouter, std::string where, std::string& pid, double& timestamp) {
	std::string result = readStringStat(clickRouter, "seq_entry", where);
	if(result.empty())
		return false;
	std::vector<std::string> entry = split(result, ' '); // result is in form of '<pid> <timestamp>'
	pid = entry[0];
	sscanf(entry[1].c_str(), "%lf", &timestamp);
	return true;
}


void WriteArp(Ptr<Ipv4ClickRouting> clickRouter, size_t nNodes,	const Ipv4Address& base) {
	// Access the handler
	for (unsigned int i = 1; i <= nNodes; i++) {
		Ipv4Address ip = Ipv4Address(base.Get() + i);
		Mac48Address mac;
		uint8_t buf[6] = { 0 };
		buf[5] = i;
		mac.CopyFrom(buf);

		// Create entry of form "<IP> <MAC>"
		std::stringstream stream;
		ip.Print(stream);
		stream << " " << mac;

		clickRouter->WriteHandler("arpquerier", "insert", stream.str().c_str());
	}
	// TODO: Currently only works with 254 nodes
}

void WriteXcastMap(Ptr<Ipv4ClickRouting> clickRouter, Ipv4Address group, const std::vector<Ipv4Address>& destinations) {
	// Create entry of form "<GroupAddr> <Dest1Addr> <Dest2Addr> ... <DestNAddr>"
	std::stringstream stream;

	// Multicast address is first in list
	group.Print(stream);

	for (unsigned int i = 0; i < destinations.size(); i++) {
		stream << " ";
		destinations[i].Print(stream);
	}

	clickRouter->WriteHandler("handleIpPacket/map", "insert", stream.str().c_str());
}

/**
 * Returns random value between 0 and max (both inclusive)
 */
inline double getRand(double max) {
	double min = 0.0;
	Ptr<UniformRandomVariable> randVar = CreateObject<UniformRandomVariable>();
	randVar->SetAttribute("Min", DoubleValue(min));
	randVar->SetAttribute("Max", DoubleValue(max));

	return randVar->GetValue();
}

// has to be defined here, cannot be used as local template argument
struct LessIpv4Address {
	bool operator()(const Ipv4Address& x, const Ipv4Address& y) const {
		return x.Get() < y.Get();
	}
};

NetDeviceContainer setPhysicalChannel(NodeContainer& nodes, double transmissionRange) {
	std::string phyMode("DsssRate11Mbps");

	// disable fragmentation for frames below 2200 bytes
	Config::SetDefault("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue("2200"));
	// turn off RTS/CTS for frames below 2200 bytes
	Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue("2200"));
	// Fix non-unicast data rate to be the same as that of unicast
	Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode",	StringValue(phyMode));

	WifiHelper wifi;
	wifi.SetStandard(WIFI_PHY_STANDARD_80211b);

	YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default();
	// ns-3 supports RadioTap and Prism tracing extensions for 802.11b
	wifiPhy.SetPcapDataLinkType(YansWifiPhyHelper::DLT_IEEE802_11_RADIO);

	YansWifiChannelHelper wifiChannel;
	wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel", "Speed", DoubleValue(299792458.0));
	//wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");

	wifiChannel.AddPropagationLoss("ns3::RangePropagationLossModel", "MaxRange", DoubleValue(transmissionRange));
	//wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel");
	wifiPhy.SetChannel(wifiChannel.Create());

	// Add a non-QoS upper mac, and disable rate control
	NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default();
	wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode",
			StringValue(phyMode), "ControlMode", StringValue(phyMode));
	// Set it to adhoc mode
	wifiMac.SetType("ns3::AdhocWifiMac");
	return wifi.Install(wifiPhy, wifiMac, nodes);
}

Ptr <PositionAllocator> getRandomRectanglePositionAllocator(double xSize, double ySize) {
	ObjectFactory pos;
	pos.SetTypeId("ns3::RandomRectanglePositionAllocator");
	std::ostringstream xSizeUniformRandomVariable;
	xSizeUniformRandomVariable << "ns3::UniformRandomVariable[Min=0.0|Max=" << xSize << "]";
	pos.Set("X", StringValue(xSizeUniformRandomVariable.str()));
	std::ostringstream ySizeUniformRandomVariable;
	ySizeUniformRandomVariable << "ns3::UniformRandomVariable[Min=0.0|Max=" << ySize << "]";
	pos.Set("Y", StringValue(ySizeUniformRandomVariable.str()));
	return pos.Create ()->GetObject <PositionAllocator> ();
}

void setRandomWaypointMobility(NodeContainer& nodes, double xSize, double ySize, double speed, double pause) {
	MobilityHelper mobility;
	Ptr <PositionAllocator> taPositionAlloc = getRandomRectanglePositionAllocator(xSize, ySize);

	std::ostringstream speedVariable;
	speedVariable << "ns3::UniformRandomVariable[Min=0.0|Max="<< speed << "]";
	std::ostringstream pauseVariable;
	pauseVariable << "ns3::UniformRandomVariable[Min=0.0|Max=" << pause << "]";

	mobility.SetMobilityModel("ns3::RandomWaypointMobilityModel",
			"Speed", StringValue(speedVariable.str()),
			"Pause", StringValue(pauseVariable.str()),
			"PositionAllocator", PointerValue(taPositionAlloc));
	mobility.SetPositionAllocator(taPositionAlloc);
	mobility.Install (nodes);
}

void setConstantPositionMobility(NodeContainer& nodes, double xSize, double ySize) {
	MobilityHelper mobility;
	Ptr <PositionAllocator> taPositionAlloc = getRandomRectanglePositionAllocator(xSize, ySize);
	mobility.SetPositionAllocator(taPositionAlloc);
	mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
	mobility.Install(nodes);
}

void setClickRouter(NodeContainer& nodes, StringValue clickConfig) {
	ClickInternetStackHelper clickinternet;
	for (unsigned int i = 0; i < nodes.GetN(); i++)
		clickinternet.SetClickFile(nodes.Get(i), clickConfig.Get());
	clickinternet.SetRoutingTableElement(nodes, "rt");
	clickinternet.Install(nodes);
}

typedef struct NetworkConfiguration {
	double x, y;
	double range;
	size_t nNodes;
	NetworkConfiguration(double x, double y, double range, size_t nNodes) :
		x(x), y(y), range(range), nNodes(nNodes) {}
	NetworkConfiguration() : x(0), y(0), range(0), nNodes(0) {}
} NetworkConfiguration;

typedef struct TrafficConfiguration {
	double senderFraction;
	size_t groupSize;
	size_t packetSize;
	Time sendInterval;
	TrafficConfiguration(double senderFraction, size_t groupSize, size_t packetSize = 256, Time sendInterval = Seconds(0.25)) :
		senderFraction(senderFraction), groupSize(groupSize), packetSize(packetSize), sendInterval(sendInterval) {}
	TrafficConfiguration() : senderFraction(0), groupSize(0), packetSize(256), sendInterval(Seconds(0.25)) {}
} TrafficConfiguration;

typedef struct MobilityConfiguration {
	double speed, pause;
	MobilityConfiguration(double speed, double pause) :
		speed(speed), pause(pause) {}
	MobilityConfiguration() : speed(0), pause(0) {}
} MobilityConfiguration;

void simulate(
		int run,
		StringValue clickConfig,
		Time duration,
		const NetworkConfiguration& netConfig,
		const TrafficConfiguration& trafficConfig,
		const MobilityConfiguration& mobilityConfig,
		std::string outFile
		) {

	RngSeedManager::SetSeed(12345);
	RngSeedManager::SetRun(run);

	size_t nSenders = (size_t) ceil(netConfig.nNodes * trafficConfig.senderFraction);

	uint32_t MaxPacketSize = trafficConfig.packetSize - 28; // IP+UDP header size = 28 byte

	// Network
	const Ipv4Address baseAddr("192.168.201.0");
	const Ipv4Mask networkMask("255.255.255.0");
	const Ipv4Address groupAddr("224.0.2.0");

	// Setup groups based on 'nSenders' (e.g. 2) and 'groupSize' (e.g. 3) in the format:
	// 'senderGroupAssign':
	// 		192.168.201.1 -> 224.0.2.1; 192.168.201.5 -> 224.0.2.5
	std::map<Ipv4Address, Ipv4Address, LessIpv4Address> senderGroupAssign;
	// 'groups':
	// 		192.168.201.1 -> 192.168.201.2, 192.168.201.3, 192.168.201.4;
	// 		192.168.201.5 -> 192.168.201.6, 192.168.201.7, 192.168.201.8
	std::map<Ipv4Address, std::vector<Ipv4Address>, LessIpv4Address> groups; // multicast group -> xcast receivers

	for (unsigned int i = 0, iAddr = 1; i < nSenders; i++) {
		Ipv4Address sender = Ipv4Address(baseAddr.Get() + iAddr);
		Ipv4Address group = Ipv4Address(groupAddr.Get() + iAddr);
		std::vector<Ipv4Address> xcastDestinations;
		iAddr = iAddr % netConfig.nNodes + 1; // Circular count from 1..nNodes
		for (unsigned int j = 0; j < trafficConfig.groupSize; j++, iAddr = iAddr % netConfig.nNodes + 1) {
			Ipv4Address dest = Ipv4Address(baseAddr.Get() + iAddr);
			xcastDestinations.push_back(dest);
		}
		senderGroupAssign.insert(std::make_pair(sender, group));
		groups.insert(std::make_pair(group, xcastDestinations));
	}

	// Set up network
	NodeContainer n;
	n.Create(netConfig.nNodes);

	NetDeviceContainer d = setPhysicalChannel(n, netConfig.range);
	if(mobilityConfig.speed == 0.0)
		setConstantPositionMobility(n, netConfig.x, netConfig.y);
	else
		setRandomWaypointMobility(n, netConfig.x, netConfig.y, mobilityConfig.speed, mobilityConfig.pause);

	setClickRouter(n, clickConfig);

	Ipv4AddressHelper ipv4;
	ipv4.SetBase(baseAddr, networkMask);
	Ipv4InterfaceContainer i = ipv4.Assign(d);

	// Set up traffic generation
	ApplicationContainer apps;
	uint16_t port = 4242;
	unsigned int nodeIndex = 0;
	for (std::map<Ipv4Address, Ipv4Address>::iterator it = senderGroupAssign.begin(); it != senderGroupAssign.end(); it++) {
		Ipv4Address groupIp = it->second;

		// Setup multicast source
		UdpClientHelper client(groupIp, port);
		client.SetAttribute("MaxPackets", UintegerValue(UINT32_MAX));
		client.SetAttribute("Interval", TimeValue(trafficConfig.sendInterval));
		client.SetAttribute("PacketSize", UintegerValue(MaxPacketSize));
		apps = client.Install(NodeContainer(n.Get(nodeIndex)));
		apps.Start(Seconds(2.0) + trafficConfig.sendInterval / nSenders * nodeIndex);
		apps.Stop(duration + Seconds(2.0));

		nodeIndex++;

		std::vector<Ipv4Address> dsts = groups.at(groupIp);
		for(std::vector<Ipv4Address>::iterator itDst = dsts.begin(); itDst != dsts.end(); itDst++) {
			UdpServerHelper server(port);
			apps = server.Install(n.Get(nodeIndex));
			apps.Start(Seconds(1.0));
			apps.Stop(duration + Seconds(3.0));
		}
	}

	//wifiPhy.EnablePcap("castor-xcast", d);

	// We fill in the ARP tables at the beginning of the simulation
	for (unsigned int i = 0; i < n.GetN(); i++) {
		Simulator::Schedule(Seconds(0.5), &WriteArp, n.Get(i)->GetObject<Ipv4ClickRouting>(), netConfig.nNodes, baseAddr);
		// Write Xcast destination mapping
		for (std::map<Ipv4Address, std::vector<Ipv4Address> >::iterator it = groups.begin(); it != groups.end(); it++)
			Simulator::Schedule(Seconds(0.5), &WriteXcastMap, n.Get(i)->GetObject<Ipv4ClickRouting>(), it->first, it->second);
	}

	  // Install FlowMonitor on all nodes
	  FlowMonitorHelper flowmon;
	  Ptr<FlowMonitor> monitor = flowmon.InstallAll();

	  Config::ConnectWithoutContext("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyTxBegin", MakeCallback(&PhyTx));

	//
	// Now, do the actual simulation.
	//
	time_t start; time(&start);
	Simulator::Stop(duration + Seconds(6.0));
	Simulator::Run();
	time_t end; time(&end);

	NS_LOG_INFO("Run #" << run << " (" << duration.GetSeconds() << " seconds, " << clickConfig.Get() << ")");
	NS_LOG_INFO("  CONFIG " << netConfig.x << "x" << netConfig.y << ", " << netConfig.nNodes << " nodes @ " << netConfig.range << " range");
	NS_LOG_INFO("  CONFIG " << nSenders << " senders -> " << trafficConfig.groupSize << " each, " << trafficConfig.packetSize << " bytes / " << trafficConfig.sendInterval.GetSeconds() << " s");
	NS_LOG_INFO("  CONFIG " << "speed " << mobilityConfig.speed << ", pause " << mobilityConfig.pause);

	NS_LOG_INFO("  Done after " << difftime(end, start) << " seconds");

	//
	// Schedule evaluation
	//
	uint32_t numPidsSent = 0;
	uint32_t numPidsRecv = 0;
	uint32_t pktBandwidthUsage = 0;
	uint32_t ackBandwidthUsage = 0;
	uint32_t totalBandwidthUsage = 0;
	uint32_t numPktsSent = 0;
	uint32_t broadcasts = 0;
	uint32_t unicasts = 0;
	uint32_t numPktsForwarded = 0;
	uint32_t numAcksForwarded = 0;
	std::map<std::string, double> pidsSent;
	std::vector<double> delays;
	double avgDelay;

	std::string pktForward = "handlepkt/forward/rec";
	std::string pktSend = "handleIpPacket/rec";
	std::string pktDeliver = "handlepkt/handleLocal/rec";
	std::string ackForward = "handleack/recAck";
	std::string ackSend = "handlepkt/sendAck/recAck";
	for(unsigned int i = 0; i < netConfig.nNodes; i++) {
		numPidsSent += readPidCount(n.Get(i)->GetObject<Ipv4ClickRouting>(), pktSend);
		numPidsRecv += readPidCount(n.Get(i)->GetObject<Ipv4ClickRouting>(), pktDeliver);
		pktBandwidthUsage += readAccumPktSize(n.Get(i)->GetObject<Ipv4ClickRouting>(), pktForward);
		ackBandwidthUsage += readAccumPktSize(n.Get(i)->GetObject<Ipv4ClickRouting>(), ackSend) +
							 readAccumPktSize(n.Get(i)->GetObject<Ipv4ClickRouting>(), ackForward);
		numPktsSent += readPktCount(n.Get(i)->GetObject<Ipv4ClickRouting>(), pktSend);
		numPktsForwarded += readPktCount(n.Get(i)->GetObject<Ipv4ClickRouting>(), pktForward);
		numAcksForwarded += readPktCount(n.Get(i)->GetObject<Ipv4ClickRouting>(), ackSend) +
							readPktCount(n.Get(i)->GetObject<Ipv4ClickRouting>(), ackForward);
		broadcasts += readBroadcasts(n.Get(i)->GetObject<Ipv4ClickRouting>(), pktForward);
		unicasts += readUnicasts(n.Get(i)->GetObject<Ipv4ClickRouting>(), pktForward);
		std::string pid;
		double timestamp;
		while(readPidTimestamp(n.Get(i)->GetObject<Ipv4ClickRouting>(), pktSend, pid, timestamp))
			pidsSent.insert(std::make_pair(pid, timestamp));
	}
	totalBandwidthUsage = pktBandwidthUsage + ackBandwidthUsage;
	for(unsigned int i = 0; i < netConfig.nNodes; i++) {
		std::string pid;
		double recTimestamp;
		while(readPidTimestamp(n.Get(i)->GetObject<Ipv4ClickRouting>(), pktDeliver, pid, recTimestamp)) {
			if(pidsSent[pid] == 0.0) {
				NS_LOG_INFO("" << pid << " was received but never sent");
				break;
			}
			double sentTimestamp = pidsSent[pid];
			double delay = recTimestamp - sentTimestamp;
			delays.push_back(recTimestamp - sentTimestamp);
			avgDelay += delay;
		}
	}

	double pdr = (double) numPidsRecv / numPidsSent;
	double delay = avgDelay / numPidsRecv * 1000;
	double buPerPidNet = (double) totalBandwidthUsage / numPidsSent;
	double buPerPidPhy = (double) phyTx / numPidsSent;
	double buPerPidPkt = (double) pktBandwidthUsage / numPidsSent;
	double buPerPidAck = (double) ackBandwidthUsage / numPidsSent;

	NS_LOG_INFO("  STAT PDR        " << pdr << " (" << numPidsRecv << "/" << numPidsSent << ")");
	NS_LOG_INFO("  STAT BU per PID " << buPerPidPhy  << " (phy), " << buPerPidNet << " (net) bytes");
	NS_LOG_INFO("        frac(PKT) " << ((double) buPerPidPkt / buPerPidNet));
	NS_LOG_INFO("        frac(ACK) " << ((double) buPerPidAck / buPerPidNet));
	NS_LOG_INFO("  STAT DELAY      " << delay << " ms");
	NS_LOG_INFO("  STAT HOP COUNT  " << numPktsForwarded);
	NS_LOG_INFO("         per PKT  " << ((double) numPktsForwarded / numPktsSent));
	NS_LOG_INFO("         per PID  " << ((double) numPktsForwarded / numPidsSent));
	NS_LOG_INFO("  STAT BROADCAST  " << ((double) broadcasts / (unicasts + broadcasts)) << " (" << broadcasts << "/" << (unicasts + broadcasts) << ")");

	//
	// Cleanup
	//
	Simulator::Destroy();

	// Write statistics to output file
	std::ofstream out;

	out.open(outFile.c_str());
	if(out.fail()) {
		NS_LOG_ERROR("Could not write to file '" << outFile << "'");
		return;
	}

	out << pdr << " "
		<< buPerPidPhy << " "
		<< buPerPidNet << " "
		<< ((double) buPerPidPkt * (buPerPidPhy/buPerPidNet)) << " "
		<< ((double) buPerPidAck * (buPerPidPhy/buPerPidNet)) << " "
		<< delay << " "
		<< broadcasts << " "
		<< unicasts;

	out.close();

}

#endif

int main(int argc, char *argv[]) {
#ifdef NS3_CLICK

	// Possible run configurations

	std::map<std::string, StringValue> clickConfigs;
	clickConfigs.insert(std::make_pair("xcast",         "/home/milan/click/conf/castor/castor_xcast_routing.click"));
	clickConfigs.insert(std::make_pair("xcast-promisc", "/home/milan/click/conf/castor/castor_xcast_routing_promisc.click"));
	clickConfigs.insert(std::make_pair("regular",       "/home/milan/click/conf/castor/castor_multicast_via_unicast_routing.click"));


	std::map<std::string, NetworkConfiguration> networkConfigs;
	// Configurations with aprroximately the same node density
	networkConfigs.insert(std::make_pair("small",  NetworkConfiguration(1000.0, 1000.0, 500.0,  10)));
	networkConfigs.insert(std::make_pair("medium", NetworkConfiguration(2000.0, 2000.0, 500.0,  50)));
	networkConfigs.insert(std::make_pair("large",  NetworkConfiguration(3000.0, 3000.0, 500.0, 100))); // as in Castor


	std::map<std::string, TrafficConfiguration> trafficConfigs;
	// as in Castor (5 unicast flows @ 100 nodes)
	trafficConfigs.insert(std::make_pair( "5_1", TrafficConfiguration(0.05,  1)));
	// 40% receivers
	trafficConfigs.insert(std::make_pair("4_10", TrafficConfiguration(0.04, 10)));
	trafficConfigs.insert(std::make_pair( "8_5", TrafficConfiguration(0.08,  5)));
	trafficConfigs.insert(std::make_pair("20_2", TrafficConfiguration(0.20,  2)));
	trafficConfigs.insert(std::make_pair("40_1", TrafficConfiguration(0.40,  1)));
	// 20% receivers
	trafficConfigs.insert(std::make_pair("2_10", TrafficConfiguration(0.02, 10)));
	trafficConfigs.insert(std::make_pair( "4_5", TrafficConfiguration(0.04,  5)));
	trafficConfigs.insert(std::make_pair("10_2", TrafficConfiguration(0.10,  2)));
	trafficConfigs.insert(std::make_pair("20_1", TrafficConfiguration(0.20,  1)));


	std::map<std::string, MobilityConfiguration> mobilityConfigs;
	mobilityConfigs.insert(std::make_pair( "0", MobilityConfiguration( 0.0, 0.0)));
	mobilityConfigs.insert(std::make_pair("10", MobilityConfiguration(10.0, 0.0)));
	mobilityConfigs.insert(std::make_pair("20", MobilityConfiguration(20.0, 0.0))); // as in Castor


	// Enable logging
	LogComponentEnable("NsclickCastor", LOG_LEVEL_INFO);

	// Set up command line usage
	CommandLine cmd;
	// Default values
	uint32_t run = 1;
	double duration = 60.0;
	std::string click          = "xcast";
	std::string networkConfig  = "small";
	std::string trafficConfig  = "4_5";
	std::string mobilityConfig = "10";
	std::string outFile		   = "";

	cmd.AddValue("run",      "The instance of this experiment.",                                           run);
	cmd.AddValue("duration", "The simulated time in seconds.",                                             duration);
	cmd.AddValue("click",    "The Click configuration file to be used.",                                   click);
	cmd.AddValue("network",  "The network configuration to use, e.g., 'small', 'medium' or 'large'.",      networkConfig);
	cmd.AddValue("traffic",  "The traffic configuration to use, e.g., 'normal'.",                          trafficConfig);
	cmd.AddValue("mobility", "The mobility model and configuration to use, e.g., 'constant' or 'moving'.", mobilityConfig);
	cmd.AddValue("outfile",  "File for statistics output.",												   outFile);

	cmd.Parse (argc, argv);

	simulate(run, clickConfigs[click], Seconds(duration), networkConfigs[networkConfig], trafficConfigs[trafficConfig], mobilityConfigs[mobilityConfig], outFile);

#else
	NS_FATAL_ERROR ("Can't use ns-3-click without NSCLICK compiled in");
#endif
}
