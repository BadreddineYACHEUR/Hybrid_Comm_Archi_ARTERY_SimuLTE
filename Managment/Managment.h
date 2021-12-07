#ifndef MANAGMENT_H_
#define MANAGMENT_H_


#include <iostream>
#include <algorithm>
#include <omnetpp.h>
#include <omnetpp/csimplemodule.h>
#include <omnetpp/clistener.h>
#include <assert.h>

using namespace omnetpp;

class Managment: public cSimpleModule, public cListener
{
	public: 
        Managment(){
            
        }

    public: 
        double SINR_ITS_G5, PRR_LTE, SINR_LTE;

	protected:
		void initialize() override;
		void finish() override;
		

	private:

};
#endif