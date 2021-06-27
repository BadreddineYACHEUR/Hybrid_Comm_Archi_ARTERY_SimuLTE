#include "HybridApp.h"
#include "artery/traci/VehicleController.h"
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
	//hybridAppIn[gateSize] = gate("hybridAppIn");
	//hybridAppOut[0]
	
}

void HybridApp::handleMessage(omnetpp::cMessage* msg){
	std::cout << "Message " << msg << " arrived.\n";

	delete msg;
}

void HybridApp::receiveSignal(cComponent*, simsignal_t sig, cObject* obj, cObject*)
{
    if (sig == fromHybridServiceSignal) {

    	auto sigMessage = check_and_cast<cMessage*>(obj);
        
        std::cout << "message from fromHybridServiceSignal received " << sigMessage << " \n";

        sendToSubApps(sigMessage);
        delete sigMessage;
        
    }
    
}

void HybridApp::sendToSubApps (omnetpp::cMessage* msg){

	Enter_Method("HybridApp sendToSubApps");

	std::cout << "sending to Sub APPS \n" ;

	send(msg->dup(), "hybridAppOut", 0);
	send(msg->dup(), "hybridAppOut", 1);

}

void HybridApp::finish()
{
}