reg maxBlockSize;
reg size;function prepareToPlay(sampleRate, blockSize)
{
	maxBlockSize = blockSize;
	cabBuffer.reserve(sampleRate * 30 / blockSize); // reserve 30sec to prevent more allocation that we already have in the audio thread (Buffer.create())
}
 function processBlock(channels)
{
	if (cabRecord)
	{		
		size = channels[0].length;
		
		cabBuffer.push([Buffer.create(size), Buffer.create(size)]);
		
		channels[0] >> cabBuffer[cabBuffer.length-1][0];
        channels[1] >> cabBuffer[cabBuffer.length-1][1];
        if (cabBuffer.length > 1000) { Console.print("Maximum buffer length reached, clearing."); cabBuffer.clear(); }
        
        for (ch in channels) { ch *= 0; } // mute output to avoid clicking
	}	
}
 function onControl(number, value)
{
	
}
 