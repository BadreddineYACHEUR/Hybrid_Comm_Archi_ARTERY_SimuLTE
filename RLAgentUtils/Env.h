#ifndef ENV_H_
#define ENV_H_

#include "torch/torch.h"
#include <iostream>
#include <stdio.h>

class Environment
{
    public:
        /* Environmnet propreties */
        torch::Tensor actions, state, new_state;
        int observation_space = 6;
        int action_space = 3;
        int number_steps = 0;
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
        double alpha = 0.8;
        double beta = 0.2;

        /* Model saving files */
        std::string pt_net = "net_model.pt";
        std::string pt_target = "target_model.pt";

    public:

        Environment();
        void init();
        std::tuple<double, bool> step(int platonId);
        double reward(int platonId);
        void update_reception_state(int platoonId, int senderId, bool is_received);
        void clear_reception_state(int platoonId);
};

#endif