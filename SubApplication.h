#ifndef SUBAPPLICATION_H_
#define SUBAPPLICATION_H_

#include <omnetpp.h>
#include <omnetpp/csimplemodule.h>
#include <omnetpp/clistener.h>
#include <assert.h>


using namespace omnetpp;


class SubApplication : public cSimpleModule, public cListener
{
	public:
		SubApplication();

	protected:
		void initialize();
		void finish();
		void receiveSignal (cComponent*, simsignal_t, cObject* /*const char**/, cObject*) override;
		void handleMessage(omnetpp::cMessage*) override;
		void sendToMainApp (omnetpp::cMessage* msg);

		omnetpp::simsignal_t toLteSignal;
		omnetpp::simsignal_t toItsG5Signal;

		std::string interfaceType;
		//Gates 
		int subApplicationIn, subApplicationOut;
	private:

};

#endif /* SUBAPPLICATION_H_ */
