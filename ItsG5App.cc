#include "ItsG5App.h"
#include "hybrid_msgs/HybridServicesMessages_m.h"
#include "artery/traci/VehicleController.h"
#include <vanetza/btp/data_request.hpp>
#include <vanetza/dcc/profile.hpp>
#include <vanetza/geonet/interface.hpp>

using namespace omnetpp;
using namespace vanetza;

Define_Module(ItsG5App)


void ItsG5App::initialize()
{
    ItsG5BaseService::initialize();
    LteSignal = cComponent::registerSignal("LteSignal");
    mVehicleController = &getFacilities().get_mutable<traci::VehicleController>();
}

void ItsG5App::trigger()
{
    Enter_Method("ItsG5App trigger");
    btp::DataRequestB req;
    req.destination_port = host_cast<ItsG5App::port_type>(getPortNumber());
    req.gn.transport_type = geonet::TransportType::SHB;
    req.gn.traffic_class.tc_id(static_cast<unsigned>(dcc::Profile::DP1));
    req.gn.communication_profile = geonet::CommunicationProfile::ITS_G5;

    using boost::units::si::meter;
    using boost::units::si::meter_per_second;
    const std::string id = mVehicleController->getVehicleId();
    auto& vehicle_api = mVehicleController->getLiteAPI().vehicle();

    auto packet = new PlatooningMessage();
    packet->setVehicleId(id.c_str());
    //packet->setPositionX(mVehicleController->getPosition().x / meter);
    //packet->setPositionY(mVehicleController->getPosition().y / meter);
    packet->setEdgeName(vehicle_api.getRoadID(id).c_str());
    packet->setLaneIndex(vehicle_api.getLaneIndex(id));
    packet->setSpeed(mVehicleController->getSpeed() / meter_per_second);
    packet->setTime(simTime());
    packet->setByteLength(40);

    

    //send Signal
    EV << "sending message \n";
    emit(LteSignal, check_and_cast<PlatooningMessage*>(packet));

    request(req, packet->dup());
}

void ItsG5App::indicate(const vanetza::btp::DataIndication& ind, omnetpp::cPacket* packet)
{
    Enter_Method("ItsG5App indicate");
    auto platooningMessage = check_and_cast<const PlatooningMessage*>(packet);

    EV << "PlatooningMessage Received" << "\n";

    const std::string id = mVehicleController->getVehicleId();
    auto& vehicle_api = mVehicleController->getLiteAPI().vehicle();
    /*if (vehicle_api.getRoadID(id) == clearLaneMessage->getEdgeName()) {
        if (vehicle_api.getLaneIndex(id) != clearLaneMessage->getLaneIndex()) {
            slowDown();
        }
    }*/

    delete platooningMessage;

}