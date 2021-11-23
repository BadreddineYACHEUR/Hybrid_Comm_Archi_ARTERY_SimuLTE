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
static const simsignal_t fromLteSignal = cComponent::registerSignal("lteToSubAppSignal");

SubApplication::SubApplication()
{
}


void SubApplication::initialize()
{
	interfaceType = par("interface_type").stringValue();

	if (interfaceType.compare("ItsG5") == 0){

		toItsG5Signal = cComponent::registerSignal("toItsG5Signal");
		getParentModule()->getParentModule()->getParentModule()->subscribe(fromItsG5Signal, this);


	}else if (interfaceType.compare("LTEMode3") == 0){

		toLteSignal = cComponent::registerSignal("toLteSignal");
		getParentModule()->getParentModule()->getParentModule()->subscribe(fromLteSignal, this);
	}


	subApplicationIn = findGate("subApplicationIn");
	subApplicationOut = findGate("subApplicationOut");
	
}

void SubApplication::finish()
{
}

void SubApplication::receiveSignal(cComponent*, simsignal_t sig, cObject* obj, cObject*){

	auto sigMessage = check_and_cast<cMessage*>(obj);

	if (sig == fromItsG5Signal) {
        
        //std::cout << "message from fromItsG5Signal received " << sigMessage << " \n";

        sendToMainApp(sigMessage);
        
    }else if (sig == fromLteSignal){
        
        //std::cout << "message from fromLteSignal received " << sigMessage << " \n";

        sendToMainApp(sigMessage);

    }

    delete sigMessage;
}

void SubApplication::handleMessage(omnetpp::cMessage* msg){

    if (msg->getArrivalGateId() == subApplicationIn){

        //std::cout << "Message from Main App --> " << msg << " Im interface:" << interfaceType << "\n";

        if (interfaceType.compare("ItsG5") == 0){
        	//std::cout << " Sending to ItsG5 \n";

        	emit(toItsG5Signal, msg->dup());

        }else if (interfaceType.compare("LTEMode3") == 0){

        	//std::cout << " Sending to LTEMode3 \n";
        	emit(toLteSignal, msg->dup());
        }
        delete msg;
    }
        

}

void SubApplication::sendToMainApp (omnetpp::cMessage* msg){

	Enter_Method("HybridApp sendToMainApp");

	//std::cout << "sending to Main App from sub app : " << interfaceType << " \n" ;

	send(msg->dup(), "subApplicationOut");

}
