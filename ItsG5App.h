#ifndef ITSG5APP_H_
#define ITSG5APP_H_

#include "artery/application/ItsG5Service.h"
#include <list>

// forward declaration
namespace traci { class VehicleController; }

struct platoonMember {
    std::string vehicleId;
    int idInPlatoon;
}; 

class ItsG5App : public artery::ItsG5Service
{
    public:
        void trigger() override;

    protected:
    	enum V_ROLE {FREE, LEADER, FOLLOWER, JOINER};
    	V_ROLE role = FREE;
    	void indicate(const vanetza::btp::DataIndication&, omnetpp::cPacket*) override;
        void initialize() override;
        void receiveSignal (omnetpp::cComponent*, omnetpp::simsignal_t, omnetpp::cObject* /*const char**/, omnetpp::cObject*) override;
        void handleMessage(omnetpp::cMessage*) override;

        void CACCSpeedControl(std::string vehicle_id, double desired_speed, double vehicle_speed);
        void CACCGapControl(std::string vehicle_id, std::string pre_vehicle_id, double vehicle_speed, double preceding_vehicle_speed, double distance, double vehicle_accel);
        double squarDistance(double xPosV1, double xPosV2, double yPosV1, double yPosV2);
        void writePRInfo(std::string fileName, std::string column1, std::string column2);

        omnetpp::simsignal_t LteSignal;

        omnetpp::simsignal_t itsG5ToSubAppSignal;
        std::string csvFile;
        int messageId = 0;
        int platoonId;
        int platoonSize = 0;
        double leader_speed = 0;

        bool is_in_platoon = false;
        std::list <platoonMember> platoonMembers;
        traci::VehicleController* mVehicleController = nullptr;

        
};

#endif /* ITSG5APP_H_ */
