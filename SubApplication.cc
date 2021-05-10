#include "SubApplication.h"
#include "artery/traci/VehicleController.h"
#include <omnetpp/cpacket.h>
#include <vanetza/btp/data_request.hpp>
#include <vanetza/dcc/profile.hpp>
#include <vanetza/geonet/interface.hpp>

#include <boost/optional/optional_io.hpp>

using namespace omnetpp;
using namespace vanetza;


Define_Module(SubApplication)

static const simsignal_t fromItsG5Signal = cComponent::registerSignal("itsG5ToSubAppSignal");
static const simsignal_t fromLteSignal = cComponent::registerSignal("lteG5ToSubAppSignal");

SubApplication::SubApplication()
{
}


void SubApplication::initialize()
{
	toLteSignal = cComponent::registerSignal("toLteSignal");
	toItsG5Signal = cComponent::registerSignal("toItsG5Signal");

	getParentModule()->subscribe(fromItsG5Signal, this);
	getParentModule()->subscribe(fromLteSignal, this);

	subApplicationIn = findGate("subApplicationIn");
	subApplicationOut = findGate("subApplicationOut");
	
}

void SubApplication::finish()
{
}

void SubApplication::receiveSignal(cComponent*, simsignal_t sig, cObject* obj, cObject*)
{}
