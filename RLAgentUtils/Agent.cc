#include "Agent.h"
#include "Network.h"

#include <math.h>
#include <chrono>
#include <string>
#include <algorithm>
#include <tuple>


Agent::Agent(int input_dims, int num_actions, int hidden_dims):
        /*buffer(capacity),*/
        network(input_dims, hidden_dims, num_actions),
        target_network(input_dims, hidden_dims, num_actions),
        agent_memory(input_dims, mem_max), optimizer(network->parameters(), torch::optim::AdamOptions(0.0001)){

            observation_space = input_dims; 
            action_space = num_actions;
            

}

int Agent::choose_action(torch::Tensor state){
    
    if (torch::rand(1).item().toDouble() > epsilon){
        torch::Tensor actions = network->forward(state);
        int action = actions.argmax().item().toInt();
        return action;
    }else{
        return torch::rand(3).argmax().item().toInt();
    }

}

void Agent::store_transition(torch::Tensor state, torch::Tensor new_state, int action, double reward, bool done){
    agent_memory.store_transition(state, new_state, action, reward, done);
}

void Agent::replace_target_network(){
    if((learn_step_counter % replace_target_cnt) == 0){
        torch::save(network, "model.pt");
        torch::load(target_network, "model.pt");
    } 
}

void Agent::decrement_epsilon(){
    if (epsilon > epsilon_min)
        epsilon -= epsilon_decay;
    else 
        epsilon = epsilon_min;
}


void Agent::learn(){

    if (agent_memory.get_mem_ctr() < batch_size)
        return ;
    
    if(learn_step_counter % 1000 == 0)
        std::cout << "Agent is learning and epsilon = " << epsilon <<  "*************************************************\n";

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

    sample_actions =sample_actions.to(torch::kInt64);

    torch::Tensor q_value = q_values.gather(1, sample_actions.unsqueeze(1)).squeeze(1);
    
    torch::Tensor maximum_next_q_values_index = std::get<1>(next_q_values.max(1));
    torch::Tensor next_q_value = target_next_q_values.gather(1, maximum_next_q_values_index.unsqueeze(1)).squeeze(1);
    torch::Tensor expected_q_value = sample_rewards + gamma*next_q_value*(1-sample_terminals);
    
    torch::Tensor loss = torch::mse_loss(q_value, expected_q_value);

    loss.backward();
    optimizer.step();

    learn_step_counter += 1;

    decrement_epsilon();

}

/*    
    def save_models(self):
        self.q_eval.save_checkpoint()
        self.q_next.save_checkpoint()

    def load_models(self):
        self.q_eval.load_checkpoint()
        self.q_next.load_checkpoint()*/

 