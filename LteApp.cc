#include "LteApp.h"
#include "hybrid_msgs/HybridServicesMessagesLTE_m.h"
#include "hybrid_msgs/HybridServicesMessages_m.h"
#include "artery/application/Middleware.h"
#include "artery/application/StoryboardSignal.h"
#include "artery/traci/VehicleController.h"
#include "artery/utility/PointerCheck.h"
#include <inet/common/ModuleAccess.h>
#include <inet/networklayer/common/L3AddressResolver.h>
#include <omnetpp/checkandcast.h>

using namespace omnetpp;

Define_Module(LteApp)

static const simsignal_t lteSignal = cComponent::registerSignal("LteSignal");


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
    getParentModule()->subscribe(lteSignal, this);

    // LTE multicast support
    inet::IInterfaceTable *ift = inet::getModuleFromPar<inet::IInterfaceTable>(par("interfaceTableModule"), this);
    inet::InterfaceEntry *ie = ift->getInterfaceByName("wlan");
    if (!ie)
        throw cRuntimeError("Wrong multicastInterface setting: no interface named wlan");
    socket.setMulticastOutputInterface(ie->getInterfaceId());
    socket.joinMulticastGroup(mcastAddress);

    // application's supporting code
    auto mw = inet::getModuleFromPar<artery::Middleware>(par("middlewareModule"), this);
    vehicleController = artery::notNullPtr(mw->getFacilities().get_mutable_ptr<traci::VehicleController>());

    // application logic
    
    messageReceived = 0;
    WATCH(messageReceived);

    //omnetpp::cMessage trigger = new omnetpp::cMessage("send message");
    //scheduleAt(simTime() + 1 trigger);

}

void LteApp::finish()
{
    socket.close();
    recordScalar("number of ITS-G5 message received", messageReceived);
}

void LteApp::receiveSignal(cComponent*, simsignal_t sig, cObject* obj, cObject*)
{
    if (sig == lteSignal) {
        auto sigMessage = check_and_cast<PlatooningMessage*>(obj);
        EV << "ITS-G5 signal received " << "// id = " << sigMessage->getVehicleId() << "\n";
        ++messageReceived;
        
        /*auto platooningLteMessage = new PlatooningMessage();
        
        platooningLteMessage->setEdgeName(sigMessage->getEdgeName());
        platooningLteMessage->setVehicleId(sigMessage->getVehicleId());
        platooningLteMessage->setLaneIndex(sigMessage->getLaneIndex());
        platooningLteMessage->setSpeed(sigMessage->getSpeed());
        platooningLteMessage->setTime(sigMessage->getTime());  */  
        
        sendV2XMessage(sigMessage);
    }
}

void LteApp::handleMessage(cMessage* msg)
{
    /*if (msg->isSelfMessage()) {
        //vehicleController->setSpeedFactor(1.0);
    } else */
    if (msg->getKind() == inet::UDP_I_DATA) {
        processV2XMessage(*check_and_cast<PlatooningMessage*>(msg));
        delete msg;
    } else {
        throw cRuntimeError("Unrecognized message (%s)%s", msg->getClassName(), msg->getName());
    }
}

void LteApp::sendV2XMessage(PlatooningMessage* packet)
{
    Enter_Method_Silent();
    //maybe add some attributs
    socket.sendTo(packet, mcastAddress, par("mcastPort"));
}

void LteApp::processV2XMessage(PlatooningMessage& packet)
{
    EV << "Message Received from LTE " << "\n";
}
