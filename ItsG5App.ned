import artery.application.ItsG5Service;


simple ItsG5App like ItsG5Service
{
	parameters:
		
		@signal[LteSignal];
		@signal[itsG5ToSubAppSignal];

}
