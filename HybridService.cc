
#include "ItsG5App.h"
#include "hybrid_msgs/HybridServicesMessages_m.h"
#include "artery/traci/VehicleController.h"
#include <vanetza/btp/data_request.hpp>
#include <vanetza/dcc/profile.hpp>
#include <vanetza/geonet/interface.hpp>
#include <cmath>
#include <list>
#include <fstream>
#include <iostream>
#include "HybridService.h"


#include "RLAgentUtils/Network.h"
#include <math.h>
#include <chrono>
#include <string>
#include <algorithm>
#include <tuple>
#include "torch/torch.h"
#include "artery/inet/VanetRadio.h"
// #include <lte/stack/phy/layer/LtePhyUeD2D.h>



using namespace omnetpp;
using namespace vanetza;


Define_Module(HybridService)

static const simsignal_t fromMainAppSignal = cComponent::registerSignal("toHybridServiceSignal");

void HybridService::init(int input_dims, int num_actions, int hidden_dims){
        network = Net(input_dims, hidden_dims, num_actions);
        target_network = Net(input_dims, hidden_dims, num_actions);
        agent_memory = Memory(input_dims, mem_max); 
        observation_space = input_dims; 
        action_space = num_actions;
}

HybridService::HybridService():optimizer(network->parameters(), torch::optim::AdamOptions(0.0001)){
	this->init(6, 512, 3);
}

void HybridService::initialize()
{
	ItsG5Service::initialize();


	mVehicleController = &getFacilities().get_mutable<traci::VehicleController>();
    const std::string vehicle_id = mVehicleController->getVehicleId();
    leader_speed = 10;  

    //writePRInfo(csvFile, "Sent", "Received");

    platoonId = -1;
    messageId = 0;



    if (vehicle_id.compare(0, 14, "platoon_leader") == 0)
        {
        	csvFile = "results/" + vehicle_id + ".csv";

        	std::fstream file;
           	file.open (csvFile, std::ios::app);
			   
            if (file) {
            	file << "Sent" << ", " << "Received" << "\n";
            	
            }

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

        csvFile = "results/" + vehicle_id + ".csv";

    	std::fstream file;
       	file.open (csvFile, std::ios::app);

        if (file) {
        	file << "Sent" << ", " << "Received" << "\n";
        	
        }
    }
    else if (vehicle_id.compare(0, 4, "free_flow") == 0)
        role = FREE;

    // Signals 
    getParentModule()->subscribe(fromMainAppSignal, this);
    toMainAppSignal = cComponent::registerSignal("toMainAppSignal");

	// Hybrid init


	network->network_id = vehicle_id + "_Net";
	target_network->network_id = vehicle_id + "_target_Net" ;

	std::cout << network << "\n the net " << network->network_id << "\n";

	std::cout << this->getParentModule() << "\n";
	std::cout << this->getParentModule()->getParentModule() << "\n";
	std::cout << this->getParentModule()->getParentModule()->getSubmodule("wlan", 0) << "\n";
	
	cModule* radio = this->getParentModule()->getParentModule()->getSubmodule("wlan", 0)->getSubmodule("radio");
	// auto radio_LTE = check_and_cast<LtePhyUeD2D*>(this->getParentModule()->getParentModule()->getSubmodule("lteNic")->getSubmodule("phy"));
	std::cout << this->getParentModule()->getParentModule()->getSubmodule("lteNic") << "\n";

    

}
//emit(LteSignal, check_and_cast<PlatooningMessage*>(packet));

void HybridService::trigger()
{
	Enter_Method("HybridService trigger");

    using boost::units::si::meter;
    using boost::units::si::meter_per_second;
    const std::string id = mVehicleController->getVehicleId();
    auto& vehicle_api = mVehicleController->getLiteAPI().vehicle();

    auto packet = new PlatooningMessage();
    packet->setVehicleId(id.c_str());
    packet->setMessageId((id + "_" + std::to_string(messageId)).c_str());

    std::cout << "ID: " << id.c_str() << " speed: " << mVehicleController->getSpeed() / meter_per_second << "\n";

    packet->setPositionX(mVehicleController->getPosition().x / meter);
    packet->setPositionY(mVehicleController->getPosition().y / meter);
    packet->setEdgeName(vehicle_api.getRoadID(id).c_str());
    packet->setLaneIndex(vehicle_api.getLaneIndex(id));
    packet->setSpeed(mVehicleController->getSpeed() / meter_per_second);
    packet->setTime(simTime());

    
    if (role == JOINER)
    {
        // the "1" message type is for join request
        packet->setMessageType(1);

        packet->setByteLength(50);

        std::cout << "Joiner sending message ID: " << id.c_str() << " speed: " << mVehicleController->getSpeed() / meter_per_second << "\n";

        sendToMainApp(packet, id);

        messageId++;
    }
    else if ((role == FOLLOWER) || (role == LEADER))
    {
        // the "0" message type is platoon beacons

        packet->setMessageType(0);
        packet->setPlatoonId(0);
        packet->setIdInPlatoon(platoonId);
        packet->setPlatoonSize(platoonSize);

        packet->setByteLength(50);

        sendToMainApp(packet, id);
            
        messageId++;
    }
    else if (role == FREE)
    {
         
    }
}


void HybridService::indicate(const btp::DataIndication& ind, cPacket* packet)
{

}



void HybridService::finish()
{
	ItsG5Service::finish();
	if (role != FREE){
		
		std::cout << "Hits " << std::to_string(message_hits).c_str() << "\n"; 
		std::cout << "LIst Size " << std::to_string(receivedMessages.size()).c_str() << "\n";
	}
}

void HybridService::handleMessage(cMessage* msg)
{
	Enter_Method("handleMessage");

	delete msg;
}

void HybridService::receiveSignal(cComponent* source, simsignal_t signal, cObject* obj, cObject*)
{

	if (signal == fromMainAppSignal) {

    	auto sigMessage = check_and_cast<PlatooningMessage*>(obj);
        
        std::cout << "Message from fromMainAppSignal received " << sigMessage->getMessageId() << " \n";

        using boost::units::si::meter;
	    using boost::units::si::meter_per_second;
	    using boost::units::si::meter_per_second_squared;

	        
	    auto receivedMessage = sigMessage;

	    // TODO: Add writing in file

	    std::list<std::string>::iterator it;
	    bool is_received = false;

        for(it = receivedMessages.begin(); it != receivedMessages.end(); ++it)
        {
            std::string tmp(it->c_str());
            if (tmp.compare(receivedMessage->getMessageId()) == 0)
            {
                receivedMessages.remove(tmp);
                is_received = true;
                break;
            }
        }

        if (is_received)
        {
            std::cout << "Hit ************** \n";
            message_hits++;

            
        }else{

	    	if (role != FREE){
	    		std::fstream file;
		        file.open (csvFile, std::ios::app);

		        if (file) {
		            file << "" << ", " << simTime() << "_" << receivedMessage->getMessageId() << ", " << std::to_string(receivedMessage->getInterface()).c_str() << "\n";
		        }
	    	}
	    	receivedMessages.push_front(receivedMessage->getMessageId());

        }

        

	    

	    auto& vehicle_api = mVehicleController->getLiteAPI().vehicle();
	    const std::string id = mVehicleController->getVehicleId();

	    if (role == LEADER){
	            
	        if (receivedMessage->getMessageType() == 1) // if it is a join request
	        {
	            auto packet = new PlatooningMessage();
	            packet->setVehicleId(id.c_str());
	            packet->setMessageId((id + "_" + std::to_string(messageId)).c_str());
	            
	            //std::cout << "JOIN RESPONSE -> ID: " << id.c_str() << "messageId: " << id + std::to_string(messageId) << "\n";
	            packet->setPositionX(mVehicleController->getPosition().x / meter);
	            packet->setPositionY(mVehicleController->getPosition().y / meter);
	            packet->setEdgeName(vehicle_api.getRoadID(id).c_str());
	            packet->setLaneIndex(vehicle_api.getLaneIndex(id));
	            packet->setSpeed(mVehicleController->getSpeed() / meter_per_second);
	            packet->setTime(simTime());

	            packet->setMessageType(2); // Type 2 is a join response 
	            packet->setAccepted(true);
	            packet->setJoinerId(receivedMessage->getVehicleId());
	            packet->setPlatoonId(0);
	            packet->setIdInPlatoon(platoonId);
	            packet->setByteLength(50);

	            bool is_in_platoon = false;
	            int found_id;

	            std::list<platoonMember>::iterator it;
	            for(it = platoonMembers.begin(); it != platoonMembers.end(); ++it)
	            {
	                
	                if (it->vehicleId.compare(receivedMessage->getVehicleId()) == 0)
	                {
	                    found_id = it->idInPlatoon;
	                    is_in_platoon = true;
	                    break;
	                }
	                
	            }

	            if (is_in_platoon)
	            {
	                packet->setPlatoonSize(platoonSize);
	                packet->setAffectedId(found_id);
	                
	            }else
	            {
	                platoonSize++;
	                std::cout << "platoon size: " << platoonSize << ", adding: " << receivedMessage->getVehicleId() <<"\n";
	                
	                platoonMember follower;
	                follower.vehicleId = receivedMessage->getVehicleId();
	                follower.idInPlatoon = platoonSize - 1;
	                platoonMembers.push_front(follower);
	                
	                packet->setPlatoonSize(platoonSize);
	                packet->setAffectedId(platoonSize - 1);
	                
	            }

	            sendToMainApp(packet, id);
	            
	            messageId++;
	        } else 
	        {
	            // Platooning message received type 0
	        }
	    }
	  

	    if (role == JOINER){

	        
	        if (receivedMessage->getMessageType() == 2) // Type 2 is a join response 
	            if ((receivedMessage->getAccepted()) && (std::string(receivedMessage->getJoinerId()).compare(id) == 0))
	            {
	                role = FOLLOWER;
	                
	                platoonId = receivedMessage->getAffectedId();
	                platoonSize = receivedMessage->getPlatoonSize();
	                leader_speed = receivedMessage->getSpeed();

	                std::cout << "Join Accepted -> ID: " << id.c_str() << " id sender " << receivedMessage->getIdInPlatoon() << " id vehicle: " << platoonId << " messageId: " << id + std::to_string(messageId) << " speed: " <<  mVehicleController->getSpeed() / meter_per_second <<  "\n";

	                // begin CACC procedure to join the platoon
	                // TODO: verify time gap

	                double vehicle_speed = mVehicleController->getSpeed() / meter_per_second;

	                if (receivedMessage->getIdInPlatoon() == (platoonId - 1))
	                {
	                    double xPosV1 = mVehicleController->getPosition().x / meter;
	                    double xPosV2 = receivedMessage->getPositionX();
	                    double yPosV1 = mVehicleController->getPosition().y / meter;
	                    double yPosV2 = receivedMessage->getPositionY();
	                    
	                    double vehicle_accel = vehicle_api.getAcceleration(id);
	                    vehicle_api.changeLane(id, 1, 2);
	                    
	 
	                    double distance = squarDistance(xPosV1, xPosV2, yPosV1, yPosV2);
	                    std::cout << "id : " << id.c_str() << " id pree : " << receivedMessage->getVehicleId() << " distance : " << distance << "\n";

	                    if ((distance > 21) && (xPosV2 > xPosV1))
	                    {
	                        std::cout << "> 20\n";
	                        
	                        mVehicleController->setSpeed(( (distance/20) * leader_speed)* meter_per_second);
	                    }
	                    else if (((distance < 21) && (distance > 19)) && (xPosV2 > xPosV1)) {

	                        std::cout << "< 15.5 \n";
	                        mVehicleController->setSpeed(( (distance/20) * leader_speed)* meter_per_second);
	                        vehicle_api.changeLane(id, receivedMessage->getLaneIndex(), 2);
	                    }

	                    else if ((distance < 15) || (xPosV2 < xPosV1))
	                    {
	                        std::cout << "< 10\n";
	                        mVehicleController->setSpeed((0.5 * leader_speed) * meter_per_second);
	                        vehicle_api.changeLane(id, pow(receivedMessage->getLaneIndex() - 1, 2), 2);
	                        
	                    }                    
	                }

	            }
	    }

	    if (role == FOLLOWER)
	    {
	        
	        if (receivedMessage->getIdInPlatoon() == 0)
	        {  
	            
	            platoonSize = receivedMessage->getPlatoonSize();
	            double vehicle_speed =  mVehicleController->getSpeed() / meter_per_second;
	            leader_speed = receivedMessage->getSpeed();
	            
	            if (is_in_platoon)
	                CACCSpeedControl(id, leader_speed, vehicle_speed);
	        }

	        if (receivedMessage->getIdInPlatoon() == (platoonId - 1))
	        {
	            //std::cout << "test gap control" << "\n";
	            double xPosV1 = mVehicleController->getPosition().x / meter;
	            double xPosV2 = receivedMessage->getPositionX();
	            double yPosV1 = mVehicleController->getPosition().y / meter;
	            double yPosV2 = receivedMessage->getPositionY();
	            double vehicle_speed =  mVehicleController->getSpeed() / meter_per_second;
	            double vehicle_accel = vehicle_api.getAcceleration(id);

	            double distance = squarDistance(xPosV1, xPosV2, yPosV1, yPosV2);

	            std::cout << "id : " << id.c_str() << " id pree : " << receivedMessage->getVehicleId() << " distance : " << distance << "\n";

	            if ((distance > 21) && (xPosV2 > xPosV1))
	            {
	                std::cout << "> 20\n";
	                
	                mVehicleController->setSpeed(( (distance/20) * leader_speed)* meter_per_second);
	            }
	            else if (((distance < 21) && (distance > 19)) && (xPosV2 > xPosV1)) {

	                std::cout << "< 15.5 \n";
	                mVehicleController->setSpeed(( (distance/20) * leader_speed)* meter_per_second);
	                vehicle_api.changeLane(id, receivedMessage->getLaneIndex(), 2);
	            }

	            else if ((distance < 15) || (xPosV2 < xPosV1))
	            {
	                std::cout << "< 10\n";
	                mVehicleController->setSpeed((0.5 * leader_speed) * meter_per_second);
	                vehicle_api.changeLane(id, pow(receivedMessage->getLaneIndex() - 1, 2), 2);
	                
	            }    

	            //CACCGapControl(id, receivedMessage->getVehicleId(), vehicle_speed, receivedMessage->getSpeed(), distance, vehicle_accel);
	        }
	    }

	    if (role == FREE){

	        double xPosV1 = mVehicleController->getPosition().x / meter;
	        double xPosV2 = receivedMessage->getPositionX();
	        double yPosV1 = mVehicleController->getPosition().y / meter;
	        double yPosV2 = receivedMessage->getPositionY();
	        double vehicle_speed =  mVehicleController->getSpeed() / meter_per_second;
	        double vehicle_accel = vehicle_api.getAcceleration(id);

	        double distance = squarDistance(xPosV1, xPosV2, yPosV1, yPosV2);
	        
	        if ((receivedMessage->getPositionX() < mVehicleController->getPosition().x / meter) && (distance < 15) && (strcmp(vehicle_api.getRoadID(id).c_str(), receivedMessage->getEdgeName()) == 0))
	        {
	            //mVehicleController->setSpeed(0.5 * receivedMessage->getSpeed() * meter_per_second);
	            
	            vehicle_api.changeLane(id, pow(receivedMessage->getLaneIndex() - 1, 2), 2);
	        }
	        //mVehicleController->setSpeed(10 * meter_per_second);

	    }
	    

	    delete receivedMessage;      
    }
}

void HybridService::sendToMainApp(cMessage* msg, std::string id)
{
	//std::cout << "sending from HybridService to main App the following message: " << messageId << "\n";

	std::fstream file;
    file.open (csvFile, std::ios::app);

    if (file) {
    	file << simTime() << ("_" + id + "_" + std::to_string(messageId)).c_str() << ", " << "" << "\n";
    }

	auto msg_ = check_and_cast<PlatooningMessage*>(msg);

	// RL algo

	bool done = false;

	// Getting stat parameters
	cModule* radio = this->getParentModule()->getParentModule()->getSubmodule("wlan", 0)->getSubmodule("radio");
	double SNIR_ITS_G5 = (check_and_cast<artery::VanetRadio*>(radio))->minSNIR_ITS_G5; 
	std::cout << "SNIR = " << SNIR_ITS_G5 << "\n";
    torch::Tensor observation = torch::tensor({0.8, SNIR_ITS_G5, 0.9, 25.0, 0.99, 50.0});
	int action = choose_action(observation);
	
	msg_->setSending_interface(action);

	emit(toMainAppSignal, msg_);
}


double HybridService::squarDistance(double xPosV1, double xPosV2, double yPosV1, double yPosV2)
{
    return sqrt(pow(xPosV2 - xPosV1, 2) + pow (yPosV2 - yPosV1, 2));
}


void HybridService::CACCSpeedControl(std::string vehicle_id, double desired_speed, double vehicle_speed)
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


void HybridService::CACCGapControl(std::string vehicle_id, std::string pre_vehicle_id, double vehicle_speed, double preceding_vehicle_speed, double distance, double vehicle_accel)
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

//	RL Agent Functions 

int HybridService::choose_action(torch::Tensor state){
    
    if (torch::rand(1).item().toDouble() > epsilon){
        torch::Tensor actions = network->forward(state);
        int action = actions.argmax().item().toInt();
        return action;
    }else{
        return torch::rand(3).argmax().item().toInt();
    }

}

void HybridService::store_transition(torch::Tensor state, torch::Tensor new_state, int action, double reward, bool done){
    agent_memory.store_transition(state, new_state, action, reward, done);
}

void HybridService::replace_target_network(){
    if((learn_step_counter % replace_target_cnt) == 0){
        torch::save(network, "model.pt");
        torch::load(target_network, "model.pt");
    } 
}

void HybridService::decrement_epsilon(){
    if (epsilon > epsilon_min)
        epsilon -= epsilon_decay;
    else 
        epsilon = epsilon_min;
}


void HybridService::learn(){

    if (agent_memory.get_mem_ctr() < batch_size)
        return ;
    
    if(learn_step_counter % 1000 == 0)
        std::cout << "Agent is learning and epsilon = " << epsilon <<  "*************************************************\n";

    optimizer.zero_grad();

    replace_target_network();

    auto sample_tuple = agent_memory.sample_memory(batch_size, observation_space);

    torch::Tensor sample_states = std::get<0>(sample_tuple);
    torch::Tensor sample_new_states = std::get<1>(sample_tuple);
    torch::Tensor sample_actions = std::get<2>(sample_tuple);
    torch::Tensor sample_rewards = std::get<3>(sample_tuple);
    torch::Tensor sample_terminals = std::get<4>(sample_tuple);

    // network action predection

    torch::Tensor q_values = network->forward(sample_states);
    torch::Tensor next_q_values = network->forward(sample_new_states);
    torch::Tensor target_next_q_values = target_network->forward(sample_new_states);

    sample_actions =sample_actions.to(torch::kInt64);

    torch::Tensor q_value = q_values.gather(1, sample_actions.unsqueeze(1)).squeeze(1);
    
    torch::Tensor maximum_next_q_values_index = std::get<1>(next_q_values.max(1));
    torch::Tensor next_q_value = target_next_q_values.gather(1, maximum_next_q_values_index.unsqueeze(1)).squeeze(1);
    torch::Tensor expected_q_value = sample_rewards + gamma*next_q_value*(1-sample_terminals);
    
    torch::Tensor loss = torch::mse_loss(q_value, expected_q_value);

    loss.backward();
    optimizer.step();

    learn_step_counter += 1;

    decrement_epsilon();

}