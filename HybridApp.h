#ifndef HYBRIDAPP_H_
#define HYBRIDAPP_H_

#include <omnetpp.h>
#include <omnetpp/csimplemodule.h>
#include <omnetpp/clistener.h>
#include <assert.h>

using namespace omnetpp;


class HybridApp : public cSimpleModule, public cListener
{
	public:
		HybridApp();

	protected:
		void initialize();
		void finish();
		void handleMessage(omnetpp::cMessage*) override;

		//Gates
		int fromServiceIn, hybridAppIn[2], hybridAppOut[2];
	private:

};

#endif /* HYBRIDAPP_H_ */
