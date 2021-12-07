//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

import artery.application.ItsG5Service;
import BaseProtocol;
import SubApplication;
import BaseCommMode;
import HybridApp;
import Managment.Managment;

module HybridService like ItsG5Service
{
    parameters:
        @class(HybridService);
        // @signal[PlatooningUnicastReceived](type=cObject);

        //string protocol_type = "BaseProtocol";

        // duplication or agregation

        string communication_mode;


        int num_sub_apps = default(2);

        @display("i=block/app2;bgb=214.88,131.92");

        //Signals

        @signal[toMainAppSignal];

    submodules:

        mode: <communication_mode> like BaseCommMode {
            @display("p=189.675,21.6");
        }

        mainApp: HybridApp {

            @display("p=63.45,102.600006");
            gates:
                hybridAppIn[num_sub_apps];
                hybridAppOut[num_sub_apps];
        }

        subApp[num_sub_apps]: SubApplication {
            @display("p=63.45,43.2,row,50");
        }

        managmentLayer: Managment {
            @display("p=189.675,84.375");
        }



        //prot: <protocol_type> like BaseProtocol {
          //  @display("p=104.72,90.44");
        //}



    connections allowunconnected:


        for i=0..(num_sub_apps - 1) {
            mainApp.hybridAppOut[i] --> subApp[i].subApplicationIn;
            subApp[i].subApplicationOut --> mainApp.hybridAppIn[i];
        }

}
