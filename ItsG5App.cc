
#include "ItsG5App.h"
#include "hybrid_msgs/HybridServicesMessages_m.h"
#include "artery/traci/VehicleController.h"
#include <vanetza/btp/data_request.hpp>
#include <vanetza/dcc/profile.hpp>
#include <vanetza/geonet/interface.hpp>
//#include <traci/sumo/utils/traci/TraCIAPI.h>
#include<cmath>


using namespace omnetpp;
using namespace vanetza;

Define_Module(ItsG5App)


void ItsG5App::initialize()
{
    ItsG5BaseService::initialize();
    LteSignal = cComponent::registerSignal("LteSignal");
    mVehicleController = &getFacilities().get_mutable<traci::VehicleController>();
    const std::string vehicle_id = mVehicleController->getVehicleId();

    platoonId = -1;
    messageId = 0;

    if (vehicle_id.compare(0, 14, "platoon_leader") == 0)
        {
            role = LEADER;
            platoonId = 0;
            platoonSize = 1;
        }
    else if (vehicle_id.compare(0, 16, "platoon_follower") == 0)
        role = JOINER;
    else if (vehicle_id.compare(0, 4, "free_flow") == 0)
        role = FREE;
    
    
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
    packet->setMessageId((id + "_" +std::to_string(messageId)).c_str());
    //std::cout << "ID: " << id.c_str() << "messageId: " << id + std::to_string(messageId) << "\n";
    packet->setPositionX(mVehicleController->getPosition().x / meter);
    packet->setPositionY(mVehicleController->getPosition().y / meter);
    packet->setEdgeName(vehicle_api.getRoadID(id).c_str());
    packet->setLaneIndex(vehicle_api.getLaneIndex(id));
    packet->setSpeed(mVehicleController->getSpeed() / meter_per_second);
    packet->setTime(simTime());

    if (simTime() > 50)
        if (role == JOINER)
        {
            // the "1" message type is for join request
            packet->setMessageType(1);

            packet->setByteLength(50);

            //send Signal
            //std::cout << "Joiner sending message ID: " << id.c_str() << "\n";
            EV << "sending message \n";
            emit(LteSignal, check_and_cast<PlatooningMessage*>(packet));

            request(req, packet);
            messageId++;
        }
        else if ((role == FOLLOWER) || (role == LEADER))
        {
            // the "0" message type is for platoon CAMs
            packet->setMessageType(0);
            packet->setPlatoonId(0);
            packet->setIdInPlatoon(platoonId);
            packet->setPlatoonSize(platoonSize);

            packet->setByteLength(50);

            //send Signal
            //std::cout << "Leader sending message ID: " << id.c_str() << "\n";
            EV << "sending message \n";
            emit(LteSignal, check_and_cast<PlatooningMessage*>(packet));

            request(req, packet);
            messageId++;
        }
        else if (role == FREE)
        {
            //for non platoon vehicles
            auto fakeCam = new FakeCAMMessage();
            fakeCam->setVehicleId(id.c_str());
            fakeCam->setMessageId((id + "_" +std::to_string(messageId)).c_str());
            //std::cout << "ID: " << id.c_str() << "messageId: " << id + std::to_string(messageId) << "\n";
            fakeCam->setPositionX(mVehicleController->getPosition().x / meter);
            fakeCam->setPositionY(mVehicleController->getPosition().y / meter);
            fakeCam->setEdgeName(vehicle_api.getRoadID(id).c_str());
            fakeCam->setLaneIndex(vehicle_api.getLaneIndex(id));
            fakeCam->setSpeed(mVehicleController->getSpeed() / meter_per_second);
            fakeCam->setTime(simTime());
            fakeCam->setByteLength(300);
            request(req, fakeCam);
            messageId++;

        }
}

void ItsG5App::indicate(const vanetza::btp::DataIndication& ind, omnetpp::cPacket* packet)
{
    Enter_Method("ItsG5App indicate");

    using boost::units::si::meter;
    using boost::units::si::meter_per_second;
    using boost::units::si::meter_per_second_squared;
    auto platooningMessage = check_and_cast<const PlatooningMessage*>(packet);
    const std::string id = mVehicleController->getVehicleId();
    auto& vehicle_api = mVehicleController->getLiteAPI().vehicle();

    if (role == LEADER) 
        if (platooningMessage->getMessageType() == 1) // if it is a join request
        {
            btp::DataRequestB req;
            req.destination_port = host_cast<ItsG5App::port_type>(getPortNumber());
            req.gn.transport_type = geonet::TransportType::SHB;
            req.gn.traffic_class.tc_id(static_cast<unsigned>(dcc::Profile::DP1));
            req.gn.communication_profile = geonet::CommunicationProfile::ITS_G5;

            auto packet = new PlatooningMessage();
            packet->setVehicleId(id.c_str());
            packet->setMessageId((id + "_" +std::to_string(messageId)).c_str());
            //std::cout << "JOIN RESPONSE -> ID: " << id.c_str() << "messageId: " << id + std::to_string(messageId) << "\n";
            packet->setPositionX(mVehicleController->getPosition().x / meter);
            packet->setPositionY(mVehicleController->getPosition().y / meter);
            packet->setEdgeName(vehicle_api.getRoadID(id).c_str());
            packet->setLaneIndex(vehicle_api.getLaneIndex(id));
            packet->setSpeed(mVehicleController->getSpeed() / meter_per_second);
            packet->setTime(simTime());

            packet->setMessageType(2);
            packet->setAccepted(true);
            platoonSize++;
            packet->setPlatoonSize(platoonSize);
            packet->setPlatoonId(0);
            packet->setIdInPlatoon(platoonId);

            packet->setByteLength(50);

            //send Signal
            EV << "sending message \n";
            emit(LteSignal, check_and_cast<PlatooningMessage*>(packet));

            request(req, packet);
            messageId++;

        } else 
        {
            // Platooning message received type 0
        }

    if (role == JOINER)
        if (platooningMessage->getMessageType() == 2)
            if (platooningMessage->getAccepted())
            {
                role = FOLLOWER;
                //std::cout << "Join Accepted -> ID: " << id.c_str() << "messageId: " << id + std::to_string(messageId) << "\n";
                platoonId = platooningMessage->getPlatoonSize();
                platoonSize = platooningMessage->getPlatoonSize() + 1;

                // begin CACC procedure to join the platoon
                // TODO: verify time gap

                double vehicle_speed =  mVehicleController->getSpeed() / meter_per_second;

                CACCSpeedControl(id, platooningMessage->getSpeed(), vehicle_speed);

                if (platooningMessage->getIdInPlatoon() == platoonId - 1)
                {
                    double xPosV1 = mVehicleController->getPosition().x / meter;
                    double xPosV2 = platooningMessage->getPositionX();
                    double yPosV1 = mVehicleController->getPosition().y / meter;
                    double yPosV2 = platooningMessage->getPositionY();
                    
                    double vehicle_accel = vehicle_api.getAccel(id);
                    std::cout << "vehicle_accel: " << vehicle_accel << " vehicle_id: " << id.c_str() << "\n";
 
                    double distance = squarDistance(xPosV1, xPosV2, yPosV1, yPosV2);

                    CACCGapControl(vehicle_speed, platooningMessage->getSpeed(), distance, vehicle_accel);
                }


            }

    if (role == FOLLOWER)
        if (platooningMessage->getIdInPlatoon() == 0)
        {  
            
            platoonSize = platooningMessage->getPlatoonSize();
            double vehicle_speed =  mVehicleController->getSpeed() / meter_per_second;
            CACCSpeedControl(id, platooningMessage->getSpeed(), vehicle_speed);
        }
        if (platooningMessage->getIdInPlatoon() == (platoonId - 1))
        {
            std::cout << "test gap control" << "\n";
            double xPosV1 = mVehicleController->getPosition().x / meter;
            double xPosV2 = platooningMessage->getPositionX();
            double yPosV1 = mVehicleController->getPosition().y / meter;
            double yPosV2 = platooningMessage->getPositionY();
            double vehicle_speed =  mVehicleController->getSpeed() / meter_per_second;
            double vehicle_accel = vehicle_api.getAccel(id);

            double distance = squarDistance(xPosV1, xPosV2, yPosV1, yPosV2);

            CACCGapControl(vehicle_speed, platooningMessage->getSpeed(), distance, vehicle_accel);
        }
    

    EV << "PlatooningMessage Received " << "\n";

    delete platooningMessage;

}


double ItsG5App::squarDistance(double xPosV1, double xPosV2, double yPosV1, double yPosV2)
{
    return sqrt(pow(xPosV2 - xPosV1, 2) + pow (yPosV2 - yPosV1, 2));
}


void ItsG5App::CACCSpeedControl(std::string vehicle_id, double desired_speed, double vehicle_speed)
{
    double k_4 = 0.4;
    auto& vehicle_api = mVehicleController->getLiteAPI().vehicletype();
    

    double accel = k_4*(desired_speed - vehicle_speed);
    std::cout << "test speed control ID:" << vehicle_id.c_str() << " Accel:: " << vehicle_speed + accel << "\n";
    mVehicleController->setSpeed((vehicle_speed + accel) * units::si::meter_per_second);

}


void ItsG5App::CACCGapControl(double vehicle_speed, double preceding_vehicle_speed, double distance, double vehicle_accel)
{

    double k_5 = 0.45;
    double k_6 = 0.25;
    double t_desired = 1;
    
    double gap_deviation = distance - (t_desired * vehicle_speed);
    double speed_deviation = preceding_vehicle_speed - vehicle_speed - (t_desired * vehicle_accel);
    
    
        double speed = vehicle_speed + (k_5 * gap_deviation) + (k_6 * speed_deviation);
        mVehicleController->setSpeed(speed * units::si::meter_per_second);   
    
    

}