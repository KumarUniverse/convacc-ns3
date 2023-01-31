
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
// #include "netw.h"
#include "overlayApplication.h"
#include "ueApp.h"
#include "utils.h"
#include <string>
#include <vector>

using namespace ns3;

/*
Low-fidelity prototype for Convergence Accelerator research project.
Part of this code contains reused code from Yudi Huang's Hex network NS3 project.
*/

NS_LOG_COMPONENT_DEFINE("convacc_proto");

int
main(int argc, char* argv[])
{
    bool logging = false;
    if (logging)
    {
        LogComponentEnable("netw", LOG_LEVEL_INFO);
    }

    // name_input_files fd_setup_wrap;
    // read_setup (fd_setup_wrap);

    // netw netw_meta (fd_setup_wrap.netw_filename, fd_setup_wrap.demands_file,
    // fd_setup_wrap.file_overlay_nodes, fd_setup_wrap.route_name);
    // netw netw_meta (fd_setup_wrap);

    // Set simulation time and mobility
    // double simTime = 1; // seconds
    // double udpAppStartTime = 0.4; //seconds
    double stop_time = 100.0; // ms

    /**
     * UE Nodes
     *
     */
    int numUEs = 4;
    NodeContainer ueNodes;
    ueNodes.Create(numUEs);

    /**
     * Base stations (gNBs)
     */
    int numGnbs = 3;
    NodeContainer gnbNodes;
    gnbNodes.Create(numGnbs);

    /**
     * Routers in ISP1
     */
    int numRouters1 = 3;
    NodeContainer router1Nodes;
    router1Nodes.Create(numRouters1);

    /**
     * Routers in ISP2
     */
    int numRouters2 = 2;
    NodeContainer router2Nodes;
    router2Nodes.Create(numRouters2);

    /**
     * Host
     */
    NodeContainer hostNode;
    hostNode.Create(1);

    // Add IP/TCP/UCP functionality to existing nodes.
    InternetStackHelper internet;
    // internet.Install(ueNodes);
    // internet.Install(gnbNodes);
    internet.Install(router1Nodes);
    internet.Install(router2Nodes);
    internet.Install(hostNode);

    /**
     * Create links between the nodes.
     */
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("1ms"));

    // UE1 to BS1 link (need to replace with wireless link)
    NetDeviceContainer p2pDevices1;
    NodeContainer ueGnb1;
    ueGnb1.Add(ueNodes.Get(0));
    ueGnb1.Add(gnbNodes.Get(0));
    p2pDevices1 = p2p.Install(ueGnb1);

    // UE2 to BS2 link (need to replace with wireless link)
    NetDeviceContainer p2pDevices2;
    NodeContainer ueGnb2;
    ueGnb2.Add(ueNodes.Get(1));
    ueGnb2.Add(gnbNodes.Get(1));
    p2pDevices2 = p2p.Install(ueGnb2);

    // UE3 to BS2 link (need to replace with wireless link)
    NetDeviceContainer p2pDevices3;
    NodeContainer ueGnb3;
    ueGnb3.Add(ueNodes.Get(2));
    ueGnb3.Add(gnbNodes.Get(1));
    p2pDevices3 = p2p.Install(ueGnb3);

    // UE4 to BS3 link (need to replace with wireless link)
    NetDeviceContainer p2pDevices4;
    NodeContainer ueGnb4;
    ueGnb4.Add(ueNodes.Get(3));
    ueGnb4.Add(gnbNodes.Get(2));
    p2pDevices4 = p2p.Install(ueGnb4);

    // gNBs to UPFs
    // ???

    // UPFs to ISPs
    // ???

    // Router Links in ISP1
    NetDeviceContainer isp1;
    NodeContainer router1a_router1b;
    router1a_router1b.Add(router1Nodes.Get(0));
    router1a_router1b.Add(router1Nodes.Get(1));
    isp1 = p2p.Install(router1a_router1b);
    NodeContainer router1a_router1c;
    router1a_router1c.Add(router1Nodes.Get(0));
    router1a_router1c.Add(router1Nodes.Get(2));
    isp1 = p2p.Install(router1a_router1c);
    NodeContainer router1b_router1c;
    router1b_router1c.Add(router1Nodes.Get(1));
    router1b_router1c.Add(router1Nodes.Get(2));
    isp1 = p2p.Install(router1b_router1c);

    // Router Links in ISP2
    NetDeviceContainer isp2;
    isp2 = p2p.Install(router2Nodes);

    // Link between ISP1 and ISP2
    NetDeviceContainer p2pDevices7;
    NodeContainer isp1And2;
    isp1And2.Add(router1Nodes.Get(2));
    isp1And2.Add(router2Nodes.Get(0));
    p2pDevices7 = p2p.Install(isp1And2);

    // Link between ISP1 and host
    NetDeviceContainer p2pDevices8;
    NodeContainer isp1AndHost;
    isp1AndHost.Add(router1Nodes.Get(0));
    isp1AndHost.Add(hostNode.Get(0));
    p2pDevices8 = p2p.Install(isp1AndHost);

    /**
     * Install Applications (Yudi's)
     **/
    std::vector<Ptr<overlayApplication>> vec_app(13); // 13 nodes
    // ObjectFactory fact;
    // fact.SetTypeId("ns3::overlayApplication");
    // fact.Set("RemotePort", UintegerValue(LISTENPORT));
    // fact.Set("ListenPort", UintegerValue(LISTENPORT));
    // fact.Set("probe_interval", TimeValue(MicroSeconds(200.0)));
    // fact.Set("sandwich_interval", TimeValue(MicroSeconds(100.0))); // Interval between the first
    // and second patch of the sandwich
    // // fact.Set ("MaxPackets", UintegerValue (1));
    // // fact.Set("PacketSize", UintegerValue(netw_meta._AppPktSize));
    // double AppStartTime = 21200;
    // for (uint32_t i = 0; i < netw_meta.n_nodes; i++)
    // {
    //     vec_app[i] = fact.Create<overlayApplication>();
    //     vec_app[i]->InitApp(&netw_meta, i, netw_meta._MAXPKTNUM);
    //     vec_app[i]->SetStartTime(MicroSeconds(AppStartTime));
    //     // vec_app[i]->SetStopTime(MilliSeconds(stop_time));
    //     underlayNodes.Get(i)->AddApplication(vec_app[i]);
    //     vec_app[i]->SetRecvSocket();
    // }

    // std::vector<PointToPointHelper> links(netw_meta.delay.size());
    // std::vector<NetDeviceContainer> NetDevices(netw_meta.delay.size());
    // std::vector<Ptr<Ipv4>> linkIpv4(2);
    // std::vector<Ipv4InterfaceAddress> linkIpv4Addr(2);
    // std::vector<uint32_t> n_devices_perNode(2);
    // std::vector<Ptr<Node>> endnodes(2);

    // for (uint32_t i = 0; i < links.size(); i++)
    // {
    //     links[i].DisableFlowControl();
    //     links[i].SetChannelAttribute("Delay", StringValue(std::to_string(netw_meta.delay[i]) +
    //     "us")); links[i].SetDeviceAttribute("DataRate",
    //     StringValue(std::to_string(netw_meta.bw[i]) + "kbps"));
    //     links[i].SetQueue("ns3::DropTailQueue", "MaxSize",
    //     QueueSizeValue(QueueSize(QueueSizeUnit::PACKETS, netw_meta._MAXBACKLOG)));
    //     // links[i].;
    //     NetDevices[i] = links[i].Install(underlayNodes.Get(netw_meta.edges_vec[i].first),
    //     underlayNodes.Get(netw_meta.edges_vec[i].second)); address.Assign(NetDevices[i]);
    //     address.NewNetwork();

    //     endnodes[0] = underlayNodes.Get(netw_meta.edges_vec[i].first);
    //     endnodes[1] = underlayNodes.Get(netw_meta.edges_vec[i].second);
    //     for (int k = 0; k < 2; k++)
    //     {
    //         linkIpv4[k] = endnodes[k]->GetObject<Ipv4>();
    //         n_devices_perNode[k] = endnodes[k]->GetNDevices();
    //         linkIpv4Addr[k] = linkIpv4[k]->GetAddress(n_devices_perNode[k] - 1, 0);
    //     }
    //     vec_app[netw_meta.edges_vec[i].first]->SetSocket(linkIpv4Addr[1].GetAddress(),
    //     netw_meta.edges_vec[i].second, n_devices_perNode[0] - 1); std::cout <<
    //     netw_meta.edges_vec[i].first << ": " << linkIpv4Addr[1].GetAddress() << std::endl;
    //     vec_app[netw_meta.edges_vec[i].second]->SetSocket(linkIpv4Addr[0].GetAddress(),
    //     netw_meta.edges_vec[i].first, n_devices_perNode[1] - 1); std::cout <<
    //     netw_meta.edges_vec[i].second << ": " << linkIpv4Addr[0].GetAddress() << std::endl;
    // }

    /**
     * Set IP addresses for the nodes.
     */
    Ipv4AddressHelper address;
    
    // BS Subnets
    // address.SetBase("10.1.1.0", "255.255.255.0");
    // Ipv4InterfaceContainer bsInterface1;
    // bsInterface1 = address.Assign(p2pDevices1);
    // address.SetBase("10.1.2.0", "255.255.255.0");
    // Ipv4InterfaceContainer bsInterface2;
    // bsInterface2 = address.Assign(p2pDevices2);
    // Ipv4InterfaceContainer bsInterface3;
    // bsInterface3 = address.Assign(p2pDevices3);
    // address.SetBase("10.1.3.0", "255.255.255.0");
    // Ipv4InterfaceContainer bsInterface4;
    // bsInterface4 = address.Assign(p2pDevices4);

    // ISP 1 and 2 Subnets
    address.SetBase("10.1.5.0", "255.255.255.0");
    Ipv4InterfaceContainer ispInterface1;
    ispInterface1 = address.Assign(isp1);
    address.SetBase("10.1.6.0", "255.255.255.0");
    Ipv4InterfaceContainer ispInterface2;
    ispInterface2 = address.Assign(isp2);

    // Host IP address
    address.SetBase("10.1.7.0", "255.255.255.0");
    Ipv4InterfaceContainer hostInterface;
    hostInterface = address.Assign(p2pDevices8);

    /**
     * Install Applications
     **/
    UdpEchoServerHelper echoServer(9);

    ApplicationContainer serverApps = echoServer.Install(hostNode);
    serverApps.Start(Seconds(0.0));
    serverApps.Stop(Seconds(10.0));

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();


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
    uePositionAlloc->Add(Vector(10, 20, 0));
    uePositionAlloc->Add(Vector(20, 30, 0));
    uePositionAlloc->Add(Vector(30, 40, 0));
    uePositionAlloc->Add(Vector(40, 50, 0));
    mob.SetPositionAllocator(uePositionAlloc);
    mob.Install(ueNodes);

    Ptr<ListPositionAllocator> gnbPositionAlloc = CreateObject<ListPositionAllocator>();
    gnbPositionAlloc->Add(Vector(20, 10, 0));
    gnbPositionAlloc->Add(Vector(30, 20, 0));
    gnbPositionAlloc->Add(Vector(40, 30, 0));
    mob.SetPositionAllocator(gnbPositionAlloc);
    mob.Install(gnbNodes);

    Ptr<ListPositionAllocator> router1PositionAlloc = CreateObject<ListPositionAllocator>();
    router1PositionAlloc->Add(Vector(80, 20, 0));
    router1PositionAlloc->Add(Vector(70, 30, 0));
    router1PositionAlloc->Add(Vector(60, 20, 0));
    mob.SetPositionAllocator(router1PositionAlloc);
    mob.Install(router1Nodes);

    Ptr<ListPositionAllocator> router2PositionAlloc = CreateObject<ListPositionAllocator>();
    router2PositionAlloc->Add(Vector(50, 10, 0));
    router2PositionAlloc->Add(Vector(40, 10, 0));
    mob.SetPositionAllocator(router2PositionAlloc);
    mob.Install(router2Nodes);

    Ptr<ListPositionAllocator> hostPositionAlloc = CreateObject<ListPositionAllocator>();
    hostPositionAlloc->Add(Vector(90, 20, 0));
    mob.SetPositionAllocator(hostPositionAlloc);
    mob.Install(hostNode);


    //Ptr<NrPointToPointEpcHelper> epcHelper = CreateObject<NrPointToPointEpcHelper> (network_base_number);
    // ^^ERROR: NrP2PEH constructor does not accept any arguments. Not w/o making mods to it.
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
    // uint32_t bwpIdForVoice = 1;

    nrHelper->SetGnbBwpManagerAlgorithmAttribute ("NGBR_LOW_LAT_EMBB", UintegerValue (bwpIdForLowLat));
    // nrHelper->SetGnbBwpManagerAlgorithmAttribute ("GBR_CONV_VOICE", UintegerValue (bwpIdForVoice));

    //Install and get the pointers to the NetDevices
    NetDeviceContainer enbNetDev = nrHelper->InstallGnbDevice (gnbNodes, allBwps);
    NetDeviceContainer ueNetDev = nrHelper->InstallUeDevice (ueNodes, allBwps);
    // ^^ERROR in line 394 & 395: msg="InternetStackHelper::Install (): Aggregating an InternetStack to a node with an existing Ipv4 object"

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

    Ptr<Node> pgw = epcHelper->GetPgwNode ();
    NodeContainer pgwContainer;
    pgwContainer.Add(pgw);
    std::vector<PointToPointHelper> vec_p2ph(13); //13 - vec_app.size()
    
    NetDeviceContainer internetDevices;
    Ipv4AddressHelper ipv4h;
    Ipv4StaticRoutingHelper ipv4RoutingHelper;

    const std::string p2pPgwEpc{std::to_string(network_base_number+1) + ".0.0.0"};
    ipv4h.SetBase (Ipv4Address(p2pPgwEpc.data()), "255.0.0.0");
    const std::string Addr_IPv4_Network_gNB {std::to_string(network_base_number) + ".0.0.0"};
    
    for (uint32_t j = 0; j < vec_app.size(); j++)
    {
        // if (netw_meta.set_tb.count(j) == 0) continue;
        // if (!netw_meta.is_w_probing && netw_meta.set_tb.count(j) == 0) continue;
        Ptr<Node> remoteHost = vec_app[j]->GetNode(); // ERROR: Attempt to dereference 0 pointer.
        // connect a remoteHost to pgw. Set up routing too
        vec_p2ph[j].SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("1000Gb/s")));
        vec_p2ph[j].SetDeviceAttribute ("Mtu", UintegerValue (10000));
        vec_p2ph[j].SetChannelAttribute ("Delay", TimeValue (Seconds (0.000)));
        vec_p2ph[j].DisableFlowControl();
        NetDeviceContainer tmp_NetDeviceContainer = vec_p2ph[j].Install (pgw, remoteHost);
        // std::cout << tmp_NetDeviceContainer.Get(0) << " " << tmp_NetDeviceContainer.Get(1) << std::endl;
        internetDevices.Add( tmp_NetDeviceContainer );
        Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
        // remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);
        remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address (Addr_IPv4_Network_gNB.data()), Ipv4Mask ("255.0.0.0"), remoteHost->GetNDevices()-1);
    }
    Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);

    // internet.Install (ueNodes);
    // Ipv4InterfaceContainer ueIpIface;
    // ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueNetDev));
    // // Set the default gateway for the UEs
    // for (uint32_t j = 0; j < ueNodes.GetN (); ++j)
    // {
    //     Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNodes.Get (j)->GetObject<Ipv4> ());
    //     ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    // }
    // // attach UEs to the closest eNB before creating the dedicated flows
    // nrHelper->AttachToClosestEnb (ueNetDev, enbNetDev);

    // // The bearer that will carry CONV_VOICE traffic
    // uint16_t dlPort_bwp0 = NRPORT;
    // EpsBearer bearer_0 (EpsBearer::NGBR_LOW_LAT_EMBB);

    // Ptr<EpcTft> tft_0 = Create<EpcTft> ();
    // EpcTft::PacketFilter dlpf_0;
    // dlpf_0.localPortStart = dlPort_bwp0;
    // dlpf_0.localPortEnd = dlPort_bwp0;
    // tft_0->Add (dlpf_0);

    // // The bearer that will carry low latency traffic
    // uint16_t dlPort_bwp1 = NRPORT + 1;
    // EpsBearer bearer_1 (EpsBearer::GBR_CONV_VOICE);

    // Ptr<EpcTft> tft_1 = Create<EpcTft> ();
    // EpcTft::PacketFilter dlpf_1;
    // dlpf_1.localPortStart = dlPort_bwp1;
    // dlpf_1.localPortEnd = dlPort_bwp1;
    // tft_1->Add (dlpf_1);

    // /** set the Recv Listen Socket for UE **/
    // // ObjectFactory fact;
    // fact.SetTypeId("ns3::ueApp");
    // std::vector<Ptr<ueApp>> vec_ue_app(ueNodes.GetN ());
    // // vec_ue_app.resize(ueNodes.GetN ());
    // uint32_t idx = 0;
    // for (uint32_t i = 0; i < netw_meta.n_nodes; i++)
    // {
    //     if (netw_meta.set_tb.count(i) == 0) continue;
    //     // if (!netw_meta.is_w_probing && netw_meta.set_tb.count(i) == 0) continue;
    //     /** Set Connection between RemoteHost and UEs **/
    //     vec_app[i]->nr_socket.resize(vec_UE[i].GetN());
    //     for (uint32_t j = 0; j < vec_UE[i].GetN(); j++)
    //     {
    //         Ptr<Node> ue = vec_UE[i].Get (j);
    //         vec_ue_app[idx] = fact.Create<ueApp>();
    //         // std::cout << "UE ID for " << app_interface.GetLocalID() << " is " << ue->GetId() << std::endl;
    //         ue->AddApplication(vec_ue_app[idx]);
    //         vec_ue_app[idx]->initUeApp(*vec_app[i]);
    //         Ptr<NetDevice> ueDevice = ueNetDev.Get (idx++);
            
    //         TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
    //         vec_app[i]->nr_socket[j] = Socket::CreateSocket(vec_app[i]->GetNode(), tid);
    //         if (vec_app[i]->nr_socket[j]->Bind() == -1) NS_FATAL_ERROR("Failed to bind socket");
    //         vec_app[i]->nr_socket[j]->Connect(InetSocketAddress(ue->GetObject<Ipv4> ()->GetAddress( 1, 0 ).GetLocal(), dlPort_bwp0));
    //         nrHelper->ActivateDedicatedEpsBearer (ueDevice, bearer_0, tft_0);

    //         /* if (j == 0)
    //         {
    //             vec_app[i]->nr_socket[j]->Connect(InetSocketAddress(ue->GetObject<Ipv4> ()->GetAddress( 1, 0 ).GetLocal(), dlPort_bwp0));
    //             nrHelper->ActivateDedicatedEpsBearer (ueDevice, bearer_0, tft_0);
    //         }
    //         else
    //         {   
    //             // std::cout << "activate for " << i << " with port = " << uint32_t(dlPort_bwp1) << std::endl;
    //             vec_app[i]->nr_socket[j]->Connect(InetSocketAddress(ue->GetObject<Ipv4> ()->GetAddress( 1, 0 ).GetLocal(), dlPort_bwp1));
    //             nrHelper->ActivateDedicatedEpsBearer (ueDevice, bearer_1, tft_1);
    //         } */

    //         // nrHelper->ActivateDedicatedEpsBearer (ueDevice, bearer_0, tft_0);
    //         // nrHelper->ActivateDedicatedEpsBearer (ueDevice, bearer_1, tft_1);
    //     }
    // }

    // End of 5G-Lena code.

    /**
     * Set IP addresses for the nodes.
     */
    // BS Subnets
    //Ipv4AddressHelper ipv4h; // already defined above
    // ipv4h.SetBase("10.1.1.0", "255.255.255.0");
    // Ipv4InterfaceContainer bsInterface1;
    // bsInterface1 = ipv4h.Assign(p2pDevices1);
    // ipv4h.SetBase("10.1.2.0", "255.255.255.0");
    // Ipv4InterfaceContainer bsInterface2;
    // bsInterface2 = ipv4h.Assign(p2pDevices2);
    // Ipv4InterfaceContainer bsInterface3;
    // bsInterface3 = ipv4h.Assign(p2pDevices3);
    // ipv4h.SetBase("10.1.3.0", "255.255.255.0");
    // Ipv4InterfaceContainer bsInterface4;
    // bsInterface4 = ipv4h.Assign(p2pDevices4);

    // // ISP 1 and 2 Subnets
    // ipv4h.SetBase("10.1.5.0", "255.255.255.0");
    // Ipv4InterfaceContainer ispInterface1;
    // ispInterface1 = ipv4h.Assign(isp1);
    // ipv4h.SetBase("10.1.6.0", "255.255.255.0");
    // Ipv4InterfaceContainer ispInterface2;
    // ispInterface2 = ipv4h.Assign(isp2);

    // // Host IP address
    // ipv4h.SetBase("10.1.7.0", "255.255.255.0");
    // Ipv4InterfaceContainer hostInterface;
    // hostInterface = ipv4h.Assign(p2pDevices8);

    // /**
    //  * Install Applications
    //  **/
    // UdpEchoServerHelper echoServer(9);

    // ApplicationContainer serverApps = echoServer.Install(hostNode);
    // serverApps.Start(Seconds(0.0));
    // serverApps.Stop(Seconds(5.0));

    // Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    /**
     * Set the positions of all of the nodes using the mobility module.
     */
    // MobilityHelper mob;
    // mob.SetMobilityModel("ns3::ConstantPositionMobilityModel");

    // Ptr<ListPositionAllocator> uePositionAlloc = CreateObject<ListPositionAllocator>();
    // uePositionAlloc->Add(Vector(10, 20, 0));
    // uePositionAlloc->Add(Vector(20, 30, 0));
    // uePositionAlloc->Add(Vector(30, 40, 0));
    // uePositionAlloc->Add(Vector(40, 50, 0));
    // mob.SetPositionAllocator(uePositionAlloc);
    // mob.Install(ueNodes);

    // Ptr<ListPositionAllocator> gnbPositionAlloc = CreateObject<ListPositionAllocator>();
    // gnbPositionAlloc->Add(Vector(20, 10, 0));
    // gnbPositionAlloc->Add(Vector(30, 20, 0));
    // gnbPositionAlloc->Add(Vector(40, 30, 0));
    // mob.SetPositionAllocator(gnbPositionAlloc);
    // mob.Install(gnbNodes);

    // Ptr<ListPositionAllocator> router1PositionAlloc = CreateObject<ListPositionAllocator>();
    // router1PositionAlloc->Add(Vector(80, 20, 0));
    // router1PositionAlloc->Add(Vector(70, 30, 0));
    // router1PositionAlloc->Add(Vector(60, 20, 0));
    // mob.SetPositionAllocator(router1PositionAlloc);
    // mob.Install(router1Nodes);

    // Ptr<ListPositionAllocator> router2PositionAlloc = CreateObject<ListPositionAllocator>();
    // router2PositionAlloc->Add(Vector(50, 10, 0));
    // router2PositionAlloc->Add(Vector(40, 10, 0));
    // mob.SetPositionAllocator(router2PositionAlloc);
    // mob.Install(router2Nodes);

    // Ptr<ListPositionAllocator> hostPositionAlloc = CreateObject<ListPositionAllocator>();
    // hostPositionAlloc->Add(Vector(90, 20, 0));
    // mob.SetPositionAllocator(hostPositionAlloc);
    // mob.Install(hostNode);

    Ptr<ListPositionAllocator> upfPositionAlloc = CreateObject<ListPositionAllocator>();
    upfPositionAlloc->Add(Vector(30, 30, 0));
    mob.SetPositionAllocator(upfPositionAlloc);
    mob.Install(pgwContainer);
    

    // Animate the network in NetAnim
    AnimationInterface anim("convacc-proto-animation.xml");

    /**
     * Name NetAnim Nodes
     */
    // Name UE Nodes
    anim.UpdateNodeDescription(0, "MEC1");
    anim.UpdateNodeDescription(1, "MEC2");
    anim.UpdateNodeDescription(2, "MEC3");
    anim.UpdateNodeDescription(3, "MEC4");

    // Name BS Nodes
    anim.UpdateNodeDescription(4, "BS1");
    anim.UpdateNodeDescription(5, "BS2");
    anim.UpdateNodeDescription(6, "BS3");

    // Name ISP1 Nodes
    anim.UpdateNodeDescription(7, "R1a");
    anim.UpdateNodeDescription(8, "R1b");
    anim.UpdateNodeDescription(9, "R1c");

    // Name ISP2 Nodes
    anim.UpdateNodeDescription(10, "R2a");
    anim.UpdateNodeDescription(11, "R2b");

    // Name Host Node
    anim.UpdateNodeDescription(12, "Host");

    // Name UPF Node
    anim.UpdateNodeDescription(13, "UPF");

    /**
     * Color NetAnim Nodes
     */
    // Color UE Nodes Green
    anim.UpdateNodeColor(0, 0, 200, 0);
    anim.UpdateNodeColor(1, 0, 200, 0);
    anim.UpdateNodeColor(2, 0, 200, 0);
    anim.UpdateNodeColor(3, 0, 200, 0);

    // Color BS Nodes Yellow
    anim.UpdateNodeColor(4, 255, 200, 0);
    anim.UpdateNodeColor(5, 255, 200, 0);
    anim.UpdateNodeColor(6, 255, 200, 0);

    // Color ISP1 Nodes Red
    anim.UpdateNodeColor(7, 255, 0, 0);
    anim.UpdateNodeColor(8, 255, 0, 0);
    anim.UpdateNodeColor(9, 255, 0, 0);

    // Color ISP2 Nodes Red
    anim.UpdateNodeColor(10, 255, 0, 0);
    anim.UpdateNodeColor(11, 255, 0, 0);

    // Color Host Node Blue
    anim.UpdateNodeColor(12, 0, 0, 255);

    /**
     * Resize NetAnim Nodes by 3.
     */
    for (int i = 0; i < 13; i++)
    {
        anim.UpdateNodeSize(i, 3, 3);
    }

    // anim.SetStartTime(Seconds(10));
    // anim.SetStopTime(Seconds(20));

    /**
     * Run Simulation
    */
    NS_LOG_INFO("Run Simulation.");
    std::cout << "before run" << std::endl;
    Time time_stop_simulation = MilliSeconds(stop_time*1.2);
    // Simulator::Schedule(time_stop_simulation, stop_NR, vec_NrHelper);
    Simulator::Stop(time_stop_simulation);
    Simulator::Run();
}