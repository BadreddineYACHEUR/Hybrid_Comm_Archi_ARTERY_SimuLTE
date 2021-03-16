packet PlatooningMessage
{
	string vehicleId;    
    string edgeName;
    int laneIndex;

    double speed;
    simtime_t time;
}


packet HeavyMessage
{
    int id;
    double size;

    simtime_t time;
}
