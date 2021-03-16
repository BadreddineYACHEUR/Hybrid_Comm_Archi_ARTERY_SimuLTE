#ifndef ITSG5APP_H_
#define ITSG5APP_H_

#include "artery/application/ItsG5Service.h"

// forward declaration
namespace traci { class VehicleController; }

class ItsG5App : public artery::ItsG5Service
{
    public:
        void trigger() override;

    protected:
    	void indicate(const vanetza::btp::DataIndication&, omnetpp::cPacket*) override;
        void initialize() override;
        omnetpp::simsignal_t LteSignal;
        traci::VehicleController* mVehicleController = nullptr;
};

#endif /* ITSG5APP_H_ */
