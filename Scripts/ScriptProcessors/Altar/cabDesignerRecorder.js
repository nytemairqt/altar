reg maxBlockSize;function prepareToPlay(sampleRate, blockSize)
{
	maxBlockSize = blockSize;
	cabBuffer.reserve(sampleRate * 30 / blockSize); // reserve 30sec to prevent more allocation that we already have in the audio thread (Buffer.create())
}
 function processBlock(channels)
{
	if (cabRecord)
	{
		// append a new empty buffer at the end of the cabBuffer
		cabBuffer.push([Buffer.create(maxBlockSize), Buffer.create(maxBlockSize)]);
		
		// copy channels
		channels[0] >> cabBuffer[cabBuffer.length-1][0];
		channels[1] >> cabBuffer[cabBuffer.length-1][1];
		
		// mute output audio to avoid clicks
		
		// safety to prevent memory leak
		if (cabBuffer.length > 1500) { Console.print("Maximum buffer length reached, clearing."); cabBuffer.clear(); }
			
		//Console.print(cabBuffer.length);
	}
	/*
	if (cabSave)
	{
		for (ch in channels) { ch *= 0; }			
	}
	*/
}
 function onControl(number, value)
{
	
}
 