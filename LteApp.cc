#include "LteApp.h"
#include "hybrid_msgs/HybridServicesMessages_m.h"
#include "artery/application/Middleware.h"
#include "artery/traci/VehicleController.h"
#include "artery/utility/PointerCheck.h"
#include <inet/common/ModuleAccess.h>
#include <inet/networklayer/common/L3AddressResolver.h>
#include <omnetpp/checkandcast.h>

using namespace omnetpp;

Define_Module(LteApp)

static const simsignal_t lteSignal = cComponent::registerSignal("LteSignal");

static const simsignal_t fromSubAppSignalbis = cComponent::registerSignal("toLteSignal");



int LteApp::numInitStages() const
{
    return inet::NUM_INIT_STAGES;
}

void LteApp::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    if (stage != inet::INITSTAGE_APPLICATION_LAYER) {
        return;
    }

    mcastAddress = inet::L3AddressResolver().resolve(par("mcastAddress"));
    mcastPort = par("mcastPort");
    socket.setOutputGate(gate("udpOut"));
    socket.bind(mcastPort);
    
    // LTE multicast support
    inet::IInterfaceTable *ift = inet::getModuleFromPar<inet::IInterfaceTable>(par("interfaceTableModule"), this);
    inet::InterfaceEntry *ie = ift->getInterfaceByName("wlan");
    if (!ie)
        throw cRuntimeError("Wrong multicastInterface setting: no interface named wlan");
    socket.setMulticastOutputInterface(ie->getInterfaceId());
    socket.joinMulticastGroup(mcastAddress);

    // application's supporting code
    auto mw = inet::getModuleFromPar<artery::Middleware>(par("middlewareModule"), this);
    getParentModule()->subscribe(lteSignal, this);
    getParentModule()->subscribe(fromSubAppSignalbis, this);

    vehicleController = artery::notNullPtr(mw->getFacilities().get_mutable_ptr<traci::VehicleController>());

    //WATCH(messageReceived);

    lteToSubAppSignal = cComponent::registerSignal("lteToSubAppSignal");

}


void LteApp::receiveSignal(cComponent*, simsignal_t sig, cObject* obj, cObject*)
{
    if (sig == fromSubAppSignalbis){

        auto sigMessage = check_and_cast<PlatooningMessage*>(obj);
        
        //std::cout << "message fromSubAppSignal in LTE received " << sigMessage->getMessageId() << " \n";

        //Send message to Network
        sendV2XMessage(sigMessage);
        delete sigMessage;

    }
}

void LteApp::handleMessage(cMessage* msg)
{
    if (msg->getKind() == inet::UDP_I_DATA) {
        
        processV2XMessage(check_and_cast<PlatooningMessage*>(msg));
        delete msg;
    } else {
        throw cRuntimeError("Unrecognized message (%s)%s", msg->getClassName(), msg->getName());
    }
}

void LteApp::sendV2XMessage(PlatooningMessage* packet)
{
    Enter_Method_Silent();
    //maybe add some attributs
    socket.sendTo(packet->dup(), mcastAddress, par("mcastPort"));
}

void LteApp::processV2XMessage(PlatooningMessage* packet)
{
    //std::cout << "message in LTE received " << packet->getMessageId() << " \n";
    packet->setInterface(1);
    emit(lteToSubAppSignal, packet->dup());

    
}

void LteApp::finish()
{
    socket.close();
}