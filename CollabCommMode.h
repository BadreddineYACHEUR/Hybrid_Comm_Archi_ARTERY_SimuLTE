#ifndef COLLABCOMMMODE_H_
#define COLLABCOMMMODE_H_

#include <omnetpp.h>
#include <omnetpp/csimplemodule.h>
#include <omnetpp/clistener.h>
#include <assert.h>

using namespace omnetpp;


class CollabCommMode : public cSimpleModule, public cListener
{
	public:
		CollabCommMode();

	protected:
		void initialize() override;
		void finish() override;
		void handleMessage(omnetpp::cMessage*) override;

	private:

};

#endif /* COLLABCOMMMODE_H_ */
