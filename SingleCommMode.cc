#include "SingleCommMode.h"
#include "artery/traci/VehicleController.h"
#include <omnetpp/cpacket.h>
#include <vanetza/btp/data_request.hpp>
#include <vanetza/dcc/profile.hpp>
#include <vanetza/geonet/interface.hpp>

#include <boost/optional/optional_io.hpp>

using namespace omnetpp;
using namespace vanetza;


Define_Module(SingleCommMode)

SingleCommMode::SingleCommMode()
{
}


void SingleCommMode::initialize()
{
}

void SingleCommMode::finish()
{
}

void SingleCommMode::handleMessage(cMessage* msg)
{
	Enter_Method("handleMessage");
}