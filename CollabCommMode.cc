#include "CollabCommMode.h"
#include "artery/traci/VehicleController.h"
#include <omnetpp/cpacket.h>
#include <vanetza/btp/data_request.hpp>
#include <vanetza/dcc/profile.hpp>
#include <vanetza/geonet/interface.hpp>

#include <boost/optional/optional_io.hpp>

using namespace omnetpp;
using namespace vanetza;


Define_Module(ColllabCommMode)

ColllabCommMode::ColllabCommMode()
{
}


void ColllabCommMode::initialize()
{
}

void ColllabCommMode::finish()
{
}

void ColllabCommMode::handleMessage(cMessage* msg)
{
	Enter_Method("handleMessage");
}