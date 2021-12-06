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
    bool done = false;
   
    
    /* Channel specs */ 
    double prr_tresh = 0.9;
    double snr_tresh = 8;

    /* Reward parameters */
    double alpha = 0.3;
    double beta = 0.7;

    Environment(){
        // actions = ITS-G5, LTE, Both
        actions = torch::tensor({0, 1, 2});
        
    }
    void init(){
        done = false; 
        number_messages = 0;
    }

    std::tuple<torch::Tensor, double, bool> step(int action){

        torch::Tensor new_state;
        
        if (action == 0)
            new_state = send_message_g5();
        else if (action == 1)
            new_state = send_message_lte();
        else new_state = send_message_both();

        this->new_state = new_state;

        double reward = this->reward();

        if(number_messages > 10000)
            done = true;
        else
            number_messages++;

        if(number_messages % 100 == 0)
            std::cout << number_messages << " sent \n";

        return std::tuple<torch::Tensor, double, bool>(new_state, reward, done);

    }

    double reward(){

        double prr_its_g5 = new_state[0].item().toDouble(); 
        double snr_its_g5 = new_state[1].item().toDouble();
        double prr_lte = new_state[2].item().toDouble(); 
        double snr_lte = new_state[3].item().toDouble();
        
        int rm = std::rand() % 2;

        double average_snr = (snr_its_g5 + snr_lte) / 2;
        double average_prr = (prr_its_g5 + prr_lte) / 2;

        double part1 = (average_snr - snr_tresh) / (average_snr + snr_tresh);
        double part2 = (average_prr - prr_tresh) / (average_prr + prr_tresh);

        double reward = rm * (alpha * part1 + beta * part2);
        

        return reward;
    }


    torch::Tensor send_message_g5(){
        return torch::tensor({((double)(std::rand() % 10) / 10), (double)(std::rand() % 25), ((double)(std::rand() % 10) / 10), (double)(std::rand() % 25), 0.99, 50.0});
    }
    torch::Tensor send_message_lte(){
        return torch::tensor({((double)(std::rand() % 10) / 10), (double)(std::rand() % 25), ((double)(std::rand() % 10) / 10), (double)(std::rand() % 25), 0.99, 50.0});
    }
    torch::Tensor send_message_both(){
        return torch::tensor({((double)(std::rand() % 10) / 10), (double)(std::rand() % 25), ((double)(std::rand() % 10) / 10), (double)(std::rand() % 25), 0.99, 50.0});
    }

};

#endif