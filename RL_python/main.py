#import os
 #for keras the CUDA commands must come before importing the keras libraries
#os.environ['CUDA_DEVICE_ORDER'] = 'PCI_BUS_ID'
#os.environ['CUDA_VISIBLE_DEVICES'] = '0'
#os.environ['TF_FORCE_GPU_ALLOW_GROWTH'] = 'true'
#import gym
#from gym import wrappers
#from gym import spaces
import os
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3'
import warnings
warnings.filterwarnings("ignore", message=r"Passing", category=FutureWarning)

import numpy as np
from DDQN import DDQNAgent
#from utils import plotLearning

import socket
import threading
import time
from random import randint


PORT = 8080

SERVER = socket.gethostbyname(socket.gethostname())
ADDR = (SERVER, PORT)
FORMAT = 'utf-8'

server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.bind(ADDR)

SP = np.array([0.01, 0.004, 0.001])
observation = np.zeros(6)
observation_ = np.zeros(6)
action = 0

if __name__ == '__main__':
   # initialize

    nb_actions = 3

    input_dim = 6

    ddqn_agent = DDQNAgent(alpha=0.0005, gamma=0.99, n_actions=nb_actions, epsilon=1.0,
                  batch_size=64, input_dims=input_dim)
    ddqn_scores = []
    eps_history = []
   
    server.listen()
    conn, addr = server.accept()
    print("\n Connection Success !!!!!!!!!!!!!!!!!!!! \n")
    
    while True:  

        msg = conn.recv(1024).decode()

        print("Message is: ")
        print(msg)
        print("\n")

        if msg:
            parts = msg.split("#")
            tokens_part_1 = parts[0].split("$")
            tokens_part_2 = parts[1].split("$")

            print("agent storing and learning")
            reward = float(tokens_part_2[0])
            done = int(tokens_part_2[1])

            observation_[0] = 1.0
            for j in range(1,3):
                observation_[j] = float(tokens_part_2[j+1])

            observation_[4] = 0.99
            observation_[5] = 50

            observation[0] = 1.0
            for j in range(1,3):
                observation[j] = float(tokens_part_1[j-1])

            observation[4] = 0.99
            observation[5] = 50

            print(observation)
            print(observation_)
            print(reward)
            print(action)
            print(done)
            
            if reward != 1:
                ddqn_agent.remember(observation, action, reward, observation_, done)
                ddqn_agent.learn()


            action = ddqn_agent.choose_action(observation)

            print(observation)
        
            conn.send(str(action).encode())

            print (action)
    
