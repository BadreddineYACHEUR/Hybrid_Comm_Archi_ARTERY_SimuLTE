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

	emit(toHybridServiceSignal, check_and_cast<SeeThroughMessage*>(msg));
	

}

void HybridApp::receiveSignal(cComponent*, simsignal_t sig, cObject* obj, cObject*)
{
    if (sig == fromHybridServiceSignal) {

    	auto sigMessage = check_and_cast<cMessage*>(obj);
		auto seeThroughMessage = check_and_cast<SeeThroughMessage*>(obj);

        
        //std::cout << "message from fromHybridServiceSignal received " << sigMessage << " \n";
		if (seeThroughMessage->getSendingInterface() == 0)
			sendToITS_G5(sigMessage);
		else if (seeThroughMessage->getSendingInterface() == 1)
			sendToLTE_V2X(sigMessage);
		else 
			sendToSubApps(sigMessage, seeThroughMessage->getSendingInterface());
        delete sigMessage;
        
    }
    
}

void HybridApp::sendToSubApps (omnetpp::cMessage* msg, int hybrid_mode){

	Enter_Method("HybridApp sendToSubApps");

	//std::cout << "sending to Sub APPS \n" ;
	if(hybrid_mode == 3){
		send(msg->dup(), "hybridAppOut", 0);
		send(msg->dup(), "hybridAppOut", 1);
	}else {
		auto part_1_see_through_message = check_and_cast<SeeThroughMessage*>(msg->dup());
		part_1_see_through_message->setPartSize(part_1_see_through_message->getMessageSize() / 2);
		auto part_2_see_through_message = check_and_cast<SeeThroughMessage*>(msg->dup());
		part_2_see_through_message->setPartId(1);
		part_2_see_through_message->setPartSize(part_2_see_through_message->getMessageSize() / 2);

		send(part_1_see_through_message, "hybridAppOut", 0);
		send(part_2_see_through_message, "hybridAppOut", 1);
	}
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