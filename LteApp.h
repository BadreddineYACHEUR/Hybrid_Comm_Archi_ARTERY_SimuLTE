#ifndef LTEAPP_H_HH5TITG4
#define LTEAPP_H_HH5TITG4

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
    void receiveSignal (cComponent*, simsignal_t, cObject* /*const char**/, cObject*) override;

private:
    void sendV2XMessage(PlatooningMessage*);
    void processV2XMessage(PlatooningMessage&);

    inet::UDPSocket socket;
    inet::L3Address mcastAddress;
    int mcastPort;
    int messageReceived;
    traci::VehicleController* vehicleController = nullptr;
};


#endif /* LTEAPP_H_HH5TITG4 */

