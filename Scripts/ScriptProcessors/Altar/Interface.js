Content.makeFrontInterface(600, 600);

include("Boilerplate/scriptReferences.js");

const audiofiles = Engine.loadAudioFilesIntoPool();



inline function onknbPitchControl(component, value)
{
	// bypass if semitones = 0	
	if (value == 0)
		pitchShifterFixed.setBypassed(true);
	else
		pitchShifterFixed.setBypassed(false);		
	
	local newPitch = Math.pow(2.0, value / 12.0);				
	pitchShifterFixed.setAttribute(pitchShifterFixed.FreqRatio, newPitch);
};

Content.getComponent("knbPitch").setControlCallback(onknbPitchControl);

var gain = 0;
var q = 0;
var freq = 0;

inline function onbtnCabGenerateControl(component, value)
{		
	if (value)
	{
		local gain = 0;
		local q = 0;
		local freq = 0;

		// Main
		for (i = 1; i<21; i++)
        {
			gain = i * cabEQMain.BandOffset + cabEQMain.Gain;	
			q = i * cabEQMain.BandOffset + cabEQMain.Q;
			freq = i * cabEQMain.BandOffset + cabEQMain.Freq;

	        if (i < 3) // Low End Broad Curves
	        {
		    	cabEQMain.setAttribute(gain, Math.randInt(-1, 1) + Math.random());  		    			    	
		    	cabEQMain.setAttribute(q, Math.randInt(1, 2) + Math.random());     
		    	cabEQMain.setAttribute(freq, Math.randInt(60, 150));  
	        }                             
	        if (i >=3 && i < 6) // Low Mud
	        {
		        cabEQMain.setAttribute(gain, Math.randInt(-6, -1) + Math.random());  		    			    	
		        cabEQMain.setAttribute(q, Math.randInt(3, 4) + Math.random());  
		        cabEQMain.setAttribute(freq, Math.randInt(200, 500));     
	        }
	        if (i >= 6 && i < 11) // Mids
	        {
		        cabEQMain.setAttribute(gain, Math.randInt(-3, -1) + Math.random());  		    			    	
   		        cabEQMain.setAttribute(q, Math.randInt(3, 4) + Math.random());  
   		        cabEQMain.setAttribute(freq, Math.randInt(600, 1800));   
	        }
	        if (i >= 11 && i < 15) // High Mids
	        {
		        cabEQMain.setAttribute(gain, Math.randInt(-4, -4) + Math.random());  		    			    						
		        cabEQMain.setAttribute(q, Math.randInt(2, 4) + Math.random());     
		        cabEQMain.setAttribute(freq, Math.randInt(2000, 6000));  
	        }
	        if (i >= 15 && i < 21) // Fizzy Top
	        {
		        cabEQMain.setAttribute(gain, Math.randInt(-6, -1) + Math.random());  		    			    						
		        cabEQMain.setAttribute(q, Math.randInt(1, 3) + Math.random()); 
		        cabEQMain.setAttribute(freq, Math.randInt(6000, 9000));      
	        }
        }
        
        // Details
        for (i = 0; i<60; i++)
        {   
            //Gains
            gain = i * cabEQDetails.BandOffset + cabEQDetails.Gain;
            cabEQDetails.setAttribute(gain, Math.randInt(-4, 4) + Math.random()); 
            
            //Q's

            q = i * cabEQDetails.BandOffset + cabEQDetails.Q;
            cabEQDetails.setAttribute(q, Math.randInt(6, 8) + Math.random()); 
            
            //Frequencies 
            
            freq = i * cabEQDetails.BandOffset + cabEQDetails.Freq;
            cabEQDetails.setAttribute(freq, Math.randInt(200, 8000));             
        }      
	}
};

Content.getComponent("btnCabGenerate").setControlCallback(onbtnCabGenerateControl);

inline function onknbCabAxisControl(component, value)
{		
	local low = 0 * cabAxis.BandOffset + cabAxis.Gain;	
	local high = 1 * cabAxis.BandOffset + cabAxis.Gain;	
	local scaledValue = -4.0 + (8.0 * value);
	
	cabAxis.setAttribute(low, 1-scaledValue); 
	cabAxis.setAttribute(high, scaledValue); 
};

Content.getComponent("knbCabAxis").setControlCallback(onknbCabAxisControl);

// fizzy


inline function onknbCabPresenceControl(component, value)
{
	local fizz = 0;
	local scaledValue = 5500.0 + (6500.0 * value);

	for (i=3; i<6; i++)
	{
		fizz = i * cabAxis.BandOffset + cabAxis.Freq;					
		cabAxis.setAttribute(fizz, scaledValue);
	}		
};

Content.getComponent("knbCabPresence").setControlCallback(onknbCabPresenceControl);

inline function onknbEQWhistleControl(component, value)
{
	local A = 0 * eqWhistle.BandOffset + eqWhistle.Gain;	
	local B = 1 * eqWhistle.BandOffset + eqWhistle.Gain;	
	local scaledA = -0.0 - (5.0 * value);
	local scaledB = -0.0 - (8.0 * value);
	
	eqWhistle.setAttribute(A, scaledA); 
	eqWhistle.setAttribute(B, scaledB); 
};

Content.getComponent("knbEQWhistle").setControlCallback(onknbEQWhistleControl);

// SAVE IR


var eventList = [];
cabMIDIPlayer.create(4, 4, 1);
const impulseSize = 1024;

const audioFiles = FileSystem.getFolder(FileSystem.AudioFiles);
Engine.loadAudioFilesIntoPool();

/** Adds a note to the event list. */
inline function addNote(eventListToUse, noteNumber, startQuarter, durationQuarter)
{
	// needs both a noteOn and noteOff
	// clear list first
	eventListToUse.clear();
	
	// noteOn
	local on = Engine.createMessageHolder();
	on.setType(1);			   
	on.setNoteNumber(noteNumber);  
	on.setChannel(1);		   
	on.setVelocity(127);	   
	on.setTimestamp(0);
	eventListToUse.push(on);
	
	// noteOff
	local off = Engine.createMessageHolder();
	off.setType(2);	
	off.setNoteNumber(noteNumber);  
	off.setChannel(1);		   
	off.setTimestamp(impulseSize);	
	eventListToUse.push(off);
}


inline function onbtnCabSaveControl(component, value)
{
	if (!value)
		return;
		
	// bypass unwanted DSP 
	gate.setBypassed(1);
	pitchShifterFixed.setBypassed(1);
	preSculpt.setBypassed(1);
	ampFixed.setBypassed(1);
	postSculpt.setBypassed(1);
	cabConvolution.setBypassed(1);
	cabAxis.setBypassed(1);
	eqWhistle.setBypassed(1);
	reverbFixed.setBypassed(1);
	
	// enable filters
	
	cabEQMain.setBypassed(0);
	cabEQDetails.setBypassed(0);

	/*
	
	need to implement dynamic fx Bypass, then re-enable when the renderer finishes
	also possibly freeze the interface while rendering (although it's basically instant) 



	*/
		
	cabFileSave.setFile(""); // clears audio buffer

	local buffer = Buffer.create(impulseSize);		
	buffer[0] = 1.0; // Dirac delta

	// get temp
	local tempFiles = FileSystem.getFolder(FileSystem.Temp);
	local tempFile = tempFiles.getChildFile("tempImpulse.wav");
	
	tempFile.writeAudioFile(buffer, Engine.getSampleRate(), 24);
	
	cabFileSave.setFile(tempFile.toString(0));	

	addNote(eventList, 64, 0, 1.0);
	
	// Swap workaround bullshit
	cabMIDIPlayer.setUseTimestampInTicks(true);	
	cabMIDIPlayer.flushMessageList(eventList);
	cabMIDIPlayer.setUseTimestampInTicks(false);
		
	// Check sequencer is loaded
	if (cabMIDIPlayer.isEmpty() != true) 
		Engine.renderAudio(cabMIDIPlayer.getEventList(), COMPOSER_RenderAudioCallback);	
};

inline function COMPOSER_RenderAudioCallback (obj) 
{
	if (obj.finished) 
	{	
		// get the buffer								
		local buffer = normalizeBuffer(obj.channels);						
		local file = audioFiles.getChildFile("myCoolImpulse.wav");

		// Force Stereo (HISE Convolution is stereo for some reason)
		file.writeAudioFile([buffer[0], buffer[0]], Engine.getSampleRate(), 24);
		
		// bypass unwanted DSP 
		gate.setBypassed(0);
		pitchShifterFixed.setBypassed(0);
		preSculpt.setBypassed(0);
		ampFixed.setBypassed(0);
		postSculpt.setBypassed(0);
		cabConvolution.setBypassed(0);
		cabAxis.setBypassed(0);
		eqWhistle.setBypassed(0);
		reverbFixed.setBypassed(0);
		
		// enable filters
		
		cabEQMain.setBypassed(1);
		cabEQDetails.setBypassed(1);
		
		cabConvolution.setFile(file.toString(0));
		
		// Remove this line later but it will point you to the temp file that has been created.
		file.show();

		//cabFileSave.setFile(file.toString(0));				// this will be replaced with our main convolution later

		// need to refresh audio files so it shows up under cab or whatever
		// need a checkbox to open explorer @ rendered file
		
	} 
}

inline function normalizeBuffer(buffer)
{
	local bufferNormalized = buffer.clone();

	local peak = 0.0;	
	local v = -1.0;
	
    for (i = 0; i < impulseSize; i++)
    {
        v = Math.abs(bufferNormalized[0][i]);       
        if (v > peak)
            peak = v;
    }

	if (peak > 0.0)
	{
	    local scale = 1.0 / peak;	
        for (i = 0; i < impulseSize; i++)           
            bufferNormalized[0][i] *= scale;            
	}	

	return bufferNormalized;
}


Content.getComponent("btnCabSave").setControlCallback(onbtnCabSaveControl);

function onNoteOn()
{
	
}
 function onNoteOff()
{
	
}
 function onController()
{
	
}
 function onTimer()
{
	
}
 function onControl(number, value)
{
	
}
 