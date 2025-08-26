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
    pitchShifterFixed.setAttribute(pitchShifterFixed.FreqRatio, newPitch);
}

inline function onknbEQWhistleControl(component, value)
{
    local A = 0 * eqWhistle.BandOffset + eqWhistle.Gain;    
    local B = 1 * eqWhistle.BandOffset + eqWhistle.Gain;    
    local scaledA = -0.0 - (5.0 * value);
    local scaledB = -0.0 - (8.0 * value);    
    eqWhistle.setAttribute(A, scaledA); 
    eqWhistle.setAttribute(B, scaledB); 
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
	chugFixed.setAttribute(chugFixed.Threshold, 1-value);
}

knbPitch.setControlCallback(onknbPitchControl);
knbEQWhistle.setControlCallback(onknbEQWhistleControl);
knbLofiLow.setControlCallback(onknbLofiControl);
knbLofiHigh.setControlCallback(onknbLofiControl);
knbOctave.setControlCallback(onknbOctaveControl);
knbOctaveFreq.setControlCallback(onknbOctaveControl);
knbChugThreshold.setControlCallback(onknbChugControl);

// Buttons

inline function onbtnBypass(component, value)
{
	switch (component)
	{
		case btnTunerMonitor:
			tuner.setAttribute(tuner.Monitor, 1-value);
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
			ampFixed.setAttribute(ampFixed.channel, value);
			pnlAmpClean.set("visible", 1-value);
			pnlAmpDirty.set("visible", value);
			break;
		case btnCabAEnable:
			cabFixed.setAttribute(cabFixed.CabAEnable, value);
			cabFixed.setAttribute(cabFixed.CabAClear, 1-value);
			break;
		case btnCabBEnable:
			cabFixed.setAttribute(cabFixed.CabBEnable, value);
			cabFixed.setAttribute(cabFixed.CabBClear, 1-value);
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
		case btnShowCabDesigner:
			pnlCabDesigner.set("visible", value);
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

inline function onbtnCabGenerateControl(component, value)
{
    if (!value)
        return;

    local gain = 0.0;
    local q = 0.0;
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

inline function onbtnCabSaveControl(component, value)
{
    if (!value)
        return;
        
    saveModuleBypassedState();

    for (m in modules)
        m.setBypassed(1);

    testAudio.setBypassed(1); // force disable

    // force enable filters
    
    cabEQMain.setBypassed(0);
    cabEQDetails.setBypassed(0);
    cabEQCustom.setBypassed(0);
    cabFileSave.setBypassed(0);
        
    cabFileSave.setFile(""); // clears audio buffer

    local buffer = Buffer.create(impulseSize);      
    buffer[0] = 1.0; // Dirac delta

    // get temp
    local tempFiles = FileSystem.getFolder(FileSystem.Temp);
    local tempFile = tempFiles.getChildFile("tempImpulse.wav");
    
    tempFile.writeAudioFile(buffer, Engine.getSampleRate(), 24);
    
    cabConvolution.setFile(tempFile.toString(0)); // make sure the cab doesn't have the previous saved impulse so we can safely overwrite
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

// Cab Select
btnCabALoadPrev.setControlCallback(onbtnCabSelectControl);
btnCabALoadNext.setControlCallback(onbtnCabSelectControl);

// More Complex Functions
btnCabGenerate.setControlCallback(onbtnCabGenerateControl);
btnCabSave.setControlCallback(onbtnCabSaveControl);
btnOpenCabFolder.setControlCallback(onbtnOpenCabFolderControl);

// Show / Hide Panels
btnShowOverdrive.setControlCallback(showPanelControl);
btnShowAmp.setControlCallback(showPanelControl);
btnShowCab.setControlCallback(showPanelControl);
btnShowCabDesigner.setControlCallback(showPanelControl);
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