#ifndef BASEPROTOCOL_H_
#define BASEPROTOCOL_H_

#include <omnetpp.h>
#include <omnetpp/csimplemodule.h>
#include <omnetpp/clistener.h>
#include <assert.h>

using namespace omnetpp;


class BaseProtocol : public cSimpleModule, public cListener
{
	public:
		BaseProtocol();

	protected:
		void initialize() override;
		void finish() override;
		void handleMessage(omnetpp::cMessage*) override;

	private:

};

#endif /* BASEPROTOCOL_H_ */
