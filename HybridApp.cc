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

HybridApp::HybridApp()
{
}


void HybridApp::initialize()
{
	fromServiceIn = findGate("serviceOut");
	int gateSize = par("num_apps");
	//hybridAppIn[0] = 
	//hybridAppOut[0]
	
}

void HybridApp::handleMessage(omnetpp::cMessage* msg){
	std::cout << "Message " << msg << " arrived.\n";
}

void HybridApp::finish()
{
}