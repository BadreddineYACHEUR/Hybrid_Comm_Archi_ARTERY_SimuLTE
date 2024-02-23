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



simple HybridApp
{
    parameters:

    	//Number of sub applications
    	int num_apps = default(0);

    	// stats: Number of message sent
    	int num_messages = 0;

    	// Other parameters

        @class(HybridApp);
        @display("i=block/app2;bgb=214.88,131.92");


        @signal[toHybridServiceSignal];



    gates:
    
        input hybridAppIn[];
        output hybridAppOut[];
    
}
