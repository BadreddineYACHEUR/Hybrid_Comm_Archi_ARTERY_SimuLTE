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
}