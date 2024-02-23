packet PlatooningMessage
{

	string messageId;

	string vehicleId;

	//platoon id
	int platoonId = -1;
	int idInPlatoon = -1;
	
	//message type
	int messageType; // 0 = Simple platooning message
					 // 1 = Join request
					 // 2 = Join response

	int interface; 	// 0 = ITS-G5
					// 1 = LTE-V2X

	int sending_interface; 	// 0 = ITS-G5
							// 1 = LTE-V2X
							// 3 = Both
					
	//leader response
	bool accepted = false;
	string joinerId;
	int platoonSize = 0;
	int affectedId;


    

    string edgeName;
    int laneIndex;
    double positionX = 0;
	double positionY = 0;


    double speed;
    simtime_t time;
}


packet FakeCAMMessage
{
	string messageId;

    string vehicleId;

    string edgeName;
    int laneIndex;
    double positionX = 0;
	double positionY = 0;

    double speed;
    simtime_t time;
}


packet SeeThroughMessage
{
    string messageId;
	int vehicleIdInFlow;

    double messageSize;
	int interface;

	int partId;
	double partSize;

	int sendingInterface; 	// 0 = ITS-G5
							// 1 = LTE-V2X
							// 3 = Both

	string edgeName;
    int laneIndex;
    double positionX = 0;
	double positionY = 0;


    double speed;
    simtime_t emissionTime;
}
