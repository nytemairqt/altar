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

include("Boilerplate/utilityFunctions.js");

// Knobs

inline function onknbInputGainControl(component, value)
{
    inputGain.setAttribute(inputGain.Gain, value);
}

inline function onknbOutputGainControl(component, value)
{
    outputGain.setAttribute(outputGain.Gain, value);
}

inline function onknbGateThresholdControl(component, value)
{
    gate.setAttribute(gate.GateThreshold, value);
}

inline function onknbCleanInputControl(component, value)
{
    ampFixed.setAttribute(ampFixed.inputGainClean, value);
}

inline function onknbCleanOutputControl(component, value)
{
    ampFixed.setAttribute(ampFixed.outputGainClean, value);
}

inline function onknbDirtyInputControl(component, value)
{
    ampFixed.setAttribute(ampFixed.inputGainDirty, value);
}

inline function onknbDirtyOutputControl(component, value)
{
    ampFixed.setAttribute(ampFixed.outputGainDirty, value);
}

inline function onknbPitchControl(component, value)
{
    if (value == 0)
        pitchShifterFixed.setBypassed(true);
    else
        pitchShifterFixed.setBypassed(false);                       
    local newPitch = Math.pow(2.0, value / 12.0);               
    pitchShifterFixed.setAttribute(pitchShifterFixed.FreqRatio, newPitch);
}

inline function onknbCabAxisControl(component, value)
{
    local low = 0 * cabAxis.BandOffset + cabAxis.Gain;  
    local high = 1 * cabAxis.BandOffset + cabAxis.Gain; 
    local scaledValue = -4.0 + (8.0 * value);                
    cabAxis.setAttribute(low, 1-scaledValue); 
    cabAxis.setAttribute(high, scaledValue); 
}

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

inline function onknbEQWhistleControl(component, value)
{
    local A = 0 * eqWhistle.BandOffset + eqWhistle.Gain;    
    local B = 1 * eqWhistle.BandOffset + eqWhistle.Gain;    
    local scaledA = -0.0 - (5.0 * value);
    local scaledB = -0.0 - (8.0 * value);    
    eqWhistle.setAttribute(A, scaledA); 
    eqWhistle.setAttribute(B, scaledB); 
};

inline function onknbReverbMixControl(component, value)
{
    reverbFixed.setAttribute(reverbFixed.Mix, value);
}

inline function onknbReverbBrightnessControl(component, value)
{
    reverbFixed.setAttribute(reverbFixed.Brightness, value);
}

inline function onknbReverbFeedbackControl(component, value)
{
    reverbFixed.setAttribute(reverbFixed.Feedback, value);
}

knbInputGain.setControlCallback(onknbInputGainControl);
knbOutputGain.setControlCallback(onknbOutputGainControl);
knbGateThreshold.setControlCallback(onknbGateThresholdControl);

knbCleanInput.setControlCallback(onknbCleanInputControl);
knbCleanOutput.setControlCallback(onknbCleanOutputControl);
knbDirtyInput.setControlCallback(onknbDirtyInputControl);
knbDirtyOutput.setControlCallback(onknbDirtyOutputControl);

knbPitch.setControlCallback(onknbPitchControl);
knbCabAxis.setControlCallback(onknbCabAxisControl);
knbCabPresence.setControlCallback(onknbCabPresenceControl);
knbEQWhistle.setControlCallback(onknbEQWhistleControl);

knbReverbMix.setControlCallback(onknbReverbMixControl);
knbReverbBrightness.setControlCallback(onknbReverbBrightnessControl);
knbReverbFeedback.setControlCallback(onknbReverbFeedbackControl);

// Buttons

inline function onbtnAmpModeControl(component, value)
{
    ampFixed.setAttribute(ampFixed.channel, value);
}

inline function onbtnAmpOversamplingControl(component, value)
{
    ampFixed.setAttribute(ampFixed.oversampling, value);
}

inline function onbtnShowCabDesignerControl(component, value)
{
    pnlCabDesigner.set("visible", value);
};

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

    addNote(eventList, 64, 0, 1.0);
    
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



btnAmpMode.setControlCallback(onbtnAmpModeControl);
btnAmpOversampling.setControlCallback(onbtnAmpOversamplingControl);
btnCabGenerate.setControlCallback(onbtnCabGenerateControl);
btnCabSave.setControlCallback(onbtnCabSaveControl);
btnOpenCabFolder.setControlCallback(onbtnOpenCabFolderControl);
btnShowCabDesigner.setControlCallback(onbtnShowCabDesignerControl);


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