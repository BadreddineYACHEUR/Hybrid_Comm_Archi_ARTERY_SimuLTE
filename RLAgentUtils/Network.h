#ifndef NETWORK_H_
#define NETWORK_H_

#pragma once


#include <iostream>
#include <torch/torch.h>
#include <string>

struct NetImpl : torch::nn::Module {

	std::string network_id;

	NetImpl(int fc1_dims, int fc2_dims, int out_dims):
		fc1(fc1_dims, fc1_dims), fc2(fc1_dims, fc2_dims), out(fc2_dims, out_dims){
		register_module("fc1", fc1);
		register_module("fc2", fc2);
		register_module("out", out);
		
	}
	NetImpl(){}

	torch::Tensor forward(torch::Tensor x){
	
		x = torch::relu(fc1(x));
		x = torch::relu(fc2(x));
		x = out(x);
		
		return x ;
	}

	torch::nn::Linear fc1{nullptr}, fc2{nullptr}, out{nullptr};

};

TORCH_MODULE(Net);

#endif
