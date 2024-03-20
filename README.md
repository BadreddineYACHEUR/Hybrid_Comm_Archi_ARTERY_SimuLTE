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



To implement the hybrid architecture, we defined a hybrid application that is linked to a set of sub applications that are defined according to the used technologie. In our implementation, we used LTE mode 3 and ITS-G5, so we had a set of two sub applications linked to an ITS-G5 application and an LTE application. Communicaiton between sub applications and the main application is done using OMNeT++ signals



What to add else: 

We created a representation of the managment layer to do the cross layer communication between the access layer and the hybrid communication layer that deals with the DRL algorithm. Then we added the managmend layer module to the "car.ned" module. Furthermore, we modified the "radio.cc" (line 546), and the "RealisticChannelModel.cc" (line 1571) file to update the SNIR and the PRR values in the managment layer 



Conclusion:
This implementation is not optimal and clean 100%, we can add lot of optimisation to the code and to the architecture.
I will try from time to time to add some modifications and make it more usable.

Feel free to contact me at byacheur@u-bordeaux.fr