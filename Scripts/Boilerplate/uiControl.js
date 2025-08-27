/*
    Copyright 2025 iamlamprey

    This file is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This file is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with This file. If not, see <http://www.gnu.org/licenses/>.
*/

include("Boilerplate/pitchDetection.js");

// Knobs

inline function onknbPitchControl(component, value)
{                    
    local newPitch = Math.pow(2.0, value / 12.0);               
    pitch.setAttribute(pitch.FreqRatio, newPitch);
}



inline function onknbCabDesignerMojoControl(component, value)
{
	/*

	local gain = 0.0;
	
	// Details
	for (i = 0; i<60; i++)
	{   
	    //Gains
	    gain = i * cabDesignerMojo.BandOffset + cabDesignerMojo.Gain;
	    cabDesignerMojo.setAttribute(gain, gain * knbCabDesignerMojo.getValue());                                 
	}  
	*/
};

inline function onknbEQWhistleControl(component, value)
{
    local A = 0 * whistle.BandOffset + whistle.Gain;    
    local B = 1 * whistle.BandOffset + whistle.Gain;    
    local scaledA = -0.0 - (5.0 * value);
    local scaledB = -0.0 - (8.0 * value);    
    whistle.setAttribute(A, scaledA); 
    whistle.setAttribute(B, scaledB); 
};

inline function onknbLofiControl(component, value)
{
	switch (component)
	{
		case knbLofiLow:
			local low = 0 * lofi.BandOffset + lofi.Freq;
			lofi.setAttribute(low, value);
			break;
		case knbLofiHigh:
			local high = 1 * lofi.BandOffset + lofi.Freq;
			lofi.setAttribute(high, value);
			break;
	}
}

inline function onknbOctaveControl(component, value)
{
	switch (component)
	{
		case knbOctave:
			octavePre.setAttribute(octavePre.Mix, value);
			octavePost.setAttribute(octavePost.Mix, value);
			break;		
		case knbOctaveFreq:
			octavePre.setAttribute(octavePre.Freq, value);
			octavePost.setAttribute(octavePost.Freq, value);
		break;
	}
}

inline function onknbChugControl(component, value)
{
	chug.setAttribute(chug.Threshold, 1-value);
}

knbPitch.setControlCallback(onknbPitchControl);
knbEQWhistle.setControlCallback(onknbEQWhistleControl);
knbLofiLow.setControlCallback(onknbLofiControl);
knbLofiHigh.setControlCallback(onknbLofiControl);
knbOctave.setControlCallback(onknbOctaveControl);
knbOctaveFreq.setControlCallback(onknbOctaveControl);
knbChugThreshold.setControlCallback(onknbChugControl);
knbCabDesignerMojo.setControlCallback(onknbCabDesignerMojoControl);

// Buttons

inline function onbtnBypass(component, value)
{
	switch (component)
	{
		case btnTunerMonitor:
			tuner.setAttribute(tuner.Monitor, 1-value);
			break;
		case btnOversampling:
			amp.setAttribute(amp.Oversampling, value);
			grit.setAttribute(grit.Oversampling, value);
			break;
		case btnClick:
			click.setBypassed(1-value);
			if (value)
			{
                setupClick();
				clickMIDI.play(0);
			}
			else
				clickMIDI.stop(0);
			break;
		case btnOctave:
			if (value)
			{
				octavePre.setBypassed(btnOctavePosition.getValue());
				octavePost.setBypassed(1-btnOctavePosition.getValue());
				btnOctavePosition.set("enabled", true);
			}
			else
			{
				octavePre.setBypassed(true);
				octavePost.setBypassed(true);
				btnOctavePosition.set("enabled", false);
			}
			break;
		case btnOctavePosition:
			btnOctave.changed();
			break;
		case btnAmpMode:
			amp.setAttribute(amp.Channel, value);
			pnlAmpClean.set("visible", 1-value);
			pnlAmpDirty.set("visible", value);
			break;
		case btnCabAEnable:
			cab.setAttribute(cab.CabAEnable, value);
			cab.setAttribute(cab.CabAClear, 1-value);
			break;
		case btnCabBEnable:
			cab.setAttribute(cab.CabBEnable, value);
			cab.setAttribute(cab.CabBClear, 1-value);
			break;
		case btnPitchSnap:
			knbPitch.set("stepSize", value ? 1.0 : 0.01);
			break;
	}
}

inline function showPanelControl(component, value)
{
	switch (component)
	{
		case btnShowOverdrive:
			pnlOverdrive.set("visible", value);
			break;
		case btnShowAmp:
			pnlAmp.set("visible", value);
			break;
		case btnShowCab:
			pnlCab.set("visible", value);
			break;
		case btnShowReverb:
			pnlReverb.set("visible", value);
			break;
		case btnShowDelay:
			pnlDelay.set("visible", value);
			break;
		case btnShowChorus:
			pnlChorus.set("visible", value);
			break;
		case btnShowRingMod:
			pnlRingMod.set("visible", value);
			break;	
		case btnShowTuner:
			pnlTuner.set("visible", value);			
			break;	
	}
}

inline function onbtnCabSelectControl(component, value)
{
	if (!value)
		return;

	switch (component)
	{
		case btnCabALoadPrev:
			// get current loaded file as index within audioFiles
			// load the prev/next file
			break;
		case btnCabALoadNext:
			break;
	}
}

inline function oncmbCabGenerateControl(component, value)
{
	switch (component)
	{
		case cmbCabDesignerSpeaker:
		switch (value)
		{
			case 1:
				cabDesignerSpeaker.restoreState(dspCabA);
				break;
			case 2:
				cabDesignerSpeaker.restoreState(dspCabB);
				break;
			case 3:
				cabDesignerSpeaker.restoreState(dspCabC);
				break;
			case 4:
				cabDesignerSpeaker.restoreState(dspCabD);
				break;
			case 5:
				cabDesignerSpeaker.restoreState(dspCabE);
				break;
			case 6:
				cabDesignerSpeaker.restoreState(dspCabF);
				break;	
		}
		break;		
		case cmbCabDesignerMic:
		switch (value)
		{
			case 1:
				cabDesignerMic.restoreState(dspNoProfile);
				break;
			case 2:
				cabDesignerMic.restoreState(dspMicA);
				break;
			case 3:
				cabDesignerMic.restoreState(dspMicB);
				break;
			case 4:
				cabDesignerMic.restoreState(dspMicC);
				break;
			case 5:
				cabDesignerMic.restoreState(dspMicD);
				break;
			case 6:
				cabDesignerMic.restoreState(dspMicE);
				break;				
		}
		break;
	}
}

inline function onbtnShowCabDesignerControl(component, value)
{
	btnCab.setValue(1-value);
	btnCab.changed();	
	pnlCabDesigner.set("visible", value);
	cabDesignerSpeaker.setBypassed(1-value);
	cabDesignerMojo.setBypassed(1-value);
	cabDesignerMic.setBypassed(1-value);
	cabDesignerEQ.setBypassed(1-value);
}

inline function onbtnCabGenerateControl(component, value)
{
    if (!value)
        return;

    local gain = 0.0;
    local q = 0.0;
    local freq = 0;
    local mojoGain = 0.0;

    // Details
    for (i = 0; i<60; i++)
    {   
    	mojoGain = (Math.randInt(-2, 2) + Math.random()) * knbCabDesignerMojo.getValue();
    	Console.print(mojoGain);
    	
        //Gains
        gain = i * cabDesignerMojo.BandOffset + cabDesignerMojo.Gain;
        cabDesignerMojo.setAttribute(gain, mojoGain);                     
        //Q's
        q = i * cabDesignerMojo.BandOffset + cabDesignerMojo.Q;
        cabDesignerMojo.setAttribute(q, Math.randInt(6, 8) + Math.random());                     
        //Frequencies                     
        freq = i * cabDesignerMojo.BandOffset + cabDesignerMojo.Freq;
        cabDesignerMojo.setAttribute(freq, Math.randInt(200, 8000));             
    }      
}

inline function onbtnCabSaveControl(component, value)
{
    if (!value)
        return;
        
    saveModuleBypassedState();

    for (m in modules)
        m.setBypassed(1);

    testAudio.setBypassed(1); // force disable

    // enable filters    
    cabDesignerSpeaker.setBypassed(0);
    cabDesignerMojo.setBypassed(0);
    cabDesignerMic.setBypassed(0);
    cabDesignerEQ.setBypassed(0);
    cabDesignerFileSave.setBypassed(0);
        
    cabDesignerFileSave.setFile(""); // clears audio buffer

    local buffer = Buffer.create(impulseSize);      
    buffer[0] = 1.0; // Dirac delta

    // get temp
    local tempFiles = FileSystem.getFolder(FileSystem.Temp);
    local tempFile = tempFiles.getChildFile("tempImpulse.wav");
    
    tempFile.writeAudioFile(buffer, Engine.getSampleRate(), 24);  
    cabFileSave.setFile(tempFile.toString(0));  

    eventList.clear();
    addNoteForCabSave(eventList, 64, 0, 1.0);
    
    // Swap workaround bullshit
    cabMIDIPlayer.setUseTimestampInTicks(true); 
    cabMIDIPlayer.flushMessageList(eventList);
    cabMIDIPlayer.setUseTimestampInTicks(false);
        
    // Check sequencer is loaded
    if (cabMIDIPlayer.isEmpty() != true) 
        Engine.renderAudio(cabMIDIPlayer.getEventList(), renderAudioCallback);  
};

inline function onbtnOpenCabFolderControl(component, value)
{
    if (!value)
        return;
    audioFiles.show();
}


// Basic Toggles
btnTunerMonitor.setControlCallback(onbtnBypass);
btnClick.setControlCallback(onbtnBypass);
btnOctave.setControlCallback(onbtnBypass);
btnOctavePosition.setControlCallback(onbtnBypass);
btnAmpMode.setControlCallback(onbtnBypass);
btnCabAEnable.setControlCallback(onbtnBypass);
btnCabBEnable.setControlCallback(onbtnBypass);
btnOversampling.setControlCallback(onbtnBypass);
btnPitchSnap.setControlCallback(onbtnBypass);

// Cab
btnCabALoadPrev.setControlCallback(onbtnCabSelectControl);
btnCabALoadNext.setControlCallback(onbtnCabSelectControl);
cmbCabDesignerSpeaker.setControlCallback(oncmbCabGenerateControl);
cmbCabDesignerMic.setControlCallback(oncmbCabGenerateControl);
btnShowCabDesigner.setControlCallback(onbtnShowCabDesignerControl);

// More Complex Functions
btnCabGenerate.setControlCallback(onbtnCabGenerateControl);
btnCabSave.setControlCallback(onbtnCabSaveControl);
btnOpenCabFolder.setControlCallback(onbtnOpenCabFolderControl);

// Show / Hide Panels
btnShowOverdrive.setControlCallback(showPanelControl);
btnShowAmp.setControlCallback(showPanelControl);
btnShowCab.setControlCallback(showPanelControl);
btnShowReverb.setControlCallback(showPanelControl);
btnShowDelay.setControlCallback(showPanelControl);
btnShowChorus.setControlCallback(showPanelControl);
btnShowRingMod.setControlCallback(showPanelControl);
btnShowTuner.setControlCallback(showPanelControl);




// Labels

inline function onlblCabSaveNameControl(component, value)
{
    cabSaveName = sanitizeFileName(value);

    if (cabSaveName == "INVALID_FILENAME")
    {
        Engine.showMessage("Invalid character in filename. File name must not include special characters, and end in .wav"); // only in compiled plugin
        Console.print("Invalid character in filename. File name must not include special characters, and end in .wav"); // debug
        lblCabSaveName.set("text", "myCab.wav");
        cabSaveName = "myCab.wav";
    }
}

lblCabSaveName.setControlCallback(onlblCabSaveNameControl);