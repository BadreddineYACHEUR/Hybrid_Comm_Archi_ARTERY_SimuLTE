#ifndef HYBRIDSERVICE_H_
#define HYBRIDSERVICE_H_

#include "artery/application/ItsG5Service.h"
#include "artery/application/VehicleDataProvider.h"


class HybridService : public artery::ItsG5Service
{
	public:
		HybridService();

		void indicate(const vanetza::btp::DataIndication&, omnetpp::cPacket*) override;
		void trigger() override;
		void receiveSignal (cComponent*, omnetpp::simsignal_t, cObject* /*const char**/, cObject*) override;
		void sendToMainApp (omnetpp::cMessage* msg, std::string id);


	protected:
		enum V_ROLE {FREE, LEADER, FOLLOWER, JOINER};
    	V_ROLE role = FREE;

		void initialize() override;
		void finish() override;
		void handleMessage(omnetpp::cMessage*) override;
		void CACCSpeedControl(std::string vehicle_id, double desired_speed, double vehicle_speed);
        void CACCGapControl(std::string vehicle_id, std::string pre_vehicle_id, double vehicle_speed, double preceding_vehicle_speed, double distance, double vehicle_accel);
        double squarDistance(double xPosV1, double xPosV2, double yPosV1, double yPosV2);

        // Dup Messages Managment

        std::list <std::string> receivedMessages;

		//Signals

		omnetpp::simsignal_t toMainAppSignal;

		// Platoon managment

		std::string csvFile;
        int messageId = 0;
        int platoonId;
        int platoonSize = 0;
        double leader_speed = 0;
        int message_hits = 0;
        bool is_in_platoon = false;
        std::list <platoonMember> platoonMembers;



	private:
		traci::VehicleController* mVehicleController = nullptr;
		omnetpp::cMessage* m_self_msg;

};

#endif /* HYBRIDSERVICE_H_ */
