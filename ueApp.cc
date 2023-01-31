#include "ueApp.h"

namespace ns3
{


NS_LOG_COMPONENT_DEFINE("ueApp");
NS_OBJECT_ENSURE_REGISTERED(ueApp);

TypeId ueApp::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::ueApp")
                            .SetParent<Application>()
                            .SetGroupName("Applications")
                            .AddConstructor<ueApp>();
    return tid;
}

TypeId ueApp::GetInstanceTypeId (void) const
{
  	return ueApp::GetTypeId ();
}

// Constructor
ueApp::ueApp()
{
    NS_LOG_FUNCTION(this);
}
void ueApp::initUeApp(overlayApplication &app_interface)
{
    local_ID_ = app_interface.GetLocalID();
    oa_interface = &app_interface;
    TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
    recv_socket = Socket::CreateSocket(GetNode(), tid);
    recv_socket_1 = Socket::CreateSocket(GetNode(), tid);
    InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), NRPORT);
    InetSocketAddress local_1 = InetSocketAddress(Ipv4Address::GetAny(), NRPORT+1);
    if (recv_socket->Bind(local) == -1)
    {
        std::cout << "Failed to bind socket" << std::endl;
        // NS_FATAL_ERROR("Failed to bind socket 0");
    }
    if (recv_socket_1->Bind(local_1) == -1)
    {
        std::cout << "Failed to bind socket" << std::endl;
        // NS_FATAL_ERROR("Failed to bind socket 1");
    }

    recv_socket->SetRecvCallback(MakeCallback(&ueApp::HandleRead, this));
    recv_socket_1->SetRecvCallback(MakeCallback(&ueApp::HandleRead, this));

    //max_probes = app_interface.meta->_MAXPKTNUM; // add later
}

// Destructor
ueApp::~ueApp()
{
    NS_LOG_FUNCTION(this);
}

void ueApp::HandleRead(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this << socket);

    // Add packet reading later.
}

void ueApp::StartApplication(void)
{
    // std::cout << "UE_App Start at: " << local_ID_ << " " << std::endl;
    NS_LOG_FUNCTION(this);
}

void ueApp::StopApplication(void)
{
    // std::cout << "UE ID: " << local_ID_ << "; Stopped" << std::endl;
    recv_socket->Close();
    recv_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
    NS_LOG_FUNCTION(this);
}

}