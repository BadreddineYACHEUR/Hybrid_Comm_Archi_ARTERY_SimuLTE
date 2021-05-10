

#ifndef BASECOMMMODE_H_
#define BASECOMMMODE_H_
#include <omnetpp.h>
#include <omnetpp/csimplemodule.h>
#include <omnetpp/clistener.h>
#include <assert.h>

using namespace omnetpp;


class BaseCommMode : public cSimpleModule, public cListener

{
	public:
		BaseCommMode();

	protected:
		void initialize() override;
		void finish() override;
		void handleMessage(omnetpp::cMessage*) override;

	private:

};

#endif /* BASECOMMMODE_H_ */
