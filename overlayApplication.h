#ifndef OVERLAY_APPLICATION_H
#define OVERLAY_APPLICATION_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"
#include "ns3/traced-callback.h"
#include "ns3/udp-socket.h"
//#include "SDtag.h"
#include <vector>
#include "utils.h"
//#include "netw.h"

namespace ns3
{

class Socket; // May need in the future to prevent an error.
class Packet;

class overlayApplication : public Application
{
public:
    u_int16_t n_nodes = 13;

    static TypeId GetTypeId(void);
    virtual TypeId GetInstanceTypeId(void) const;

    /** Init **/
    overlayApplication();
    virtual ~overlayApplication();
    //void InitApp(netw* meta, uint32_t localId, uint32_t MaxPktSize); // don't need meta class for now
    void InitApp(uint32_t localId); //, uint32_t MaxPktSize);
    void SetLocalID(uint32_t localID);
    uint32_t GetLocalID(void) const;

    /** Connection **/
    void SetSocket(Address ip, uint32_t idx, uint32_t deviceID);
    void SetRecvSocket(void);
    
    /** Functions **/
    // Need to implement HandleRead() & CheckCongestion() later.
    void HandleRead(Ptr<Socket> socket);
    // bool CheckCongestion(uint32_t deviceID, uint32_t src, uint32_t dest, uint16_t PktID);

    // Do later:
    // bool is_overlay; // Don't think I need this
    std::vector<Ptr<Socket>> nr_socket;
    // netw* meta;

    // void ScheduleProbing(Time dt, uint32_t idx);
    // void CentralOrchestration();
    // void SendProbeNaive(uint32_t idx);
    // bool StateCheckRecv();
    // void Send_Attack_Flow(uint32_t idx);

protected:
    virtual void DoDispose(void);
private:
    virtual void StartApplication(void);
    virtual void StopApplication(void);


    // Do later:
    // void SetTag(SDtag& tagToUse, uint8_t SourceID, uint8_t DestID,
    //     uint8_t currentHop = 1, uint32_t PktID = 0, uint8_t IsProbe = 0, uint8_t,
    //     IsQueued = 0, uint8_t SandWichID = 10, uint8_t SandWichLargeID=0);

    // void ScheduleBackground(Time dt, uint32_t idx);
    // void SendBackground(uint32_t idx);
    // uint32_t GMM_Pkt_Size(void);

    // Do later:
    /** probing **/
    // Time probe_interval; // probe interval
    // Time sandwich_interval; // Interval for the sandwich probing -- microsecond (us, 1e-6) 
    // std::vector<EventId> m_sendEvent; // background traffic
    // std::vector<EventId> probe_event;
    // Ptr<ParetoRandomVariable> rand_burst_pareto;

    /** connection **/
    uint16_t m_peerPort;
    uint16_t ListenPort;
    std::vector<Ptr<Socket>> tab_socket;
    Ptr<Socket> recv_socket;
    std::unordered_map<uint32_t, uint32_t> map_neighbor_device; // <idx_neighbor, deviceID>
    
    /** Basic Meta **/
    //bool is_run = true;
    uint16_t m_local_ID;
};

}

#endif