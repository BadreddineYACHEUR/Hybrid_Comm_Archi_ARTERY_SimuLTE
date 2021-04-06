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
    	enum V_ROLE {FREE, LEADER, FOLLOWER, JOINER};
    	V_ROLE role = FREE;
    	void indicate(const vanetza::btp::DataIndication&, omnetpp::cPacket*) override;
        void initialize() override;
        void CACCSpeedControl(std::string vehicle_id, double desired_speed, double vehicle_speed);
        void CACCGapControl(double vehicle_speed, double preceding_vehicle_speed, double distance, double vehicle_accel);
        double squarDistance(double xPosV1, double xPosV2, double yPosV1, double yPosV2);

        omnetpp::simsignal_t LteSignal;
        int messageId = 0;
        int platoonId;
        int platoonSize = 0;
        traci::VehicleController* mVehicleController = nullptr;
};

#endif /* ITSG5APP_H_ */
