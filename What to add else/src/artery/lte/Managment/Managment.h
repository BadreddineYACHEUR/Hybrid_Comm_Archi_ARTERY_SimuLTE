#ifndef ARTERY_MANAGMENT_MANAGMENT_H_
#define ARTERY_MANAGMENT_MANAGMENT_H_


#include <iostream>
#include <algorithm>
#include <omnetpp.h>
#include <omnetpp/csimplemodule.h>

namespace artery 
{
namespace Managment 
{

class Managment : public omnetpp::cSimpleModule
{
	public: 
		Managment(){
			
		}

	public: 
		double SINR_ITS_G5, PRR_ITS_G5, PRR_LTE, SINR_LTE;
		double SINR_ITS_G5_first, PRR_ITS_G5_first, PRR_LTE_first, SINR_LTE_first;
		bool first_message_ITS_G5 = true;
		bool first_message_LTE = true;

	protected:
		void initialize() override;
		void finish() override;
		

	private:

};
}
}


#endif