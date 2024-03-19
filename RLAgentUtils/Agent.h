#ifndef AGENT_H_
#define AGENT_H_

#pragma once

#include "torch/torch.h"
#include <iostream>
#include <stdio.h>
#include "Network.h"
#include "Env.h"
#include "Memory.h"
#include <string>

class Agent{

    private: Net network, target_network;
    //private: torch::optim::Adam dqn_optimizer;
    private: std::string checkpoint;
    private: double epsilon = 1.0;
    private: double epsilon_min = 0.01;
    private: double epsilon_decay = 5e-7;
    private: float lr = 0.01;
    private: float gamma = 0.99;
    private: torch::optim::Adam optimizer;
    private: int action_space, observation_space;
    
    private: int learn_step_counter = 0;
    private: int replace_target_cnt = 10000;
    private: int batch_size = 32;
    private: int mem_max = 500000;
    
    private: Memory agent_memory;

    public:
        Agent(int input_dims, int num_actions, int hidden_dims);

        int choose_action(torch::Tensor state);
        void store_transition(torch::Tensor state, torch::Tensor new_state, int action, double reward, bool done);

        void load_enviroment();

        void replace_target_network();
        void decrement_epsilon();

        void learn();

};

#endif