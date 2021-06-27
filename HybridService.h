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
		void sendToMainApp (omnetpp::cMessage* msg);
		std::string getVehicleId();


	protected:
		void initialize() override;
		void finish() override;
		void handleMessage(omnetpp::cMessage*) override;

		//Signals

		omnetpp::simsignal_t toMainAppSignal;


	private:
		std::string vehicleId;
		traci::VehicleController* mVehicleController = nullptr;
		omnetpp::cMessage* m_self_msg;

};

#endif /* HYBRIDSERVICE_H_ */
