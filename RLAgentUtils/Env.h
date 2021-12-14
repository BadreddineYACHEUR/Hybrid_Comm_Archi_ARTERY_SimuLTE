#ifndef ENV_H_
#define ENV_H_

#include "torch/torch.h"
#include <iostream>
#include <stdio.h>

struct Environment
{
    /* Environmnet propreties */
    torch::Tensor actions, state, new_state;
    int observation_space = 6;
    int action_space = 3;
    int number_messages = 0;
    int choosen_action;
    bool done = false;
   
    
    /* Channel specs */ 
    double prr_tresh = 0.9;
    double snr_tresh = 8;

    /* Reward parameters */
    double alpha = 0.3;
    double beta = 0.7;

    /* Model saving files */

    std::string pt_net = "net_model.pt";
    std::string pt_target = "target_model.pt";


    Environment(){
        // actions = ITS-G5, LTE, Both
        actions = torch::tensor({0, 1, 2});
        init();
        
    }


    void init(){
        done = false; 
        number_messages = 0;
    }

    std::tuple<double, bool> step(){


        double reward = this->reward();

        if(number_messages > 10000)
            done = true;
        else
            number_messages++;

        return std::tuple<double, bool>(reward, done);

    }

    double reward(){

        //double prr_its_g5 = new_state[0].item().toDouble();

        double snr_its_g5_ = new_state[1].item().toDouble();
        double prr_lte_ = new_state[2].item().toDouble(); 
        double snr_lte_ = new_state[3].item().toDouble();

        double snr_its_g5 = state[1].item().toDouble();
        double prr_lte = state[2].item().toDouble(); 
        double snr_lte = state[3].item().toDouble();
        
        double diff_snr = (snr_its_g5_ - snr_its_g5) + (snr_lte_ - snr_lte);
        double diff_prr = (prr_lte_ - prr_lte);

        // double part1 = (average_snr - snr_tresh) / (average_snr + snr_tresh);
        // double part2 = (average_prr - prr_tresh) / (average_prr + prr_tresh);

        double reward = (alpha * diff_snr + beta * diff_prr);
        

        return reward;
    }
};

#endif