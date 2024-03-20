#include "Env.h"
#include "torch/torch.h"
#include <iostream>
#include <stdio.h>
#include <math.h>
#include <chrono>
#include <string>
#include <algorithm>
#include <tuple>

const int NB_VEHICLE = 4;

static int received_status[NB_VEHICLE][NB_VEHICLE + 1] = {0};

Environment::Environment(){
    // actions = ITS-G5, LTE, Both
    actions = torch::tensor({0, 1, 2});
    init();
    
}


void Environment::init(){
    done = false; 
    number_steps = 0;
    number_hits = 0;
}

std::tuple<double, bool> Environment::step(int sender_id, int service_type){

    number_steps++;

    double reward = 0;
    
    if(service_type == 0)
        reward = this->reward(sender_id);
    else    
        reward = this->reward_2(sender_id);

    // Check if the message was received by all platoon vehicles
    bool is_received_by_all = true;
    //std::cout << "Check if the message was received by all platoon vehicles ";
    
    for (int j=0; j<NB_VEHICLE; j++){
        if(((received_status[sender_id][j] == 0) && (j != sender_id)) || (received_status[sender_id][j] == 21) ){
            is_received_by_all = false;
        }
    }

    //std::cout << " \n";

    if(is_received_by_all)
        received_status[sender_id][NB_VEHICLE] += 1;


    // check if the game is finished
    if(received_status[sender_id][NB_VEHICLE] == 50){
        done = true;
        received_status[sender_id][NB_VEHICLE] = 0;
    }

    return std::tuple<double, bool>(reward, done);

}

double Environment::reward(int sender_id){

    double prr_its_g5_ = new_state[0].item().toDouble();
    double snr_its_g5_ = new_state[1].item().toDouble();

    double prr_lte_ = new_state[2].item().toDouble(); 
    double snr_lte_ = new_state[3].item().toDouble();


    double prr_its_g5 = state[0].item().toDouble();
    double snr_its_g5 = state[1].item().toDouble();

    double prr_lte = state[2].item().toDouble(); 
    double snr_lte = state[3].item().toDouble();

    // Normalization params
    snr_lte = (snr_lte <= max_snr_lte) ? ((snr_lte - min_snr_lte) / (max_snr_lte - min_snr_lte)) : 1; 
    snr_lte_ = (snr_lte_ <= max_snr_lte) ? ((snr_lte_ - min_snr_lte) / (max_snr_lte - min_snr_lte)) : 1;

    snr_its_g5 = (snr_its_g5 <= max_snr_its_g5) ? ((snr_its_g5 - min_snr_its_g5) / (max_snr_its_g5 - min_snr_its_g5)) : 1; 
    snr_its_g5_ = (snr_its_g5_ <= max_snr_its_g5) ? ((snr_its_g5_ - min_snr_its_g5) / (max_snr_its_g5 - min_snr_its_g5)) : 1;

    // Reward on Received status
    double reward_part_0 = 0;

    for(int i=0; i<NB_VEHICLE; i++){
        if(received_status[sender_id][i] == 2){
            reward_part_0 = reward_part_0 - 1; 
            number_hits++; 
        }else if((received_status[sender_id][i] == 0) && (i != sender_id)){
            reward_part_0 = reward_part_0 - 2;
        }
    }

    // Reward on good behavior 
    double reward_part_1 = 0;

    if(choosen_action == 0){
        if(snr_its_g5 < snr_tresh_g5)
            reward_part_1 -= (snr_tresh_g5 - snr_its_g5);
        else reward_part_1 = 0;

    }else if(choosen_action == 1){
        if(prr_lte < prr_tresh){
            reward_part_1 -= (prr_tresh - prr_lte);
            if(snr_lte < snr_tresh_lte)
                reward_part_1 -= (snr_tresh_lte - snr_lte);
        }
        else reward_part_1 = 0;
    }else{
        if((snr_its_g5 > snr_tresh_g5) || (snr_lte > snr_tresh_lte))
            reward_part_1 -= 0.1;
        else reward_part_1 = 0;
    } 

    // Reward on Net performance
    double reward_part_2 = 0;

    if(snr_its_g5_ < snr_its_g5)
        reward_part_2 -= (snr_its_g5 - snr_its_g5_);
    if(snr_lte_ < snr_lte)
        reward_part_2 -= (snr_lte - snr_lte_);

    // Summarized reward

    double reward = reward_part_0; //alpha * reward_part_0 + beta * reward_part_1;
    
    // Return the reward

    return reward;
}

double Environment::reward_2(int sender_id){

    double reward_part_0 = 0;

    for(int i=0; i<NB_VEHICLE; i++){
        if(received_status[sender_id][i] == 11){
            reward_part_0 = reward_part_0;
        }else if(((received_status[sender_id][i] == 0) && (i != sender_id))){
            reward_part_0 = reward_part_0 - 2;
        }else if(received_status[sender_id][i] == 21){
            reward_part_0 = reward_part_0 - 1;
        }else if(received_status[sender_id][i] == 22) {
            number_hits++;
        }
    }
    
    return reward_part_0;


}

void Environment::update_reception_state(int receiver_id, int sender_id, bool is_received, int comm_mode){

    if((receiver_id != -1) && (sender_id != -1)){

        if(comm_mode == 2)
            if(is_received){
                received_status[sender_id][receiver_id] = 22;
            }else
                received_status[sender_id][receiver_id] = 21;
        }else{
            received_status[sender_id][receiver_id] = 11;
    }
}

void Environment::clear_reception_state(int sender_id){
	
    for(int i=0; i<NB_VEHICLE ; i++){
        received_status[sender_id][i] = 0;
    }
        
}