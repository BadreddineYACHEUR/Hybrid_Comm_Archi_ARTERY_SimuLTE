

#ifndef SINGLECOMMMODE_H_
#define SINGLECOMMMODE_H_


#include <omnetpp.h>
#include <omnetpp/csimplemodule.h>
#include <omnetpp/clistener.h>
#include <assert.h>

using namespace omnetpp;


class SingleCommMode : public cSimpleModule, public cListener
{
	public:
		SingleCommMode();

	protected:
		void initialize() override;
		void finish() override;
		void handleMessage(omnetpp::cMessage*) override;

	private:

};

#endif /* SINGLECOMMMODE_H_ */
