#ifndef LTEAPP_H_
#define LTEAPP_H_

#include <inet/transportlayer/contract/udp/UDPSocket.h>
#include <omnetpp/clistener.h>
#include "hybrid_msgs/HybridServicesMessages_m.h"
#include <omnetpp/csimplemodule.h>


namespace traci { class VehicleController; }

class LteApp : public omnetpp::cSimpleModule, public omnetpp::cListener
{


protected:
    int numInitStages() const override;
    void initialize(int stage) override;
    void finish() override;
    void handleMessage(omnetpp::cMessage*) override;
    //void receiveSignal (cComponent*, simsignal_t, const char*, cObject*) override;
    void receiveSignal(omnetpp::cComponent*, omnetpp::simsignal_t, omnetpp::cObject*, omnetpp::cObject*) override;

private:
    void sendV2XMessage(PlatooningMessage*);
    void processV2XMessage(PlatooningMessage&);

    inet::UDPSocket socket;
    inet::L3Address mcastAddress;
    int mcastPort;
    int messageReceived;
    traci::VehicleController* vehicleController = nullptr;

    omnetpp::simsignal_t lteToSubAppSignal;


};


#endif /* LTEAPP_H_*/

