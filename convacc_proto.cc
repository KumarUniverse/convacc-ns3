
#include "ns3/application.h"
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"
#include "ns3/network-module.h"
#include "ns3/nr-mac-scheduler-tdma-rr.h"
#include "ns3/nr-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/traffic-control-helper.h"
#include "ns3/traffic-control-layer.h"
#include <ns3/antenna-module.h>
// #include "netw.h" // meta class, add later.
#include "overlayApplication.h"
#include "ueApp.h"
#include "utils.h"
#include <string>
#include <vector>

#define LISTENPORT 9
#define N_NODES 16
// N_NODES - the total number of nodes in the network topology.
#define N_U_NODES 6
// N_U_NODES - the total number of underlay nodes (routers + host nodes) in the network.
//#define N_NODES 10
// N_NODES - the total number of UEs + routers + host nodes in the network.
#define N_EDGES 15
// N_EDGES - the total number of edges (wired or wireless) in the topology.
#define N_RH_EDGES 6
// N_RH_EDGES - the total number of host and router edges.

using namespace ns3;

/*
Low-fidelity prototype for Convergence Accelerator research project.
Part of this code contains reused code from Yudi Huang's Hex network NS3 project.
*/

NS_LOG_COMPONENT_DEFINE("convacc_proto");

void
installOverlayApps(std::vector<Ptr<overlayApplication>> *vec_app_ptr,
    std::vector<std::pair<uint32_t, uint32_t>> *neighbors_map_ptr, Ipv4AddressHelper *addr_ptr, NodeContainer *nc_ptr)
{
    /**
     * Used to install the overlay applications on the underlay nodes.
    */
    std::vector<Ptr<overlayApplication>> &vec_app = *vec_app_ptr;
    vec_app[0]; // normal access thru reference.
    std::vector<std::pair<uint32_t, uint32_t>> &neighbors_map = *neighbors_map_ptr;
    Ipv4AddressHelper &address = *addr_ptr;
    NodeContainer &nc = *nc_ptr;
    
    //PointToPointHelper link;
    NetDeviceContainer NetDevice;
    std::vector<Ptr<Ipv4>> linkIpv4(2);
    std::vector<Ipv4InterfaceAddress> linkIpv4Addr(2);
    std::vector<uint32_t> n_devices_perNode(2);
    std::vector<Ptr<Node>> endnodes(2);

    // link.DisableFlowControl();
    // link.SetChannelAttribute("Delay", StringValue(std::to_string(20) + "us"));
    // link.SetDeviceAttribute("DataRate", StringValue(std::to_string(2000000) + "kbps"));

    for (auto const& srcdest : neighbors_map)
    {
        PointToPointHelper link;
        link.DisableFlowControl();
        link.SetChannelAttribute("Delay", StringValue(std::to_string(20) + "us"));
        link.SetDeviceAttribute("DataRate", StringValue(std::to_string(2000000) + "kbps"));

        endnodes[0] = nc.Get(srcdest.first);
        endnodes[1] = nc.Get(srcdest.second);
        
        NetDevice = link.Install(endnodes[0], endnodes[1]);
        address.Assign(NetDevice);
        address.NewNetwork();

        for (int i = 0; i < 2; i++)
        {
            linkIpv4[i] = endnodes[i]->GetObject<Ipv4>();
            n_devices_perNode[i] = endnodes[i]->GetNDevices();
            linkIpv4Addr[i] = linkIpv4[i]->GetAddress(n_devices_perNode[i] - 1, 0); // IPv4 interfaces (devide ID) are 0-indexed.
            // In GetAddress(), the 2nd argument is 0 bcuz we want to configure the 1st address of that interface.
            // An interface can have multiple IP addresses, but this is not necessary for regular cases.
            // So unless you're using multiple IP addresses per interface, keep it 0.
        }

        vec_app[srcdest.first]->SetSocket(linkIpv4Addr[1].GetAddress(), srcdest.second, n_devices_perNode[0] - 1);
        vec_app[srcdest.second]->SetSocket(linkIpv4Addr[0].GetAddress(), srcdest.first, n_devices_perNode[1] - 1);
    }
}

int
main(int argc, char* argv[])
{
    bool logging = false;
    if (logging)
    {
        LogComponentEnable("netw", LOG_LEVEL_INFO);
    }

    double stop_time = 300.0; // ms

    /**
     * UE Nodes
     *
     */
    uint32_t numUEs = 4;
    NodeContainer ueNodes;
    ueNodes.Create(numUEs);

    /**
     * Base Station Nodes (gNBs)
     */
    uint32_t numGnbs = 3;
    NodeContainer gnbNodes;
    gnbNodes.Create(numGnbs);

    /**
     * Router Nodes in ISP1
     */
    uint32_t numRouters1 = 3;
    NodeContainer router1Nodes;
    router1Nodes.Create(numRouters1);

    /**
     * Routers Nodes in ISP2
     */
    uint32_t numRouters2 = 2;
    NodeContainer router2Nodes;
    router2Nodes.Create(numRouters2);

    /**
     * Host Node
     */
    NodeContainer hostNode;
    hostNode.Create(1);

    /**
     * Underlay Network
     *
     */
    NodeContainer underlayNodes; // N_U_NODES number of underlay nodes in total (if UEs are included).
    underlayNodes.Add(router1Nodes);
    underlayNodes.Add(router2Nodes);
    underlayNodes.Add(hostNode);
    
    Ipv4AddressHelper address;
    address.SetBase("10.0.0.0", "255.255.255.0");

    // Add IP/TCP/UCP functionality to underlay nodes.
    InternetStackHelper internet;
    internet.Install(underlayNodes); // don't want to add internet to UEs here. ctrl+f internet.Install(ueNodes)

    /**
     * Install Applications (Yudi's)
     **/
    std::vector<Ptr<overlayApplication>> vec_app(N_U_NODES);
    ObjectFactory fact;
    fact.SetTypeId("ns3::overlayApplication");
    fact.Set("RemotePort", UintegerValue(LISTENPORT));
    fact.Set("ListenPort", UintegerValue(LISTENPORT));
    // fact.Set("probe_interval", TimeValue(MicroSeconds(200.0))); // add later when adding probing
    // fact.Set("sandwich_interval", TimeValue(MicroSeconds(100.0))); // Interval between the first
    // and second patch of the sandwich
    // fact.Set ("MaxPackets", UintegerValue (1));
    // fact.Set("PacketSize", UintegerValue(netw_meta._AppPktSize));
    
    double AppStartTime = 21200; // in microseconds
    // The AppStartTime is for the BS apps. The UE apps start immediately.
    for (uint32_t i = 0; i < N_U_NODES; i++)
    {
        vec_app[i] = fact.Create<overlayApplication>();
        vec_app[i]->InitApp(i);
        vec_app[i]->SetStartTime(MicroSeconds(AppStartTime));
        // vec_app[i]->SetStopTime(MilliSeconds(stop_time));
        underlayNodes.Get(i)->AddApplication(vec_app[i]);
        vec_app[i]->SetRecvSocket(); // ERROR you may get here: assert failed. cond="socketFactory"
        // The error gets thrown when UEs are included in the underlay.
    }

    // Specify the src-dest links between the underlay nodes using a map.
    // std::map<uint32_t, uint32_t> neighbors_map;
    std::vector<std::pair<uint32_t, uint32_t>> neighbors_map;
    neighbors_map = {{0, 1}, {0, 2}, {1, 2}, {3, 4}, {2, 3}, {1, 5}}; // UEs excluded from underlayNodes

    installOverlayApps(&vec_app, &neighbors_map, &address, &underlayNodes);


    /**
     * 5G Lena Code
    */
    double centralFrequency = 29e9;
    double bandwidth = 5e9;
    double txPower = 30;
    uint16_t numerology = 5;

    uint32_t network_base_number = 7;
    std::string pattern = "F|F|F|F|F|F|F|F|F|F|";


    /**
     * Set the positions of all of the nodes using the mobility module.
     */
    MobilityHelper mob;
    mob.SetMobilityModel("ns3::ConstantPositionMobilityModel");

    Ptr<ListPositionAllocator> uePositionAlloc = CreateObject<ListPositionAllocator>();
    int ueZVal = 2; // UEs must be placed at a slight height to prevent loss when comm. w/ BS.
    uePositionAlloc->Add(Vector(10, 15, ueZVal));
    uePositionAlloc->Add(Vector(20, 25, ueZVal));
    uePositionAlloc->Add(Vector(30, 30, ueZVal));
    uePositionAlloc->Add(Vector(45, 40, ueZVal));
    mob.SetPositionAllocator(uePositionAlloc);
    mob.Install(ueNodes);

    Ptr<ListPositionAllocator> gnbPositionAlloc = CreateObject<ListPositionAllocator>();
    int gnbZVal = 20; // base stations must be placed at a height to prevent loss when comm. w/ UE.
    gnbPositionAlloc->Add(Vector(20, 10, gnbZVal)); 
    gnbPositionAlloc->Add(Vector(30, 20, gnbZVal));
    gnbPositionAlloc->Add(Vector(45, 30, gnbZVal));
    mob.SetPositionAllocator(gnbPositionAlloc);
    mob.Install(gnbNodes);

    Ptr<ListPositionAllocator> router1PositionAlloc = CreateObject<ListPositionAllocator>();
    router1PositionAlloc->Add(Vector(70, 15, 0));
    router1PositionAlloc->Add(Vector(65, 25, 0));
    router1PositionAlloc->Add(Vector(60, 15, 0));
    mob.SetPositionAllocator(router1PositionAlloc);
    mob.Install(router1Nodes);

    Ptr<ListPositionAllocator> router2PositionAlloc = CreateObject<ListPositionAllocator>();
    router2PositionAlloc->Add(Vector(50, 10, 0));
    router2PositionAlloc->Add(Vector(40, 10, 0));
    mob.SetPositionAllocator(router2PositionAlloc);
    mob.Install(router2Nodes);

    Ptr<ListPositionAllocator> hostPositionAlloc = CreateObject<ListPositionAllocator>();
    hostPositionAlloc->Add(Vector(80, 25, 0));
    mob.SetPositionAllocator(hostPositionAlloc);
    mob.Install(hostNode);


    Ptr<NrPointToPointEpcHelper> epcHelper = CreateObject<NrPointToPointEpcHelper> ();
    Ptr<IdealBeamformingHelper> idealBeamformingHelper = CreateObject<IdealBeamformingHelper>();
    Ptr<NrHelper> nrHelper = CreateObject<NrHelper> ();
    nrHelper->SetBeamformingHelper (idealBeamformingHelper);
    nrHelper->SetEpcHelper (epcHelper);

    BandwidthPartInfoPtrVector allBwps;
    CcBwpCreator ccBwpCreator;
    OperationBandInfo band;

    // For the case of manual configuration of CCs and BWPs
    // Right now, all the base stations share the same BWP.
    std::unique_ptr<ComponentCarrierInfo> cc0 (new ComponentCarrierInfo ());
    std::unique_ptr<BandwidthPartInfo> bwp0 (new BandwidthPartInfo ());

    band.m_centralFrequency  = centralFrequency;
    band.m_channelBandwidth = bandwidth;
    band.m_lowerFrequency = band.m_centralFrequency - band.m_channelBandwidth / 2;
    band.m_higherFrequency = band.m_centralFrequency + band.m_channelBandwidth / 2;

    // Component Carrier 0
    cc0->m_ccId = 0;
    cc0->m_centralFrequency = centralFrequency;
    cc0->m_channelBandwidth = bandwidth;
    cc0->m_lowerFrequency = cc0->m_centralFrequency - cc0->m_channelBandwidth / 2;
    cc0->m_higherFrequency = cc0->m_centralFrequency + cc0->m_channelBandwidth / 2;

    // BWP 0
    bwp0->m_bwpId = 0;
    bwp0->m_centralFrequency = cc0->m_centralFrequency;
    bwp0->m_channelBandwidth = cc0->m_channelBandwidth;
    bwp0->m_lowerFrequency = bwp0->m_centralFrequency - bwp0->m_channelBandwidth / 2;
    bwp0->m_higherFrequency = bwp0->m_centralFrequency + bwp0->m_channelBandwidth / 2;

    // Add BWP to CC0
    cc0->AddBwp (std::move (bwp0));

    // Add CC to the corresponding operation band.
    band.AddCc (std::move (cc0));

    nrHelper->SetPathlossAttribute ("ShadowingEnabled", BooleanValue (false));
    epcHelper->SetAttribute ("S1uLinkDelay", TimeValue (MilliSeconds (0)));
    epcHelper->SetAttribute ("S1uLinkDataRate", DataRateValue (DataRate ("1000Gb/s")));
    epcHelper->SetAttribute ("S1uLinkMtu", UintegerValue (10000));
    nrHelper->SetSchedulerTypeId (TypeId::LookupByName ("ns3::NrMacSchedulerOfdmaPF"));
    idealBeamformingHelper->SetAttribute ("BeamformingMethod", TypeIdValue (DirectPathBeamforming::GetTypeId ()));
    nrHelper->InitializeOperationBand (&band);
    allBwps = CcBwpCreator::GetAllBwps ({band});

    Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue (999999999));

    // Antennas for all the UEs
    nrHelper->SetUeAntennaAttribute ("NumRows", UintegerValue (1));
    nrHelper->SetUeAntennaAttribute ("NumColumns", UintegerValue (1));
    nrHelper->SetUeAntennaAttribute ("AntennaElement", PointerValue (CreateObject<IsotropicAntennaModel> ()));

    // Antennas for all the gNbs
    nrHelper->SetGnbAntennaAttribute ("NumRows", UintegerValue (4));
    nrHelper->SetGnbAntennaAttribute ("NumColumns", UintegerValue (4));
    nrHelper->SetGnbAntennaAttribute ("AntennaElement", PointerValue (CreateObject<IsotropicAntennaModel> ()));

    uint32_t bwpIdForLowLat = 0;
    // uint32_t bwpIdForVoice = 1; // Not needed for now.

    nrHelper->SetGnbBwpManagerAlgorithmAttribute ("NGBR_LOW_LAT_EMBB", UintegerValue (bwpIdForLowLat));
    // nrHelper->SetGnbBwpManagerAlgorithmAttribute ("GBR_CONV_VOICE", UintegerValue (bwpIdForVoice));

    //Install and get the pointers to the NetDevices
    NetDeviceContainer enbNetDev = nrHelper->InstallGnbDevice (gnbNodes, allBwps);
    NetDeviceContainer ueNetDev = nrHelper->InstallUeDevice (ueNodes, allBwps);
    // ^^possible ERROR in prev 2 lines: msg="InternetStackHelper::Install ():
    // Aggregating an InternetStack to a node with an existing Ipv4 object"

    int64_t randomStream = 1;
    randomStream += nrHelper->AssignStreams (enbNetDev, randomStream);
    randomStream += nrHelper->AssignStreams (ueNetDev, randomStream);

    // Set the attribute of the netdevice (enbNetDev.Get (0)) and bandwidth part (0), (1), ...
    for (uint32_t j = 0; j < enbNetDev.GetN(); j++)
    {
        for (uint32_t u = 0; u < allBwps.size(); u++)
        {
            nrHelper->GetGnbPhy (enbNetDev.Get (j), u)->SetAttribute ("Numerology", UintegerValue (numerology));
            nrHelper->GetGnbPhy (enbNetDev.Get (j), u)->SetAttribute ("TxPower", DoubleValue (txPower));
            nrHelper->GetGnbPhy (enbNetDev.Get (j), u)->SetAttribute ("Pattern", StringValue (pattern));
        }
    }

    for (auto it = enbNetDev.Begin (); it != enbNetDev.End (); ++it)
        DynamicCast<NrGnbNetDevice> (*it)->UpdateConfig ();
    for (auto it = ueNetDev.Begin (); it != ueNetDev.End (); ++it)
        DynamicCast<NrUeNetDevice> (*it)->UpdateConfig ();

    // !!Need to debug here.

    Ptr<Node> pgw = epcHelper->GetPgwNode ();
    NodeContainer pgwContainer;
    pgwContainer.Add(pgw);
    std::vector<PointToPointHelper> vec_p2ph(vec_app.size());
    
    NetDeviceContainer internetDevices;
    Ipv4AddressHelper ipv4h;
    Ipv4StaticRoutingHelper ipv4RoutingHelper;

    const std::string p2pPgwEpc{std::to_string(network_base_number+1) + ".0.0.0"};
    ipv4h.SetBase (Ipv4Address(p2pPgwEpc.data()), "255.0.0.0");
    const std::string Addr_IPv4_Network_gNB {std::to_string(network_base_number) + ".0.0.0"};
    

    // Connect UPF (PGW) to router 2b and router 1b.
    // The PGW should already be connected to the SGW.
    std::vector<int> upf_links = {1, 4};
    for (int i : upf_links)
    {
        Ptr<Node> remoteHost = vec_app[i]->GetNode(); // possible ERROR: Attempt to dereference 0 pointer.
        // Connect a remoteHost to PGW. Set up routing too
        vec_p2ph[i].SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("1000Gb/s")));
        vec_p2ph[i].SetDeviceAttribute ("Mtu", UintegerValue (10000));
        vec_p2ph[i].SetChannelAttribute ("Delay", TimeValue (Seconds (0.000)));
        vec_p2ph[i].DisableFlowControl();
        NetDeviceContainer tmp_NetDeviceContainer = vec_p2ph[i].Install (pgw, remoteHost);
        // std::cout << tmp_NetDeviceContainer.Get(0) << " " << tmp_NetDeviceContainer.Get(1) << std::endl;
        internetDevices.Add( tmp_NetDeviceContainer );
        Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
        // remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);
        remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address (Addr_IPv4_Network_gNB.data()), Ipv4Mask ("255.0.0.0"), remoteHost->GetNDevices()-1);
    }
    Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);

    internet.Install (ueNodes);
    Ipv4InterfaceContainer ueIpIface;
    ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueNetDev));
    // Set the default gateway for the UEs
    for (uint32_t j = 0; j < ueNodes.GetN (); ++j)
    {
        Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNodes.Get (j)->GetObject<Ipv4> ());
        ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }
    // attach UEs to the closest eNB before creating the dedicated flows
    nrHelper->AttachToClosestEnb (ueNetDev, enbNetDev);

    // The bearer that will carry low latency traffic.
    uint16_t dlPort_bwp0 = NRPORT;
    EpsBearer bearer_0 (EpsBearer::NGBR_LOW_LAT_EMBB);

    Ptr<EpcTft> tft_0 = Create<EpcTft> ();
    EpcTft::PacketFilter dlpf_0;
    dlpf_0.localPortStart = dlPort_bwp0;
    dlpf_0.localPortEnd = dlPort_bwp0;
    tft_0->Add (dlpf_0);

    // The bearer that will carry CONV_VOICE traffic. Not needed for now.
    // uint16_t dlPort_bwp1 = NRPORT + 1;
    // EpsBearer bearer_1 (EpsBearer::GBR_CONV_VOICE);

    // Ptr<EpcTft> tft_1 = Create<EpcTft> ();
    // EpcTft::PacketFilter dlpf_1;
    // dlpf_1.localPortStart = dlPort_bwp1;
    // dlpf_1.localPortEnd = dlPort_bwp1;
    // tft_1->Add (dlpf_1);

    /** set the Recv Listen Socket for UE **/
    // ObjectFactory fact;
    fact.SetTypeId("ns3::ueApp");
    std::vector<Ptr<ueApp>> vec_ue_app(ueNodes.GetN ());
    //uint32_t idx = 0;
    for (uint32_t i = 0; i < numUEs; i++)
    {
        /** Set Connection between RemoteHost and UEs **/
        vec_app[i]->nr_socket.resize(1);
        Ptr<Node> ue = ueNodes.Get (i);
        // Create and add the UE application to the UE nodes.
        vec_ue_app[i] = fact.Create<ueApp>();
        // std::cout << "UE ID for " << app_interface.GetLocalID() << " is " << ue->GetId() << std::endl;
        ue->AddApplication(vec_ue_app[i]);
        vec_ue_app[i]->initUeApp(*vec_app[i]);
        Ptr<NetDevice> ueDevice = ueNetDev.Get (i);
        
        TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
        vec_app[i]->nr_socket[0] = Socket::CreateSocket(vec_app[i]->GetNode(), tid);
        if (vec_app[i]->nr_socket[0]->Bind() == -1)
            NS_FATAL_ERROR("Failed to bind socket");
        vec_app[i]->nr_socket[0]->Connect(InetSocketAddress(ue->GetObject<Ipv4> ()->GetAddress(1, 0).GetLocal(), dlPort_bwp0));
        nrHelper->ActivateDedicatedEpsBearer (ueDevice, bearer_0, tft_0);
    }

    // End of 5G-Lena code.


    // Set the position of the UPF (PGW node) using the mobility module.
    Ptr<ListPositionAllocator> upfPositionAlloc = CreateObject<ListPositionAllocator>();
    int upfXPos = 50;
    int upfYPos = 20;
    int upfZPos = 0;
    upfPositionAlloc->Add(Vector(upfXPos, upfYPos, upfZPos));
    mob.SetPositionAllocator(upfPositionAlloc);
    mob.Install(pgwContainer);

    // Set the positions of the SGW and MME nodes the same as that of PGW.
    // Note: Node 14 is assumed to be the SGW and node 15 as MME.
    Ptr<Node> sgw = NodeList::GetNode (14);
    Ptr<Node> mme = NodeList::GetNode (15);
    NodeContainer sgwMmeContainer;
    sgwMmeContainer.Add(sgw);
    sgwMmeContainer.Add(mme);
    Ptr<ListPositionAllocator> sgwMmePositionAlloc = CreateObject<ListPositionAllocator>();
    sgwMmePositionAlloc->Add(Vector(upfXPos, upfYPos, upfZPos));
    sgwMmePositionAlloc->Add(Vector(upfXPos, upfYPos, upfZPos));
    mob.SetPositionAllocator(sgwMmePositionAlloc);
    mob.Install(sgwMmeContainer);
    

    // Animate the network in NetAnim
    AnimationInterface anim("convacc-proto-animation.xml");

    /**
     * Name NetAnim Nodes
     */
    // Name the 4 UE Nodes as MECs
    for (int i = 0; i < 4; i++)
        anim.UpdateNodeDescription(i, "MEC" + std::to_string(i));

    // Name the 3 BS Nodes
    for (int i = 0; i < 3; i++)
    {
        int j = i + 4;
        anim.UpdateNodeDescription(j, "BS" + std::to_string(i));
    }

    // Name the 3 ISP1 Nodes
    for (int i = 0; i < 3; i++)
    {
        int j = i + 7;
        char c = (char) (i + ((int) 'a'));
        anim.UpdateNodeDescription(j, "R1" + std::string(1, c));
    }

    // Name the 2 ISP2 Nodes
    for (int i = 0; i < 2; i++)
    {
        int j = i + 10;
        char c = (char) (i + ((int) 'a'));
        anim.UpdateNodeDescription(j, "R2" + std::string(1, c));
    }

    // Name Host Node
    anim.UpdateNodeDescription(12, "Host");

    // Name UPF Node
    anim.UpdateNodeDescription(13, "UPF");

    // Remove the names of SGW and MME nodes (14 & 15).
    for (int i = 0; i < 2; i++)
    {
        int j = i + 14;
        anim.UpdateNodeDescription(j, "");
    }

    /**
     * Color NetAnim Nodes
     */
    // Color the 4 UE Nodes Green
    for (int i = 0; i < 4; i++)
        anim.UpdateNodeColor(i, 0, 200, 0);

    // Color the 3 BS Nodes Yellow
    for (int i = 4; i < 7; i++)
        anim.UpdateNodeColor(i, 255, 200, 0);

    // Color the 3 ISP1 Nodes Red
    for (int i = 7; i < 10; i++)
        anim.UpdateNodeColor(i, 255, 0, 0);

    // Color the 2 ISP2 Nodes Red
    for (int i = 10; i < 12; i++)
        anim.UpdateNodeColor(i, 255, 0, 0);

    // Color Host Node Blue
    anim.UpdateNodeColor(12, 0, 0, 255);

    // Color UPF, SGW and MME Nodes Purple
    for (int i = 13; i < N_NODES; i++)
        anim.UpdateNodeColor(i, 100, 10, 170);


    /**
     * Resize NetAnim Nodes by a factor of 3.
     */
    int resizeFactor = 3;
    for (int i = 0; i < N_NODES; i++)
    {
        anim.UpdateNodeSize(i, resizeFactor, resizeFactor);
    }

    // anim.SetStartTime(Seconds(10));
    // anim.SetStopTime(Seconds(20));

    /**
     * Run Simulation
    */
    NS_LOG_INFO("Run Simulation.");
    std::cout << "Before run" << std::endl;
    Time time_stop_simulation = MilliSeconds(stop_time*1.2);
    // Simulator::Schedule(time_stop_simulation, stop_NR, vec_NrHelper);
    Simulator::Stop(time_stop_simulation);
    Simulator::Run();
    std::cout << "After run" << std::endl;
}