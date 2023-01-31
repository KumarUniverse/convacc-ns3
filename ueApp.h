#ifndef UEAPP_H
#define UEAPP_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"
#include "ns3/traced-callback.h"
#include "ns3/udp-socket.h"
#include "ns3/log.h"
#include "overlayApplication.h"
#include "SDtag.h"

namespace ns3
{

#define NRPORT 1234

class ueApp : public Application
{
public:
    static TypeId GetTypeId(void);
    virtual TypeId GetInstanceTypeId(void) const;
    ueApp();
    void initUeApp(overlayApplication &app_interface);
    virtual ~ueApp();

    /** Functions **/
    void HandleRead(Ptr<Socket> socket);
private:
    uint32_t local_ID_;
    // uint32_t cnt_probes = 0; // add later
    // uint32_t max_probes;
    Ptr<Socket> recv_socket;
    Ptr<Socket> recv_socket_1;
    overlayApplication *oa_interface;
    virtual void StartApplication(void);
    virtual void StopApplication(void);
};

}

#endif