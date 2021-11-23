#ifndef MEMORY_H_
#define MEMORY_H_

#include <iostream>
#include "torch/torch.h"
#include <algorithm>
#include <tuple>

class Memory{

    int mem_size, mem_cntr;
    torch::Tensor state_memory, new_state_memory, reward_memory, action_memory, terminal_memory;

    public:
        Memory(int input_dims, int mem_max){
            mem_cntr = 0;
            mem_size = mem_max;
            state_memory = torch::zeros({mem_size, input_dims});
            new_state_memory = torch::zeros({mem_size, input_dims});
            action_memory = torch::zeros(mem_size);
            reward_memory = torch::zeros(mem_size);
            terminal_memory = torch::zeros(mem_size);
        }

        void store_transition(torch::Tensor state, torch::Tensor new_state, int action, double reward, double terminal){

            int index = mem_cntr % mem_size;

            state_memory[index] = state;
            action_memory[index] = action;
            new_state_memory[index] = new_state;
            reward_memory[index] = reward;
            terminal_memory[index] = terminal;

            mem_cntr += 1;
        }

        std::tuple<torch::Tensor, torch::Tensor, torch::Tensor, torch::Tensor, torch::Tensor> \
        sample_memory(int batch_size, int input_dims){        
            int mem_max = std::min(mem_cntr, mem_size);        

            // Choose_random implementation
            int i;
            torch::Tensor proba = torch::rand(mem_max);

            torch::Tensor batch_states = torch::zeros({batch_size-1, input_dims});
            torch::Tensor batch_new_states = torch::zeros({batch_size-1, input_dims});
            torch::Tensor batch_rewards = torch::zeros(batch_size-1);
            torch::Tensor batch_actions = torch::zeros(batch_size-1);
            torch::Tensor batch_terminals = torch::zeros(batch_size-1);

            for(i=0; i< (batch_size-1); i++){

                int index = proba.argmax().item().toInt();
                proba[index] = 0;
                batch_actions[i] = action_memory[index];
                batch_rewards[i] = reward_memory[index];
                batch_terminals[i] = terminal_memory[index];
                batch_states[i] = state_memory[index];
                batch_new_states[i] = new_state_memory[index];
                
            }
            return std::tuple<torch::Tensor, torch::Tensor, \
                                torch::Tensor, torch::Tensor, torch::Tensor>(\
                                batch_states, batch_new_states, batch_actions, batch_rewards, batch_terminals);
                    
        }

        int get_mem_ctr(){
            return mem_cntr;
        }

        int get_mem_size(){
            return mem_size;
        }

};

#endif