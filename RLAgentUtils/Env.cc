#include "Env.h"
#include "torch/torch.h"
#include <iostream>
#include <stdio.h>
#include <math.h>
#include <chrono>
#include <string>
#include <algorithm>
#include <tuple>


static int received_status[6][7] = {0};

Environment::Environment(){
    // actions = ITS-G5, LTE, Both
    actions = torch::tensor({0, 1, 2});
    init();
    
}


void Environment::init(){
    done = false; 
    number_steps = 0;
}

std::tuple<double, bool> Environment::step(int platonId){

    number_steps++;

    double reward = this->reward(platonId);

    // Check if the message was received by all platoon vehicles
    bool is_received_by_all = true;
    //std::cout << "Check if the message was received by all platoon vehicles ";
    
    for (int j=0; j<6; j++){
        if(((received_status[platonId][j] == 0) && (j != platonId)) || (received_status[platonId][j] == 2)){
            is_received_by_all = false;
        }

        //std::cout << received_status[platonId][j] << " ";   
    }

    //std::cout << " \n";

    if(is_received_by_all)
        received_status[platonId][6] += 1;


    // check if the game is finished
    if(received_status[platonId][6] == 100){
        done = true;
        received_status[platonId][6] = 0;
    }

    return std::tuple<double, bool>(reward, done);

}

double Environment::reward(int platonId){

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

    for(int i=0; i<6; i++){
        if(received_status[platonId][i] == 2){
            reward_part_0 = reward_part_0 - 1;  
        }else if((received_status[platonId][i] == 0) && (i != platonId)){
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

    //double reward = (0.8 * reward_part_0) + (0.2 * (beta * reward_part_1 + alpha * reward_part_2));
    double reward = reward_part_0; //alpha * reward_part_0 + beta * reward_part_1;
    //std::cout << "reward in step: " << reward_part_0 << " " << reward_part_1 << " " << reward_part_2 << "\n";
    
    // Return the reward
    
    return reward;
}

void Environment::update_reception_state(int platoonId, int senderId, bool is_received){

    if((platoonId != -1) && (senderId != -1))
        if (is_received){
            received_status[senderId][platoonId] = 2;
        }else
            received_status[senderId][platoonId] = 1;
}

void Environment::clear_reception_state(int platoonId){
	
    for(int i=0; i<6 ; i++){
        received_status[platoonId][i] = 0;
    }
        
}