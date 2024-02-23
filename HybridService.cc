
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
#include <arpa/inet.h>
#include "torch/torch.h"
#include "artery/inet/VanetRadio.h"
#include "/home/byacheur/Apps/artery/src/artery/lte/Managment/Managment.h"


using namespace omnetpp;
using namespace vanetza;


Define_Module(HybridService)

// lte sock configuratioon
int PORT = 8080;

int sock = 0, buffer_line, H_server;
struct sockaddr_in serv_addr;

// signal to communicate with the main app
static const simsignal_t fromMainAppSignal = cComponent::registerSignal("toHybridServiceSignal");


// init for RL using libtorch
void HybridService::init(int input_dims, int num_actions, int hidden_dims){
        network = Net(input_dims, hidden_dims, num_actions);
        target_network = Net(input_dims, hidden_dims, num_actions);
        agent_memory = Memory(input_dims, mem_max); 
        observation_space = input_dims; 
        action_space = num_actions;
}
HybridService::HybridService():optimizer(network->parameters(), torch::optim::AdamOptions(0.0005)){

	this->init(environment.observation_space, environment.action_space, 128);
}



void HybridService::initialize()
{
	ItsG5Service::initialize();


	mVehicleController = &getFacilities().get_mutable<traci::VehicleController>();
    const std::string vehicle_id = mVehicleController->getVehicleId();

	// Managment layer to get the interfaces states and channel quality 
	managmentLayer = check_and_cast<artery::Managment::Managment*>(this->getParentModule()->getParentModule()->getSubmodule("managmentLayer"));

	// init platooning vars
    platoonId = -1;
    messageId = 0;

    if (vehicle_id.compare(0, 14, "platoon_leader") == 0){
		role = LEADER;
		platoonId = 0;
		platoonSize = 1;

		platoonMember leader;
		leader.vehicleId = vehicle_id;
		leader.idInPlatoon = 0;
		platoonMembers = {leader};
		
		if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
			{
				std::cout <<"\n Socket creation error" << endl;
			}

		serv_addr.sin_family = AF_INET;
		serv_addr.sin_port = htons(PORT);

		// Convert IPv4 and IPv6 addresses from text to binary form
		if(inet_pton(AF_INET, "127.0.1.1", &serv_addr.sin_addr)<=0) 
		{
			std::cout << "Invalid address/ Address not supported " << endl;
		}
		
		if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		{
			std::cout <<"Connection Failed " << endl;;
		}

    }
    else if (vehicle_id.compare(0, 16, "platoon_follower") == 0){
        role = JOINER; 
	}
    else if (vehicle_id.compare(0, 4, "free") == 0)
        role = FREE;


	// write RL REWARD and simulation parameters into the results files
	if (vehicle_id.compare(0, 4, "free") != 0){

		csvFile = "results/" + vehicle_id + ".csv";
		csvFileSNIRLTE = "results/SNIRLTE" + vehicle_id + ".csv";
		csvFileSNIRG5 = "results/SNIRG5" + vehicle_id + ".csv";
		csvFileR = "results/Reward" + vehicle_id + ".csv";

		std::fstream file;
		std::fstream fileLTE;
		std::fstream fileG5;
		std::fstream fileR;
		file.open (csvFile, std::ios::app);
		fileLTE.open (csvFileSNIRLTE, std::ios::app);
		fileG5.open (csvFileSNIRG5, std::ios::app);
		fileR.open (csvFileR, std::ios::app);
			
		if (file) {
        	file << "Sent" << ", " << "mode" << ", " << "Received" << ", " << "interface" << "\n";
        }
		file.close();

		if (fileLTE) {
			fileLTE << "SINR" << ", " << "PRR" << "\n";
		}
		fileLTE.close();

		if (fileG5) {
			fileG5 << "SINR" << ", " << "PRR" << "\n";
		}
		fileG5.close();

		if (fileR) {
			fileR << "Reward" << ", " << "N of messages" << ", " << "Hits" << "\n";
		}
		fileR.close();
	}

    // Signals 
    getParentModule()->subscribe(fromMainAppSignal, this);


    toMainAppSignal = cComponent::registerSignal("toMainAppSignal");

	
	if((role == JOINER) || (role == LEADER)){			

		// Loading nn parameters
		
		network->network_id = vehicle_id + "_Net";
		target_network->network_id = vehicle_id + "_target_Net" ;

		std::string net_model_name = environment.pt_net + vehicle_id;
		std::string target_model_name = environment.pt_target + vehicle_id;
		
	}
    
}

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
		mVehicleController = &getFacilities().get_mutable<traci::VehicleController>();
    	const std::string vehicle_id = mVehicleController->getVehicleId();

		std::cout << "V " << vehicle_id << "\n"; 
		std::cout << "Hits " << std::to_string(message_hits).c_str() << "\n"; 
		std::cout << "LIst Size " << std::to_string(receivedMessages.size()).c_str() << "\n";

		
/* saving models 
		
		std::string net_model_name = environment.pt_net + vehicle_id;
		std::string target_model_name = environment.pt_target + vehicle_id;

		torch::save(network, net_model_name);
		torch::save(target_network, target_model_name);
*/
	}
}

void HybridService::handleMessage(cMessage* msg)
{
	Enter_Method("handleMessage");

	delete msg;
}

void HybridService::receiveSignal(cComponent* source, simsignal_t signal, cObject* obj, cObject*)
{
	// we are using signals to receive a message istead of the indicate method 
	if (signal == fromMainAppSignal) {

    	auto sigMessage = check_and_cast<PlatooningMessage*>(obj);
        
        using boost::units::si::meter;
	    using boost::units::si::meter_per_second;
	    using boost::units::si::meter_per_second_squared;
	        
	    auto receivedMessage = sigMessage;
		nb_received_messsages++;

		// Hit detection

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
            message_hits++;            
        }else{

	    	if (role != FREE){
	    		std::fstream file;
		        file.open (csvFile, std::ios::app);

		        if (file) {
		            file << "" << ", " << "" << ", " << simTime() << "_" << receivedMessage->getMessageId() << ", " << std::to_string(receivedMessage->getInterface()).c_str() << "\n";
		        }
				file.close();
	    	}
			
	    	receivedMessages.push_front(receivedMessage->getMessageId());

        }

		// Environment Update

		environment.update_reception_state(platoonId, receivedMessage->getIdInPlatoon(), is_received, receivedMessage->getSending_interface());
        
		// Message reception logic 

	    auto& vehicle_api = mVehicleController->getLiteAPI().vehicle();

	    const std::string id = mVehicleController->getVehicleId();

		// processing the Join request platoning message
	    if (role == LEADER){
	            
	        if (receivedMessage->getMessageType() == 1) // if it is a join request
	        {
	            auto packet = new PlatooningMessage();
	            packet->setVehicleId(id.c_str());
	            packet->setMessageId((id + "_" + std::to_string(messageId)).c_str());
	            
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

		// processing the join response platooning message
	    if (role == JOINER){
	        
	        if (receivedMessage->getMessageType() == 2) // Type 2 is a join response 
	            if ((receivedMessage->getAccepted()) && (std::string(receivedMessage->getJoinerId()).compare(id) == 0))
	            {
	                role = FOLLOWER;
	                
	                platoonId = receivedMessage->getAffectedId();
	                platoonSize = receivedMessage->getPlatoonSize();
	                leader_speed = receivedMessage->getSpeed();

	                // begin CACC procedure to join the platoon

	                double vehicle_speed = mVehicleController->getSpeed() / meter_per_second;

	                if (receivedMessage->getIdInPlatoon() == (platoonId - 1))
	                {
	                    double xPosV1 = mVehicleController->getPosition().x / meter;
	                    double xPosV2 = receivedMessage->getPositionX();
	                    double yPosV1 = mVehicleController->getPosition().y / meter;
	                    double yPosV2 = receivedMessage->getPositionY();
	                    
	                    double vehicle_accel = vehicle_api.getAcceleration(id);
	                    
	 
	                    double distance = squarDistance(xPosV1, xPosV2, yPosV1, yPosV2);

	                    if ((distance > inPlatoonDistance) && (xPosV2 > xPosV1))
	                    {
	                        mVehicleController->setSpeed(((distance/inPlatoonDistance) * leader_speed)* meter_per_second);
	                    }
	                    else if (((distance < (inPlatoonDistance)) && (distance > (inPlatoonDistance - 1))) && (xPosV2 > xPosV1)) {

	                     
	                        mVehicleController->setSpeed(((distance/inPlatoonDistance) * leader_speed)* meter_per_second);
	                        vehicle_api.changeLane(id, receivedMessage->getLaneIndex(), 10);
	                    }

	                    else if ((distance < (inPlatoonDistance - 1)) || (xPosV2 < xPosV1))
	                    {
	                        mVehicleController->setSpeed(((distance/inPlatoonDistance) * leader_speed) * meter_per_second);
	                        if (xPosV2 < xPosV1)
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
	            
	        }

	        if (receivedMessage->getIdInPlatoon() == (platoonId - 1))
	        {
	            double xPosV1 = mVehicleController->getPosition().x / meter;
	            double xPosV2 = receivedMessage->getPositionX();
	            double yPosV1 = mVehicleController->getPosition().y / meter;
	            double yPosV2 = receivedMessage->getPositionY();
	            double vehicle_speed =  mVehicleController->getSpeed() / meter_per_second;
	            double vehicle_accel = vehicle_api.getAcceleration(id);

	            double distance = squarDistance(xPosV1, xPosV2, yPosV1, yPosV2);
				
				if((vehicle_api.getRoadID(id).compare("gneE2") == 0) || (vehicle_api.getRoadID(id).compare("gneE1") == 0)){
					xPosV2 = xPosV2 * -1;
					xPosV1 = xPosV1 * -1;
				}

	            if ((distance > inPlatoonDistance) && (xPosV2 > xPosV1))
	            {
	                mVehicleController->setSpeed(((distance/inPlatoonDistance) * leader_speed) * meter_per_second);
	            }
	            else if ((distance < inPlatoonDistance) && (distance > (inPlatoonDistance - 1)) && (xPosV2 > xPosV1)) 
				{
	                mVehicleController->setSpeed(((distance/inPlatoonDistance) * leader_speed)* meter_per_second);
	                vehicle_api.changeLane(id, receivedMessage->getLaneIndex(), 10);
	            }

	            else if ((distance < (inPlatoonDistance - 1)) || (xPosV2 < xPosV1))
	            {
	                mVehicleController->setSpeed(((distance/inPlatoonDistance) * leader_speed) * meter_per_second);
	                
	            }    

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

			if((vehicle_api.getRoadID(id).compare("gneE2") == 0) || (vehicle_api.getRoadID(id).compare("gneE1") == 0)){
				xPosV2 = xPosV2 * -1;
				xPosV1 = xPosV1 * -1;
			}
	        
	        if ((xPosV2 < xPosV1) && (distance < 30) && (strcmp(vehicle_api.getRoadID(id).c_str(), receivedMessage->getEdgeName()) == 0))
	        {	            
	            vehicle_api.changeLane(id, pow(receivedMessage->getLaneIndex() - 1, 2), 1);
	        }
	    }
	    

	    delete receivedMessage;      
    }
}


void HybridService::sendToMainApp(cMessage* msg, std::string id)
{
	auto msg_ = check_and_cast<PlatooningMessage*>(msg);
	std::fstream file;

	// To build the message to the socket
	double reward_from_env = 1;
	bool done_from_env = 0;

	// Update state of the environment and launch the learning process
	
	if(messageId > 0){
		
		environment.new_state = torch::tensor({1.0, managmentLayer->SINR_ITS_G5_first, managmentLayer->PRR_LTE_first, managmentLayer->SINR_LTE_first, 0.99, 50.0});
		int service_type = 0;
		std::tuple<double, bool> step_result = environment.step(platoonId, service_type);

		reward_from_env = std::get<0>(step_result);
		done_from_env = std::get<1>(step_result);

		avg_reward += reward_from_env;
		
		// Write into the file for stats

		if(done_from_env){
			file.open(csvFileR, std::ios::app);
			if (file) {
				file << avg_reward / environment.number_steps << ", " << environment.number_steps << ", " << environment.number_hits << "\n";
			}
			file.close();
			std::cout << "REWARD: " << avg_reward / environment.number_steps << " STEPS: " << environment.number_steps << "   ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** **\n";
			avg_reward = 0;
			environment.init();
		}
		
		// C++
		//store_transition(environment.state, environment.new_state, environment.choosen_action, std::get<0>(step_result), std::get<1>(step_result));
		//learn();

		file.open(csvFileSNIRG5, std::ios::app);
		if (file) {
			file << environment.new_state[1].item().toDouble() - environment.state[1].item().toDouble() << ", " << "" << "\n";
		}
		file.close();

		file.open(csvFileSNIRLTE, std::ios::app);
		if (file) {
			file << environment.new_state[3].item().toDouble() - environment.state[3].item().toDouble() << ", " << managmentLayer->PRR_LTE << "\n";
		}
		file.close();
	}

	
	// Proceding state and doing the action

	// Getting stat parameters
	torch::Tensor observation = torch::tensor({1.0, managmentLayer->SINR_ITS_G5, managmentLayer->PRR_LTE, managmentLayer->SINR_LTE, 0.99, 50.0});
	environment.state = observation;

	// send to ddqn agent and receive action
	std::string evaluationToSendToDDQN = "#" + std::to_string(reward_from_env) + "$" + std::to_string(done_from_env) + "$" + std::to_string(managmentLayer->SINR_ITS_G5_first) \
		+ "$" + std::to_string(managmentLayer->PRR_LTE_first) + "$" + std::to_string(managmentLayer->SINR_LTE_first);
		
	std::string stateToSendToDDQN = std::to_string(managmentLayer->SINR_ITS_G5) + "$" + std::to_string(managmentLayer->PRR_LTE) + \
		"$" + std::to_string(managmentLayer->SINR_LTE) + evaluationToSendToDDQN ;

	sendto(sock, stateToSendToDDQN.c_str(), strlen(stateToSendToDDQN.c_str()), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	char reception_buffer[4096] = {0};

	buffer_line = read(sock , reception_buffer, 1024);
	int action = std::atoi(reception_buffer);

	//int action = choose_action(observation); C++
	environment.choosen_action = action;
	msg_->setSending_interface(action);

	environment.clear_reception_state(platoonId);

    file.open (csvFile, std::ios::app);

    if (file){
    	file << simTime() << ("_" + id + "_" + std::to_string(messageId)).c_str() << ", " << action << "\n";
    }
	file.close();

	emit(toMainAppSignal, msg_);
}


// CACC functions
double HybridService::squarDistance(double xPosV1, double xPosV2, double yPosV1, double yPosV2)
{
    return sqrt(pow(xPosV2 - xPosV1, 2) + pow (yPosV2 - yPosV1, 2));
	// TODO:
	// fix distance formula and fix the platooning decomposition
	// return pow(xPosV1 - xPosV2, 2);
}


void HybridService::CACCSpeedControl(std::string vehicle_id, double desired_speed, double vehicle_speed)
{
    double k_4 = 0.4;    

    double time_gap = (100/desired_speed) - (100/vehicle_speed);

    if (time_gap > 0.2)
    {
        double accel = k_4*(desired_speed - vehicle_speed);

        //std::cout << "test speed control ID:" << vehicle_id.c_str() << " Difference in speed:: " << desired_speed - vehicle_speed << "\n";

        mVehicleController->setSpeed((vehicle_speed + accel) * boost::units::si::meter_per_second);
    }
    
}


void HybridService::CACCGapControl(std::string vehicle_id, std::string pre_vehicle_id, double vehicle_speed, double preceding_vehicle_speed, double distance, double vehicle_accel)
{

    double k_5 = 0.45;
    double k_6 = 0.25;
    double t_desired = 0.4;
    double speed_difference = preceding_vehicle_speed - vehicle_speed;
    //std::cout << "enter GAP Control" << "\n";
    double gap_deviation = distance - (t_desired * vehicle_speed);
    double speed_deviation = preceding_vehicle_speed - vehicle_speed - (t_desired * vehicle_accel);
    
    if ((gap_deviation < 0.2) && (speed_deviation < 0.1))
    {
        //std::cout << "gap deviation " << gap_deviation << " speed_deviation " << speed_deviation << "\n";

        //std::cout << "test gap control ID: between " << vehicle_id.c_str() << " and pree :: " << pre_vehicle_id << " Difference in distance:: " << distance << "speed Difference ::" << speed_difference << "\n";
        double speed = vehicle_speed + (k_5 * gap_deviation) + (k_6 * speed_deviation);
        //std::cout << speed << "\n";
        mVehicleController->setSpeed(speed * boost::units::si::meter_per_second);
    }
}

//	libtorch RL Agent Functions 

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

    const std::string vehicle_id = mVehicleController->getVehicleId();

    if(learn_step_counter % 100 == 0){

		std::string net_model_name = environment.pt_net + vehicle_id;
		std::string target_model_name = environment.pt_target + vehicle_id;

		torch::save(network, net_model_name);
		torch::save(target_network, target_model_name);

        std::cout << "Agent " << vehicle_id << " is learning and epsilon = " << epsilon <<  "***************" << "And MEM cntr = " << agent_memory.get_mem_ctr() << "\n";
	}

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

    sample_actions = sample_actions.to(torch::kInt64);

    torch::Tensor q_value = q_values.gather(1, sample_actions.unsqueeze(1)).squeeze(1);
    
    torch::Tensor maximum_next_q_values_index = std::get<1>(next_q_values.max(1));
    torch::Tensor next_q_value = target_next_q_values.gather(1, maximum_next_q_values_index.unsqueeze(1)).squeeze(1);
    torch::Tensor expected_q_value = sample_rewards + gamma*next_q_value;
    
    torch::Tensor loss = torch::mse_loss(q_value, expected_q_value);

    loss.backward();
    optimizer.step();

    learn_step_counter += 1;

    decrement_epsilon();

}
