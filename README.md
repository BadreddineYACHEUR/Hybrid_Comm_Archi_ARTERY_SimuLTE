In this Hybrid vehicular communciaiton Implementation we have:

- The implementation of the communciaiton modes.
- The implementation of two services that are used to assess the performances of the hybrid communciaiton
- The definition of two Deep reinforcement learning implementations:
    - using Python library PyTorch
    - using the C++ library libTorch

To reproduce this architecture on your OMNeT++ environement you should install: 
- SUMO
- Artery
- SimuLTE
- Python or link the C++ library as showen in the "CMAkeLists.txt" file

The implementation of the deep reinforcement learning agent in C++ is specefied within the RLAgentUtils folder:
- The Network.h file defines the neural network used in the DRL algorithm
- The Memory.h defines a class where learning transistion's atributes are stored
- The Env .cc and .h files is a class that updates the environment state by executing the step() method and deals with the reward that should be produced after an action is done
- The Agent .cc and .h files defines the DRL agent and the actions it takes to learn

The implementation of the deep reinforcement learning agent using Python is specefied within the RL_python folder

To implement the hybrid architecture, we defined a hyybrid application that is linked to a set of sub applications that are defined according to the used technologie. In our implementation, we used LTE mode 3 and ITS-G5, so we had a set of two sub applications linked to an ITS-G5 application and an LTE application. Communicaiton between sub applications and the main application is done using OMNeT++ signals
We created a representation of the managment layer to do the cross layer communication between the access layer and the hybrid communication layer that deals with the DRL algorithm.


Conclusion:
This implementation is not optimal and clean 100%, we can add lot of optimisation to the code and the the architecture.