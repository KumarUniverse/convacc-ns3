#include "overlayApplication.h"
#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/net-device.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/queue.h"
#include "ns3/uinteger.h"
#include "ns3/random-variable-stream.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/point-to-point-module.h"
//#include "SDtag.h"
#include <assert.h>
//#include "netw.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("overlayApplication");
NS_OBJECT_ENSURE_REGISTERED(overlayApplication);

TypeId overlayApplication::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::overlayApplication")
                            .SetParent<Application>()
                            .SetGroupName("Applications")
                            .AddConstructor<overlayApplication>()
                            .AddAttribute("RemotePort",
                                            "The destination port of the outbound packets",
                                            UintegerValue(0),
                                            MakeUintegerAccessor(&overlayApplication::m_peerPort),
                                            MakeUintegerChecker<uint16_t>())
                            .AddAttribute("ListenPort", "Port on which we listen for incoming packets.",
                                            UintegerValue(0),
                                            MakeUintegerAccessor(&overlayApplication::ListenPort),
                                            MakeUintegerChecker<uint16_t>());

    return tid;
}

TypeId overlayApplication::GetInstanceTypeId (void) const
{
  	return overlayApplication::GetTypeId ();
}

// Constructor
overlayApplication::overlayApplication()
{
    NS_LOG_FUNCTION(this);
}

// Destructor
overlayApplication::~overlayApplication()
{
    NS_LOG_FUNCTION(this);

    tab_socket.clear();
    recv_socket = 0;
    // m_sendEvent.clear(); // add later
    // probe_event.clear();
    map_neighbor_device.clear();
}

void overlayApplication::DoDispose(void)
{
    NS_LOG_FUNCTION(this);
    Application::DoDispose();
}

// App initialization method
void overlayApplication::InitApp(uint32_t localId) //, uint32_t MaxPktSize)
{
    tab_socket.resize(n_nodes, 0);
    m_peerPort = 9;
    recv_socket = 0; // null pointer
    // m_sendEvent.resize(meta->n_nodes, EventId());
    // probe_event.resize(meta->n_nodes, EventId());
    SetLocalID(localId);
    // rand_burst_pareto = CreateObject<ParetoRandomVariable>();
    // rand_burst_pareto->SetAttribute ("Scale", DoubleValue (meta->parato_scale));
    // rand_burst_pareto->SetAttribute ("Shape", DoubleValue (meta->parato_shape));
    // rand_burst_pareto->SetAttribute ("Bound", DoubleValue (meta->parato_bound));
}

void overlayApplication::SetLocalID(uint32_t localID)
{
    NS_LOG_FUNCTION(this);
    m_local_ID = localID;
}

uint32_t overlayApplication::GetLocalID(void) const
{
    NS_LOG_FUNCTION(this);
    return m_local_ID;
}

void overlayApplication::SetSocket(Address ip, uint32_t idx, uint32_t deviceID)
{
    NS_LOG_FUNCTION(this);
    if (tab_socket[idx] == 0)
    {
        TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
        tab_socket[idx] = Socket::CreateSocket(GetNode(), tid);
        if (Ipv4Address::IsMatchingType(ip) == true)
        {
            if (tab_socket[idx]->Bind() == -1)
            {
                NS_FATAL_ERROR("Failed to bind socket");
            }
            tab_socket[idx]->Connect(InetSocketAddress(Ipv4Address::ConvertFrom(ip), m_peerPort));
        }
        else if (InetSocketAddress::IsMatchingType(ip) == true)
        {
            if (tab_socket[idx]->Bind() == -1)
            {
                NS_FATAL_ERROR("Failed to bind socket");
            }
            tab_socket[idx]->Connect(ip);
        }
        else
        {
            NS_ASSERT_MSG(false, "Incompatible address type: " << ip);
        }
        tab_socket[idx]->SetAllowBroadcast(false);
        map_neighbor_device.insert(std::pair<uint32_t, uint32_t>(idx, deviceID));
    }
    else
    {
        std::cout << "create an existing socket" << std::endl;
    }
}

void overlayApplication::SetRecvSocket(void)
{
    /**
     * Set up socket for forwarding
     **/
    TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
    recv_socket = Socket::CreateSocket(GetNode(), tid);
    InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), ListenPort);
    if (recv_socket->Bind(local) == -1)
    {
        NS_FATAL_ERROR("Failed to bind socket");
    }

    recv_socket->SetRecvCallback(MakeCallback(&overlayApplication::HandleRead, this));
}

void overlayApplication::HandleRead(Ptr<Socket> socket)
{
    // Do nothing for now.

    // NS_LOG_FUNCTION(this << socket);

    // Ptr<Packet> packet;
    // Address from;
    // Address localAddress;
    // while ((packet = socket->RecvFrom(from)))
    // {
    //     socket->GetSockName(localAddress);
    //     // m_rxTrace(packet);
    //     // m_rxTraceWithAddresses(packet, from, localAddress);

    //     // std::cout << "Node ID: " << m_local_ID << "; pkt received" << std::endl;
    //     SDtag tagPktRecv;
    //     packet->PeekPacketTag(tagPktRecv);
    //     std::string keys{std::to_string(tagPktRecv.GetSourceID()) + ' ' + std::to_string(tagPktRecv.GetDestID())};
    //     // NS_LOG_INFO("Node ID: " << m_local_ID << "; pkt received -- " << keys);

    //     std::vector<int> &routes = meta->routing_map[keys];
    //     // meta->cnt_node_received_pkt[m_local_ID]++;
    //     // if (tagPktRecv.GetBwpID() > 2)
    //     // {
    //     //     std::cout << "This node = " << m_local_ID << " from " << uint32_t(tagPktRecv.GetSourceID()) << " to " << uint32_t(tagPktRecv.GetDestID()) << " with " << meta->cnt_node_attack_pkt[m_local_ID] << " at \t" << Now()  << std::endl;
    //     //     meta->cnt_node_attack_pkt[m_local_ID]++;
    //     // }

    //     if (tagPktRecv.GetDestID() == GetLocalID())
    //     {
    //         if (tagPktRecv.GetIsProbe() > 0)
    //         {
    //             /* if (tagPktRecv.GetSourceID() == SRC && tagPktRecv.GetDestID() == DEST)
    //             {
    //                 std::cout << SRC << " - " << DEST << " -PktID=" << tagPktRecv.GetPktID() << " received at: " << Simulator::Now().GetMicroSeconds() << std::endl;
    //                 // std::cout << SRC << " - " << DEST << " with large: " << 4 << " -PktID=" << tagPktRecv.GetPktID() << " sandwithID = " << (uint32_t)tagPktRecv.GetSandWichID() << ": " << Simulator::Now().GetMicroSeconds() << std::endl;
    //             } */
    //             switch (meta->probe_type)
    //             {
    //                 case ProbeType::naive:
    //                 {
    //                     // meta->cnt_queuing[keys][tagPktRecv.GetPktID()] = tagPktRecv.GetIsQueued();
    //                     /* uint32_t idx_tunnel = meta->tunnel_hashmap[keys];
    //                     meta->cnt_queuing[idx_tunnel][tagPktRecv.GetPktID()] = tagPktRecv.GetIsQueued(); */
    //                     // meta->cnt_true_delays[keys][tagPktRecv.GetPktID()] = Simulator::Now().GetNanoSeconds() - (uint64_t)(tagPktRecv.GetStartTime());
    //                     break;
    //                 }
    //                 case ProbeType::sandwich_v1:
    //                 {
                        
    //                     // if (tagPktRecv.GetSandWichID() == 1){}
    //                     // else meta->update_log_sandwich_v1(tagPktRecv.GetSourceID(), tagPktRecv.GetDestID(), tagPktRecv.GetSandWichLargeID(), tagPktRecv.GetPktID());
    //                     break;
    //                 }
    //                 case ProbeType::calibration:
    //                 {
    //                     break;
    //                 }
    //                 default:
    //                     break;
    //             }
    //             if (is_NR)
    //             {
    //                 // ueTag tag_ue_forward;
    //                 // tag_ue_forward.SetStartTime( (uint64_t)(Simulator::Now().GetMicroSeconds()) );
    //                 // // packet->RemovePacketTag(tagPktRecv);
    //                 // packet->AddPacketTag(tag_ue_forward);
    //                 // Ptr<Packet> new_p;
    //                 // new_p = Create<Packet>(meta->pkt_size_ran);
    //                 // m_txTrace(p);
    //                 // new_p->AddPacketTag(tagPktRecv);
    //                 // packet->ReplacePacketTag(tagPktRecv);
    //                 // std::cout << "Node ID: " << m_local_ID << "-Forwarding to UE, PktID = " << tagPktRecv.GetPktID() << ": " << uint32_t(tagPktRecv.GetUeID()) << " with start time" << tagPktRecv.GetStartTime() << " at " << "\t" << Now() << std::endl;
    //                 /* if ((tagPktRecv.GetPktID() == 0 || tagPktRecv.GetPktID() >= meta->_MAXPKTNUM-1 || tagPktRecv.GetPktID()%50 == 0 || tagPktRecv.GetPktID()%50 == 1 || tagPktRecv.GetPktID()%50 == 2) && tagPktRecv.GetUeID() == 0)
    //                 {
    //                     std::cout <<"pkt_ID = " << tagPktRecv.GetPktID() << "First forwarding time = " << "\t" << Now() << std::endl;
    //                 } */
                    
    //                 /* for (uint32_t i = 0; i < nr_socket.size(); i++)
    //                 {
    //                     // nr_socket[i]->Send(new_p);
    //                     nr_socket[i]->Send(packet);
    //                     // int nBytes = nr_socket[i]->Send(packet);
    //                     // std::cout << "nBytes = " << nBytes << std::endl;
    //                 } */
    //                 if ( meta->set_tb.count(m_local_ID) && tagPktRecv.GetUeID() == 0) 
    //                 // nr_socket[tagPktRecv.GetUeID()]->Send(packet);
    //                 {
    //                     for (uint32_t k = 0; k < 1; k++)
    //                     {
    //                         nr_socket[tagPktRecv.GetUeID()]->Send(packet);
    //                     }
    //                 }

    //             }
    //         }
    //         /* if (tagPktRecv.GetIsProbe() == 0)
    //         {
    //             std::cout << m_local_ID << ": recv background at " << Simulator::Now().As(Time::US) << " with " << keys << " with pkt_size = " << packet->GetSize() << std::endl; 
    //         } */

    //     }
    //     else
    //     {
    //         // std::cout << "Source ID: " << (uint32_t)tagPktRecv.GetSourceID() << ", target ID: " << (uint32_t)tagPktRecv.GetDestID() << ", this hop" << m_local_ID << ", next hop" << routes[tagPktRecv.GetCurrentHop() + 1] << std::endl;
    //         assert(routes[tagPktRecv.GetCurrentHop()] == m_local_ID);
    //         /* if (tagPktRecv.GetSourceID() == SRC && tagPktRecv.GetDestID() == DEST)
    //         {
    //             std::cout << "Node ID: " << m_local_ID << " forward at: " << Simulator::Now().ToDouble(Time::US) << std::endl;
    //         } */
            
    //         /* if ( CheckCongestion(map_neighbor_device[routes[tagPktRecv.GetCurrentHop()+1]], (uint32_t)tagPktRecv.GetSourceID(), (uint32_t)tagPktRecv.GetDestID(), (uint16_t)tagPktRecv.GetPktID()) )
    //         {
    //             tagPktRecv.SetIsQueued(1);
    //         } */
    //         tagPktRecv.AddCurrentHop();
    //         packet->ReplacePacketTag(tagPktRecv);
    //         tab_socket[routes[tagPktRecv.GetCurrentHop()]]->Send(packet);
    //     }
    // }
}

// void overlayApplication::CentralOrchestration()
// {
//     /**
//      * Used to centrally orchestrate the flowing of probes.
//     */
// }

void overlayApplication::StartApplication(void)
{
    NS_LOG_FUNCTION(this);
    /**
     * Set up background traffic
     **/
    
    // if (meta->background_type == CrossType::PktPoisson || meta->background_type == CrossType::ParetoBurst)
    // {
    //     // std::cout << "test background: " << meta->old_E.size() << " - " << meta->old_E[0] << " " << meta->old_E[1] << std::endl;
    //     for (uint32_t j = 0; j < meta->n_nodes; j++)
    //     {
    //         if (meta->adj_mat[m_local_ID][j] == false) continue;
    //         // std::cout << "test background: " << m_local_ID << " " << j << std::endl;
    //         Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
    //         Time random_offset = MicroSeconds(rand->GetValue(0, 50));
    //         ScheduleBackground(random_offset, j);
    //     }
    // }

    /**
     * Set up probing flows. TO DO
    */
}

void overlayApplication::StopApplication()
{
    NS_LOG_FUNCTION(this);

    // std::cout << "Node ID: " << m_local_ID << " stop Application" << std::endl;
    for (uint32_t i = 0; i < tab_socket.size(); i++)
    {
        // std::cout << "iter Node ID: " << m_local_ID << " i" << i << std::endl;
        if (tab_socket[i] != 0)
        {
            tab_socket[i]->Close();
            tab_socket[i]->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
        }
    }
    if (recv_socket != 0)
    {
        // std::cout << "iter Node ID: " << m_local_ID << " recv_socket" << std::endl;
        recv_socket->Close();
        recv_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
    }
    // std::cout << "iter Node ID: " << m_local_ID << " complete" << std::endl;
}

}