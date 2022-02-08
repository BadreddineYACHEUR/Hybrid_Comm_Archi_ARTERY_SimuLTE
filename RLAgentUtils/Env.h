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
   
    
    // Normalization params
    double min_snr_lte = 1;
    double max_snr_lte = 60;

    double min_snr_its_g5 = 0.1;
    double max_snr_its_g5 = 200;

    /* Channel specs */
    double prr_tresh = 0.9;
    double snr_tresh_lte = (12 - min_snr_lte) / (max_snr_lte - min_snr_lte);
    double snr_tresh_g5 = (20 - min_snr_its_g5) / (max_snr_its_g5 - min_snr_its_g5);


    /* Reward parameters */
    double alpha = 0.2;
    double beta = 0.8;

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
        done = false;

        if(number_messages > 10000){
            number_messages = 0;
            done = true;
        }
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

        // Normalization params

        snr_lte = (snr_lte <= max_snr_lte) ? ((snr_lte - min_snr_lte) / (max_snr_lte - min_snr_lte)) : 1; 
        snr_lte_ = (snr_lte_ <= max_snr_lte) ? ((snr_lte_ - min_snr_lte) / (max_snr_lte - min_snr_lte)) : 1;

        snr_its_g5 = (snr_its_g5 <= max_snr_its_g5) ? ((snr_its_g5 - min_snr_its_g5) / (max_snr_its_g5 - min_snr_its_g5)) : 1; 
        snr_its_g5_ = (snr_its_g5_ <= max_snr_its_g5) ? ((snr_its_g5_ - min_snr_its_g5) / (max_snr_its_g5 - min_snr_its_g5)) : 1;

        // std::cout << "snr = " << snr_tresh_lte << " " << snr_tresh_g5 << " " << snr_its_g5 << " " << snr_its_g5_ << " " << snr_lte << " " << snr_lte_ << " " << choosen_action <<  "\n"; 


        double reward_part_1, reward_part_2;

        if(choosen_action == 0){
            if((snr_its_g5 > snr_tresh_g5) && (snr_lte <= snr_tresh_lte))
                reward_part_1 = 1 + (snr_its_g5 - snr_tresh_g5);
            else reward_part_1 = 0;

        }else if(choosen_action == 1){
            if(prr_lte > prr_tresh){
                reward_part_1 = 1 + (prr_lte - prr_tresh);
                if((snr_lte > snr_tresh_lte) && (snr_its_g5 <= snr_tresh_g5))
                    reward_part_1 += (snr_lte - snr_tresh_lte);
            }
            else reward_part_1 = 0;
        }else{
            if((snr_its_g5 <= snr_tresh_g5) && (snr_lte <= snr_tresh_lte))
                reward_part_1 = 1;
            else reward_part_1 = 0;
        } 

        double diff_snr = (snr_its_g5_ - snr_its_g5) + (snr_lte_ - snr_lte);
        double diff_prr = (prr_lte_ - prr_lte);
        reward_part_2 = diff_snr;

        double reward = beta * reward_part_1 + alpha * reward_part_2;
        
        // std::cout << "Reward = " << reward << "\n"; 
        return reward;
    }
};

#endif