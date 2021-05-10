
#include "ItsG5App.h"
#include "hybrid_msgs/HybridServicesMessages_m.h"
#include "artery/traci/VehicleController.h"
#include <vanetza/btp/data_request.hpp>
#include <vanetza/dcc/profile.hpp>
#include <vanetza/geonet/interface.hpp>
#include <cmath>
#include <list>
#include "HybridService.h"

using namespace omnetpp;
using namespace vanetza;


Define_Module(HybridService)

HybridService::HybridService()
{
}

void HybridService::initialize()
{
	ItsG5Service::initialize();

	m_self_msg = new cMessage("Self Message");
	

	mVehicleController = &getFacilities().get_mutable<traci::VehicleController>();	
	vehicleId = mVehicleController->getVehicleId();

	serviceOut = findGate("serviceOut");
}



void HybridService::indicate(const btp::DataIndication& ind, cPacket* packet)
{

}



void HybridService::finish()
{
	cancelAndDelete(m_self_msg);
	ItsG5Service::finish();
}

void HybridService::handleMessage(cMessage* msg)
{
	Enter_Method("handleMessage");

	// if (msg == m_self_msg) {
	// 	EV_INFO << "HybridService: self message\n";
	// }
	// else if (msg->getArrivalGateId() == serviceIn) {
	// }
	// else if (msg->getArrivalGateId() == serviceOut) {
	// 	sendPlatooningUnicast(msg);
	// }

	delete msg;
}

void HybridService::trigger()
{
	Enter_Method("HybridService trigger");
	cMessage *msg = new cMessage("Test Gate COmmunication");
	sendDirect(msg, serviceOut);
}

void HybridService::receiveSignal(cComponent* source, simsignal_t signal, cObject*, cObject*)
{
}

void HybridService::sendToMainApp(cMessage* msg)
{
	

}


std::string HybridService::getVehicleId()
{
	return vehicleId;
}
