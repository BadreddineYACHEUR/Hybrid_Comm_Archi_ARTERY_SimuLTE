#include "HybridApp.h"
#include "artery/traci/VehicleController.h"
#include "hybrid_msgs/HybridServicesMessages_m.h"
#include <omnetpp/cpacket.h>
#include <vanetza/btp/data_request.hpp>
#include <vanetza/dcc/profile.hpp>
#include <vanetza/geonet/interface.hpp>

#include <boost/optional/optional_io.hpp>

using namespace omnetpp;
using namespace vanetza;


Define_Module(HybridApp)

static const simsignal_t fromHybridServiceSignal = cComponent::registerSignal("toMainAppSignal");

HybridApp::HybridApp()
{
}


void HybridApp::initialize()
{
	int gateSize = par("num_apps");
	getParentModule()->subscribe(fromHybridServiceSignal, this);

	// Signals 
    toHybridServiceSignal = cComponent::registerSignal("toHybridServiceSignal");
}

void HybridApp::handleMessage(omnetpp::cMessage* msg){

	//std::cout << "Message " << msg << " arrived.\n";

	emit(toHybridServiceSignal, check_and_cast<PlatooningMessage*>(msg));

}

void HybridApp::receiveSignal(cComponent*, simsignal_t sig, cObject* obj, cObject*)
{
    if (sig == fromHybridServiceSignal) {

    	auto sigMessage = check_and_cast<cMessage*>(obj);
		auto platooningMessage = check_and_cast<PlatooningMessage*>(obj);

        
        //std::cout << "message from fromHybridServiceSignal received " << sigMessage << " \n";
		if (platooningMessage->getSending_interface() == 0)
			sendToITS_G5(sigMessage);
		else if (platooningMessage->getSending_interface() == 1)
			sendToLTE_V2X(sigMessage);
		else 
			sendToSubApps(sigMessage);
        delete sigMessage;
        
    }
    
}

void HybridApp::sendToSubApps (omnetpp::cMessage* msg){

	Enter_Method("HybridApp sendToSubApps");

	//std::cout << "sending to Sub APPS \n" ;

	send(msg->dup(), "hybridAppOut", 0);
	send(msg->dup(), "hybridAppOut", 1);

}

void HybridApp::sendToITS_G5 (omnetpp::cMessage* msg){

	Enter_Method("HybridApp sendToSubApps");

	//std::cout << "sending to Sub APPS \n" ;

	send(msg->dup(), "hybridAppOut", 0);

}

void HybridApp::sendToLTE_V2X (omnetpp::cMessage* msg){

	Enter_Method("HybridApp sendToSubApps");

	//std::cout << "sending to Sub APPS \n" ;

	send(msg->dup(), "hybridAppOut", 1);

}

void HybridApp::finish()
{
}