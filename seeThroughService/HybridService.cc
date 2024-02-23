
#include "../ItsG5App.h"
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
#include "../RLAgentUtils/Network.h"
#include <math.h>
#include <chrono>
#include <string>
#include <algorithm>
#include <tuple>
#include <arpa/inet.h>
#include "torch/torch.h"
#include "artery/inet/VanetRadio.h"
// #include "stack/phy/ChannelModel/LteRealisticChannelModel.h"
#include "/home/byacheur/Apps/artery/src/artery/lte/Managment/Managment.h"


using namespace omnetpp;
using namespace vanetza;


Define_Module(HybridService)

int PORT = 8080;

int sock = 0, buffer_line, H_server;
struct sockaddr_in serv_addr;
static int flow_id = 0;

static bool connection_done = false;

static const simsignal_t fromMainAppSignal = cComponent::registerSignal("toHybridServiceSignal");

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
	managmentLayer = check_and_cast<artery::Managment::Managment*>(this->getParentModule()->getParentModule()->getSubmodule("managmentLayer"));

    messageId = 0;

	csvFile = "results/messages" + vehicle_id + ".csv";
	csvFileR = "results/Reward" + vehicle_id + ".csv";
	

	if (!connection_done){

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
		
		// printf("\nConnection to server !!!! \n");

		if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		{
			std::cout <<"Connection Failed " << endl;;
		}

		// Preparing statistic files	

		std::fstream file;
		std::fstream fileR;
		file.open (csvFile, std::ios::app);
		fileR.open (csvFileR, std::ios::app);
			
		if (file) {
			file << "Sent" << ", " << "mode" << ", " << "Received" << ", " << "interface" << "\n";
		}
		file.close();

		if (fileR) {
			fileR << "Reward" << ", " << "N of messages" << ", " << "Hits" << "\n";
		}
		fileR.close();
		connection_done = true;
		 
	}
	

    // Signals for Hybrid control
    getParentModule()->subscribe(fromMainAppSignal, this);

    toMainAppSignal = cComponent::registerSignal("toMainAppSignal");


	// Hybrid init
	
	vehicle_id_in_flow = flow_id;

	flow_id++;	
}

void HybridService::trigger()
{
	Enter_Method("HybridService trigger");

    using boost::units::si::meter;
    using boost::units::si::meter_per_second;
    const std::string id = mVehicleController->getVehicleId();
    auto& vehicle_api = mVehicleController->getLiteAPI().vehicle();


	auto packet = new SeeThroughMessage();
	packet->setVehicleIdInFlow(vehicle_id_in_flow);
	packet->setMessageId((id + "_" + std::to_string(messageId)).c_str());

	//std::cout << "ID: " << id.c_str() << " speed: " << mVehicleController->getSpeed() / meter_per_second << "\n";

	packet->setPositionX(mVehicleController->getPosition().x / meter);
	packet->setPositionY(mVehicleController->getPosition().y / meter);
	packet->setEdgeName(vehicle_api.getRoadID(id).c_str());
	packet->setLaneIndex(vehicle_api.getLaneIndex(id));
	packet->setSpeed(mVehicleController->getSpeed() / meter_per_second);
	packet->setEmissionTime(simTime());

	packet->setByteLength(100);
	packet->setMessageSize(100);
	packet->setPartSize(50);
	packet->setPartId(0);

	//std::cout << "Joiner sending message ID: " << id.c_str() << " speed: " << mVehicleController->getSpeed() / meter_per_second << "\n";

	sendToMainApp(packet, id);
	messageId++;
	

	

}

void HybridService::indicate(const btp::DataIndication& ind, cPacket* packet)
{
	
}


void HybridService::sendToMainApp(cMessage* msg, std::string id)
{
	auto msg_ = check_and_cast<SeeThroughMessage*>(msg);
	std::fstream file;

	// To build the message to the socket
	double reward_from_env = 1;
	bool done_from_env = 0;

	// Update state of the environment and launch the learning process
	if(messageId > 0){
		
		environment.new_state = torch::tensor({1.0, managmentLayer->SINR_ITS_G5_first, managmentLayer->PRR_LTE_first, managmentLayer->SINR_LTE_first, 0.99, 50.0});
		int service_type  = 1;
		std::tuple<double, bool> step_result = environment.step(vehicle_id_in_flow, service_type);

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
			std::cout << "REWARD: " << avg_reward / environment.number_steps << " STEPS: " << environment.number_steps << "   * ** ** ** ** ** ** ** ** **\n";
			avg_reward = 0;
			environment.init();
		}
		
		// C++
		//store_transition(environment.state, environment.new_state, environment.choosen_action, std::get<0>(step_result), std::get<1>(step_result));
		//learn();
	}
	
	// Proceding state and selecting the action

	// Getting stat parameters
	torch::Tensor observation = torch::tensor({1.0, managmentLayer->SINR_ITS_G5, managmentLayer->PRR_LTE, managmentLayer->SINR_LTE, 0.90, 50.0});
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
	
	//C++
	//int action = choose_action(observation); 
	environment.choosen_action = action;
	msg_->setSendingInterface(action);

	environment.clear_reception_state(vehicle_id_in_flow);

    file.open (csvFile, std::ios::app);

    if (file){
    	file << simTime() << ("_" + id + "_" + std::to_string(messageId)).c_str() << ", " << action << "\n";
    }
	file.close();

	emit(toMainAppSignal, msg_);

	managmentLayer->first_message_ITS_G5 = true;
	managmentLayer->first_message_LTE = true;
	
}


void HybridService::handleMessage(cMessage* msg)
{
	Enter_Method("handleMessage");

	delete msg;
}

void HybridService::receiveSignal(cComponent* source, simsignal_t signal, cObject* obj, cObject*)
{

	if (signal == fromMainAppSignal){

    	auto sigMessage = check_and_cast<SeeThroughMessage*>(obj);
        
        // std::cout << "Message from fromMainAppSignal received " << sigMessage->getMessageId() << " \n";

        using boost::units::si::meter;
	    using boost::units::si::meter_per_second;
	    using boost::units::si::meter_per_second_squared;
	        
	    auto receivedMessage = sigMessage;

		nb_received_messsages++;

		if((receivedMessage->getSendingInterface() == 0) || (receivedMessage->getSendingInterface() == 1)){

			std::fstream file;
			file.open(csvFile, std::ios::app);

			if (file) {
				file << "" << ", " << "" << ", " << simTime() << "_" << receivedMessage->getMessageId() << ", " << std::to_string(receivedMessage->getInterface()).c_str() <<  ", " <<  simTime() - receivedMessage->getEmissionTime() << "," << receivedMessage->getSendingInterface() <<"\n";
			}
			file.close();

			// Environment Update
			environment.update_reception_state(vehicle_id_in_flow, receivedMessage->getVehicleIdInFlow(), 0, receivedMessage->getSendingInterface());

		}else{

			// Hit detection

			std::list<record>::iterator it;
			bool is_received = false;

			for(it = receivedMessages.begin(); it != receivedMessages.end(); ++it)
			{
				if ((it->id).compare(receivedMessage->getMessageId()) == 0)
				{
					is_received = true;
					break;
				}	
			}

			if (is_received)
			{
				message_hits++; 
				receivedMessages.erase(it);
				

				std::fstream file;
				file.open (csvFile, std::ios::app);

				if(file){
					file << "" << ", " << "" << ", " << simTime() << "_" << receivedMessage->getMessageId() << ", " << std::to_string(receivedMessage->getInterface()).c_str() <<  ", " <<  simTime() - receivedMessage->getEmissionTime() << "," << receivedMessage->getSendingInterface() <<"\n";
				}
				file.close();

				
			}else{
				
				record record_tmp;
				record_tmp.id = receivedMessage->getMessageId();
				record_tmp.latency = simTime() - receivedMessage->getEmissionTime();
				receivedMessages.push_front(record_tmp);
				
			}
			// Environment Update
			environment.update_reception_state(vehicle_id_in_flow, receivedMessage->getVehicleIdInFlow(), is_received, receivedMessage->getSendingInterface());

		}



    }
	
	
}


void HybridService::finish()
{
	ItsG5Service::finish();

	const std::string vehicle_id = mVehicleController->getVehicleId();

	std::fstream file;
	file.open(csvFile, std::ios::app);

	if(file){
		file << vehicle_id << ", Hits, " << std::to_string(message_hits).c_str() << ", Size, " << std::to_string(receivedMessages.size()).c_str() << "\n"; 
	}

	std::cout << "V " << vehicle_id << "\n";
	std::cout << "Hits " << std::to_string(message_hits).c_str() << "\n"; 
	std::cout << "LIst Size " << std::to_string(receivedMessages.size()).c_str() << "\n";
	
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
