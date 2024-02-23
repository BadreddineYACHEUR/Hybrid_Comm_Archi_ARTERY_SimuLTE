

#ifndef DUPCOMMMODE_H_
#define DUPCOMMMODE_H_

#include <omnetpp.h>
#include <omnetpp/csimplemodule.h>
#include <omnetpp/clistener.h>
#include <assert.h>

using namespace omnetpp;


class DupCommMode : public cSimpleModule, public cListener

{
	public:
		DupCommMode();

	protected:
		void initialize() override;
		void finish() override;
		void handleMessage(omnetpp::cMessage*) override;

	private:

};

#endif /* DUPCOMMMODE_H_ */
