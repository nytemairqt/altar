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

include("Boilerplate/scriptReferences.js");

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

inline function saveModuleBypassedState()
{
    moduleBypassedStates.clear();
    for (m in modules)
        moduleBypassedStates.push(m.isBypassed());
}

inline function restoreModuleBypassedState()
{
    for (i=0; i<moduleBypassedStates.length; i++)
        modules[i].setBypassed(moduleBypassedStates[i]);
}

inline function renderAudioCallback (obj) 
{
    if (obj.finished) 
    {   
        // get the buffer                               
        local buffer = normalizeBuffer(obj.channels);                       
        local file = audioFiles.getChildFile(cabSaveName);

        // Force Stereo (HISE Convolution is stereo for some reason)
        file.writeAudioFile([buffer[0], buffer[0]], Engine.getSampleRate(), 24);

        restoreModuleBypassedState();
        cabEQMain.setBypassed(1);
        cabEQDetails.setBypassed(1);
        cabFileSave.setBypassed(1);     
        testAudio.setBypassed(0); // force enable audio player  
        cabEQCustom.restoreState(cabEQCustomBlankState); // reset the custom EQ 
        cabConvolution.setFile("");     
        cabConvolution.setFile(file.toString(0));
        cabConvolution.setBypassed(0);              
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

inline function sanitizeFileName(fileName)
{
    local strings = ["/", "!", "@", "#", "$", "%", "^", "&", "*", "(", ")", "|", ",", ";", ":", "{", "}", "[", "]", "<", ">", "?", "~", "`", "?", " "];
    
    for (s in strings)
        if (fileName.contains(s))
            return "INVALID_FILENAME";

    local sanitized = fileName;

    if (sanitized.contains(".") && !sanitized.contains(".wav"))
        return "INVALID_FILENAME";

    local stops = fileName.split(".").length > 2;

    if (stops)
        return "INVALID_FILENAME";

    if (!sanitized.contains(".wav"))
    {
        Console.print("appending .wav");
        sanitized = sanitized + ".wav";
    }
    else
    {
        if (sanitized.indexOf(".wav") != sanitized.length - 4)
            return "INVALID_FILENAME";
    }

    return sanitized;
}