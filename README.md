In this Hybrid vehicular communication implementation, we have:

- The implementation of the communication modes.
- The implementation of two services used to assess the performance of the hybrid communication.
- The definition of two Deep Reinforcement Learning implementations:
    - using the Python library PyTorch.
    - using the C++ library libTorch.



To reproduce this architecture in your OMNeT++ environment, you should install:

- SUMO
- Artery
- SimuLTE
- Python or link the C++ library as shown in the "CMakeLists.txt" file.



The implementation of the deep reinforcement learning agent in C++ is specified within the RLAgentUtils folder:

- The Network.h file defines the neural network used in the DRL algorithm.
- The Memory.h defines a class where learning transition attributes are stored.
- The Env.cc and .h files represent a class that updates the environment state by executing the step() method and deals with the reward produced after an action is performed.
- The Agent.cc and .h files define the DRL agent and the actions it takes to learn.

The implementation of the deep reinforcement learning agent using Python is specified within the RL_python folder.



To implement the hybrid architecture, we defined a hybrid application linked to a set of sub-applications defined according to the used technology. In our implementation, we used LTE mode 3 and ITS-G5, resulting in a set of two sub-applications linked to an ITS-G5 application and an LTE application. Communication between sub-applications and the main application is facilitated using OMNeT++ signals.

Additionally, we created a representation of the management layer to enable cross-layer communication between the access layer and the hybrid communication layer, which deals with the DRL algorithm. Subsequently, we integrated the management layer module into the "car.ned" module. Furthermore, we modified the "radio.cc" (line 546) and the "RealisticChannelModel.cc" (line 1571) files to update the SNIR and PRR values in the management layer.



Conclusion:
While this implementation is not optimal and entirely clean, there is potential for further optimization both in the code and architecture. I will periodically work on making modifications to enhance its usability.

Feel free to contact me at byacheur@u-bordeaux.fr.
