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

namespace CabDesigner
{
	const cabDesignerFileSave = Synth.getAudioSampleProcessor("cabDesignerFileSave");
    const cabDesignerMIDIPlayer = Synth.getMidiPlayer("cabDesignerMIDIPlayer");
    const btnShowCabDesigner = Content.getComponent("btnShowCabDesigner");
    const btnCab = Content.getComponent("btnCab");
    const btnCabGenerate = Content.getComponent("btnCabGenerate");    
    const btnCabSave = Content.getComponent("btnCabSave");
    const btnOpenCabFolder = Content.getComponent("btnOpenCabFolder");
    const cmbCabDesignerSpeaker = Content.getComponent("cmbCabDesignerSpeaker");
    const cmbCabDesignerMic = Content.getComponent("cmbCabDesignerMic");
    const knbCabDesignerMojo = Content.getComponent("knbCabDesignerMojo");
    const knbCabDesignerAge = Content.getComponent("knbCabDesignerAge");
    const lblCabSaveName = Content.getComponent("lblCabSaveName");
    const fltCabDesignerEQ = Content.getComponent("fltCabDesignerEQ");
    const pnlCabDesigner = Content.getComponent("pnlCabDesigner");

    var eventList = []; // used by cab apparently
    const impulseSize = 1024;
    const moduleBypassedStates = [];
    const audioFiles = FileSystem.getFolder(FileSystem.AudioFiles);
    reg cabSaveName = "myCab.wav";
    const modules = Synth.getAllEffects(".*");
    
    cabDesignerMIDIPlayer.create(4, 4, 1);

    pnlCabDesigner.setPaintRoutine(function(g) // move to LAF later
    {
        var bounds = [145, 310, 860, 490];

        g.setColour(Colours.withAlpha(Colours.black, 1.0));
        g.fillRoundedRectangle(bounds, 2.0);
    });


    pnlCabDesigner.setMouseCallback(function(event)
    {
        var x = 145;
        var y = 310;
        var w = 860;
        var h = 490;
        
        if (event.mouseDownX < x || event.mouseDownX > (x + w) || event.mouseDownY < y || event.mouseDownY > (y + h)) 
        {
            btnShowCabDesigner.setValue(0);
            btnShowCabDesigner.changed();
        }   
    });

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

    inline function addNoteForCabSave(eventListToUse, noteNumber, startQuarter, durationQuarter)
    {
        /* Flushes the MIDI list for the impulse player */
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

    inline function normalizeBuffer(buffer)
    {
        /* Normalizes the dirac IR */
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

    inline function renderAudioCallback (obj) 
    {
        /* Handles file-writing for IR */
        if (obj.finished) 
        {   
            // get the buffer                               
            local buffer = normalizeBuffer(obj.channels);                       
            local file = audioFiles.getChildFile(cabSaveName);
            
            // FIX ME:
            // check if chosen file name is already loaded in either cab waveform
            // if it is, show alert window and return

            // HISE conv expects stereo
            file.writeAudioFile([buffer[0], buffer[0]], Engine.getSampleRate(), 24);

            restoreModuleBypassedState(); // DONT DELETE ME
            //cabDesignerSpeaker.setBypassed(1);
            //cabDesignerMojo.setBypassed(1);
            //cabDesignerMic.setBypassed(1);
            //cabDesignerEQ.setBypassed(1);
            //cabDesignerFileSave.setBypassed(1);             
            testAudio.setBypassed(0); // force enable audio player  
            //cabDesignerEQ.restoreState(dspNoProfile); // reset the custom EQ 
            
            // FIX ME:
            // refresh audio files list somehow so new cab shows up in list
        } 
    }

    

    inline function sanitizeFileName(fileName)
    {
        /* Ensures people aren't putting stupid characters in their IR names */
        local lbl = Content.getComponent("lblCabSaveName");
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
            sanitized = sanitized + ".wav";
            lbl.set("text", sanitized);
        }
        else
        {
            if (sanitized.indexOf(".wav") != sanitized.length - 4)
                return "INVALID_FILENAME";
        }

        return sanitized;
    }

    inline function onbtnShowCabDesignerControl(component, value)
    {
        btnCab.setValue(1-value);
        btnCab.changed();   
        pnlCabDesigner.set("visible", value);   
    }

    btnShowCabDesigner.setControlCallback(onbtnShowCabDesignerControl);

    inline function onbtnCabGenerateControl(component, value)
    {
        if (!value)
            return;

        local gain = 0.0;
        local q = 0.0;
        local freq = 0;
        local mojoGain = 0.0;

        Console.print("generated a cab!");

        // call a third-party node function to randomize cab        
    }

    btnCabGenerate.setControlCallback(onbtnCabGenerateControl);

    inline function onbtnCabSaveControl(component, value)
    {
        if (!value)
            return;
            
        saveModuleBypassedState();

        for (m in modules) { m.setBypassed(1); }                    

        testAudio.setBypassed(1); // force disable

        // force enable third party node here
                    
        cabDesignerFileSave.setFile(""); // clears audio buffer

        local buffer = Buffer.create(impulseSize);      
        buffer[0] = 1.0; // Dirac delta

        // get temp
        local tempFiles = FileSystem.getFolder(FileSystem.Temp);
        local tempFile = tempFiles.getChildFile("tempImpulse.wav");
        
        tempFile.writeAudioFile(buffer, Engine.getSampleRate(), 24);  
        cabDesignerFileSave.setFile(tempFile.toString(0));  

        eventList.clear();
        addNoteForCabSave(eventList, 64, 0, 1.0);
        
        // swap workaround bullshit
        cabDesignerMIDIPlayer.setUseTimestampInTicks(true); 
        cabDesignerMIDIPlayer.flushMessageList(eventList);
        cabDesignerMIDIPlayer.setUseTimestampInTicks(false);
            
        // Check sequencer is loaded
        if (cabDesignerMIDIPlayer.isEmpty() != true) 
            Engine.renderAudio(cabDesignerMIDIPlayer.getEventList(), renderAudioCallback);  
    };

    btnCabSave.setControlCallback(onbtnCabSaveControl);

    inline function onbtnOpenCabFolderControl(component, value)
    {
        if (!value)
            return;
        audioFiles.show();
    }

    btnOpenCabFolder.setControlCallback(onbtnOpenCabFolderControl);

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
}