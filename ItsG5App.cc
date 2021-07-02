
#include "ItsG5App.h"
#include "hybrid_msgs/HybridServicesMessages_m.h"
#include "artery/traci/VehicleController.h"
#include <vanetza/btp/data_request.hpp>
#include <vanetza/dcc/profile.hpp>
#include <vanetza/geonet/interface.hpp>
#include <cmath>
#include <string.h>
#include <list>
#include <iostream>
#include <mutex>
#include <fstream>


using namespace omnetpp;
using namespace vanetza;

Define_Module(ItsG5App)

static const simsignal_t fromSubAppSignal = cComponent::registerSignal("toItsG5Signal");
static int num_fack_cam_sent = 0;
static int num_fack_cam_received = 0;

void ItsG5App::initialize()
{
    ItsG5BaseService::initialize();
    
    mVehicleController = &getFacilities().get_mutable<traci::VehicleController>();
    const std::string vehicle_id = mVehicleController->getVehicleId();
    leader_speed = 10;


    //writePRInfo(csvFile, "Sent", "Received");

    platoonId = -1;
    messageId = 0;

    if (vehicle_id.compare(0, 14, "platoon_leader") == 0)
        {
            
            role = LEADER;
            platoonId = 0;
            platoonSize = 1;
            leader_speed = 10;

            platoonMember leader;
            leader.vehicleId = vehicle_id;
            leader.idInPlatoon = 0;
            platoonMembers = {leader};
            mVehicleController->setSpeed(10 * units::si::meter_per_second);

        }

    else if (vehicle_id.compare(0, 16, "platoon_follower") == 0)
    {

        role = JOINER;
    }
    else if (vehicle_id.compare(0, 4, "free_flow") == 0)
        role = FREE;
    

    //hybrid signals 

    itsG5ToSubAppSignal = cComponent::registerSignal("itsG5ToSubAppSignal");    
    getParentModule()->subscribe(fromSubAppSignal, this);
}

void ItsG5App::trigger()
{
    Enter_Method("ItsG5App trigger");

    // btp::DataRequestB req;
    // req.destination_port = host_cast<ItsG5App::port_type>(getPortNumber());
    // req.gn.transport_type = geonet::TransportType::SHB;
    // req.gn.traffic_class.tc_id(static_cast<unsigned>(dcc::Profile::DP1));
    // req.gn.communication_profile = geonet::CommunicationProfile::ITS_G5;

    // using boost::units::si::meter;
    // using boost::units::si::meter_per_second;
    // const std::string id = mVehicleController->getVehicleId();
    // auto& vehicle_api = mVehicleController->getLiteAPI().vehicle();

    // auto packet = new PlatooningMessage();
    // packet->setVehicleId(id.c_str());
    // packet->setMessageId((id + "_" + std::to_string(messageId)).c_str());

    // //std::cout << "ID: " << id.c_str() << " speed: " << mVehicleController->getSpeed() / meter_per_second << "\n";

    // packet->setPositionX(mVehicleController->getPosition().x / meter);
    // packet->setPositionY(mVehicleController->getPosition().y / meter);
    // packet->setEdgeName(vehicle_api.getRoadID(id).c_str());
    // packet->setLaneIndex(vehicle_api.getLaneIndex(id));
    // packet->setSpeed(mVehicleController->getSpeed() / meter_per_second);
    // packet->setTime(simTime());

    
    // if (role == JOINER)
    // {
    //     // the "1" message type is for join request
    //     packet->setMessageType(1);

    //     packet->setByteLength(50);

        
    //     //std::cout << "Joiner sending message ID: " << id.c_str() << " speed: " << mVehicleController->getSpeed() / meter_per_second << "\n";

    //     //request(req, packet);
               
    //     // std::fstream file;
    //     // file.open (csvFile, std::ios::app);

    //     // if (file) {
    //     //     file << simTime() << ("_" + id + "_" + std::to_string(messageId)).c_str() << ", " << "" << "\n";
    //     // }

    //     //messageId++;
    // }
    // else if ((role == FOLLOWER) || (role == LEADER))
    // {
    //     //if ((simTime() > 70) && (role == LEADER))
    //       //  mVehicleController->setSpeed(1 * meter_per_second);

    //     // the "0" message type is platoon beacons

    //     packet->setMessageType(0);
    //     packet->setPlatoonId(0);
    //     packet->setIdInPlatoon(platoonId);
    //     packet->setPlatoonSize(platoonSize);

    //     packet->setByteLength(50);


    //     //request(req, packet);
            
    //     // std::fstream file;
    //     // file.open (csvFile, std::ios::app);

    //     // if (file) {
    //     //     file << simTime() << ("_" + id + "_" + std::to_string(messageId)).c_str() << ", " << "" << "\n";
    //     // }

    //     //messageId++;
    // }
    // else if (role == FREE)
    // {
    //     // //for non platoon vehicles
    //     // auto fakeCam = new FakeCAMMessage();
    //     // fakeCam->setVehicleId(id.c_str());
    //     // fakeCam->setMessageId((id + "_" + std::to_string(messageId)).c_str());
    //     // //std::cout << "ID: " << id.c_str() << "messageId: " << id + std::to_string(messageId) << "\n";
    //     // fakeCam->setPositionX(mVehicleController->getPosition().x / meter);
    //     // fakeCam->setPositionY(mVehicleController->getPosition().y / meter);
    //     // fakeCam->setEdgeName(vehicle_api.getRoadID(id).c_str());
    //     // fakeCam->setLaneIndex(vehicle_api.getLaneIndex(id));
    //     // fakeCam->setSpeed(mVehicleController->getSpeed() / meter_per_second);
    //     // fakeCam->setTime(simTime());
    //     // fakeCam->setByteLength(300);
    //     // num_fack_cam_sent++;
    //     // messageId++;
    //     // request(req, fakeCam);

    // }
}

void ItsG5App::indicate(const vanetza::btp::DataIndication& ind, omnetpp::cPacket* packet)
{
    Enter_Method("ItsG5App indicate");

    auto receivedMessage = check_and_cast<PlatooningMessage*>(packet);

    //std::cout << "ITS-G5 receiving from network " << receivedMessage->getMessageId() << " \n";
    receivedMessage->setInterface(0);

    emit(itsG5ToSubAppSignal, receivedMessage->dup());

    delete receivedMessage;


}

void ItsG5App::receiveSignal(cComponent*, simsignal_t sig, cObject* obj, cObject*){

    if (sig == fromSubAppSignal){

        auto sigMessage = check_and_cast<PlatooningMessage*>(obj);
        
        //std::cout << "message from fromSubAppSignal in ITSG5 received " << sigMessage->getMessageId() << " \n";
        btp::DataRequestB req;
        req.destination_port = host_cast<ItsG5App::port_type>(getPortNumber());
        req.gn.transport_type = geonet::TransportType::SHB;
        req.gn.traffic_class.tc_id(static_cast<unsigned>(dcc::Profile::DP1));
        req.gn.communication_profile = geonet::CommunicationProfile::ITS_G5;
        
        request(req, sigMessage);
            
            // std::fstream file;
            // file.open (csvFile, std::ios::app);

            // if (file) {
            //     file << simTime() << ("_" + id + "_" + std::to_string(messageId)).c_str() << ", " << "" << "\n";
            // }     
    }


}

void ItsG5App::handleMessage(omnetpp::cMessage* msg){

}

void writePRInfo(std::string fileName, std::string column1, std::string column2) {
    
  
    std::fstream file;
    file.open (fileName, std::ios::app);

    if (file) {
        file << column1 << ", " << column2 << "\n";
    }
}

double ItsG5App::squarDistance(double xPosV1, double xPosV2, double yPosV1, double yPosV2)
{
    return sqrt(pow(xPosV2 - xPosV1, 2) + pow (yPosV2 - yPosV1, 2));
}


void ItsG5App::CACCSpeedControl(std::string vehicle_id, double desired_speed, double vehicle_speed)
{
    double k_4 = 0.4;    

    double time_gap = (100/desired_speed) - (100/vehicle_speed);

    if (time_gap > 0.2)
    {
        double accel = k_4*(desired_speed - vehicle_speed);

        std::cout << "test speed control ID:" << vehicle_id.c_str() << " Difference in speed:: " << desired_speed - vehicle_speed << "\n";

        mVehicleController->setSpeed((vehicle_speed + accel) * units::si::meter_per_second);
    }
    
}


void ItsG5App::CACCGapControl(std::string vehicle_id, std::string pre_vehicle_id, double vehicle_speed, double preceding_vehicle_speed, double distance, double vehicle_accel)
{

    double k_5 = 0.45;
    double k_6 = 0.25;
    double t_desired = 0.4;
    double speed_difference = preceding_vehicle_speed - vehicle_speed;
    std::cout << "enter GAP Control" << "\n";
    double gap_deviation = distance - (t_desired * vehicle_speed);
    double speed_deviation = preceding_vehicle_speed - vehicle_speed - (t_desired * vehicle_accel);
    
    if ((gap_deviation < 0.2) && (speed_deviation < 0.1))
    {
        std::cout << "gap deviation " << gap_deviation << " speed_deviation " << speed_deviation << "\n";

        std::cout << "test gap control ID: between " << vehicle_id.c_str() << " and pree :: " << pre_vehicle_id << " Difference in distance:: " << distance << "speed Difference ::" << speed_difference << "\n";
        double speed = vehicle_speed + (k_5 * gap_deviation) + (k_6 * speed_deviation);
        std::cout << speed << "\n";
        mVehicleController->setSpeed(speed * units::si::meter_per_second);
    }


}