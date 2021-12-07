#include "BaseCommMode.h"
#include "artery/traci/VehicleController.h"
#include <omnetpp/cpacket.h>
#include <vanetza/btp/data_request.hpp>
#include <vanetza/dcc/profile.hpp>
#include <vanetza/geonet/interface.hpp>

#include <boost/optional/optional_io.hpp>

using namespace omnetpp;
using namespace vanetza;


Define_Module(BaseCommMode)

BaseCommMode::BaseCommMode()
{
}


void BaseCommMode::initialize()
{
	
}

void BaseCommMode::finish()
{
}

void BaseCommMode::handleMessage(cMessage* msg)
{
	Enter_Method("handleMessage");

	// if (msg == m_self_msg) {
	// 	EV_INFO << "PlatooningService: self message\n";
	// }
	// else if (msg->getArrivalGateId() == serviceIn) {
	// }
	// else if (msg->getArrivalGateId() == serviceOut) {
	// 	sendPlatooningUnicast(msg);
	// }

	//delete msg;
}