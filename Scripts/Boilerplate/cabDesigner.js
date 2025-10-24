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
    const btnCabDesignerGenerate = Content.getComponent("btnCabDesignerGenerate");
    const cabDesigner = Synth.getEffect("cabDesigner");
    const fxSlots = [Synth.getSlotFX("modularA"), Synth.getSlotFX("modularB"), Synth.getSlotFX("modularC"), Synth.getSlotFX("modularD"), Synth.getSlotFX("modularE"), Synth.getSlotFX("modularF"), Synth.getSlotFX("modularG")];	            
    const cabDesignerSlot = Synth.getSlotFX("cabDesigner");
    const btnCloseCabDesigner = Content.getComponent("btnCloseCabDesigner");
    const cabDesignerEQ = Synth.getEffect("cabDesignerEQ");
    const fltCabDesignerEQ = Content.getComponent("fltCabDesignerEQ");        

    const btnCabSave = Content.getComponent("btnCabSave");
    const btnOpenCabFolder = Content.getComponent("btnOpenCabFolder");
    const cmbCabDesignerSpeaker = Content.getComponent("cmbCabDesignerSpeaker");
    const cmbCabDesignerMic = Content.getComponent("cmbCabDesignerMic");
    const knbCabDesignerMojo = Content.getComponent("knbCabDesignerMojo");
    const knbCabDesignerAge = Content.getComponent("knbCabDesignerAge");
    const lblCabSaveName = Content.getComponent("lblCabSaveName");   
    const pnlCabDesigner = Content.getComponent("pnlCabDesigner");

    var eventList = []; // used by cab apparently
    const impulseSize = 1024;
    const moduleBypassedStates = [];
    const audioFiles = FileSystem.getFolder(FileSystem.AudioFiles);
    reg cabSaveName = "myCab.wav";
    const modules = Synth.getAllEffects(".*");
    
    cabDesignerMIDIPlayer.create(4, 4, 1);

    inline function onbtnCloseCabDesignerControl(component, value)
    {
        if (value) { btnShowCabDesigner.setValue(0); btnShowCabDesigner.changed(); }
    }

    btnCloseCabDesigner.setControlCallback(onbtnCloseCabDesignerControl);	  

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

	// Show cab designer
    inline function onbtnShowCabDesignerControl(component, value)
    {                         
        local attribute = component.get("text");                
        for (slot in fxSlots)
            if (slot.getCurrentEffectId() == "cab")
            {
                local effect = slot.getCurrentEffect();
                local index = effect.getAttributeIndex(attribute);
                effect.setAttribute(index, value);                
            }
        cabDesigner.setBypassed(1-value);
        cabDesignerEQ.setBypassed(1-value);
        pnlCabDesigner.set("visible", value);  
    }

    btnShowCabDesigner.setControlCallback(onbtnShowCabDesignerControl);    

	// Select speaker
    inline function oncmbCabDesignerSpeakerControl(component, value)
    {
        cabDesigner.setAttribute(cabDesigner.SpeakerType, value-1);
    }

    cmbCabDesignerSpeaker.setControlCallback(oncmbCabDesignerSpeakerControl);    
    
    // Select microphone
    inline function oncmbCabDesignerMicControl(component, value)
    {
	    cabDesigner.setAttribute(cabDesigner.MicrophoneType, value-1);
    }
    
    cmbCabDesignerMic.setControlCallback(oncmbCabDesignerMicControl);

	// Save cab IR    

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

    // Look And Feel
    const pad = 8;
    const bounds = [pad, pad, 850 - pad * 2, 400 - pad * 2];
    const eqBounds = [fltCabDesignerEQ.get("x"), fltCabDesignerEQ.get("y"), fltCabDesignerEQ.get("width"), fltCabDesignerEQ.get("height")];

    pnlCabDesigner.loadImage("{PROJECT_FOLDER}bgCab.jpg", "bg");
    pnlCabDesigner.loadImage("{PROJECT_FOLDER}trim.png", "trim");
    pnlCabDesigner.setPaintRoutine(function(g)
    {                       
        g.drawImage("bg", bounds, 0, 0);
        g.drawImage("trim", bounds, 0, 0);
        g.setColour(ColourData.clrDarkgrey);
        g.drawRoundedRectangle(bounds, 0.0, 3.0);    
        
        g.setColour(ColourData.clrComponentBGGrey);
        g.fillRoundedRectangle(eqBounds, 0.0);       
        
        g.setColour(ColourData.clrMidgrey);
        g.drawRoundedRectangle(eqBounds, 0.0, 2.0);        
        
    }); 
    
    const cmbCabDesignerLAF = Content.createLocalLookAndFeel();
    	
   	cmbCabDesignerLAF.registerFunction("drawComboBox", function(g, obj)
    {	        	
	    // BG & Text
	    g.setColour(obj.hover ? ColourData.clrMidgrey : ColourData.clrDarkgrey);
	    g.fillRoundedRectangle(obj.area, 4.0);	    
	    g.setColour(ColourData.clrWhite);
	    g.drawAlignedText(obj.text, [8, 0, obj.area[2] - 16, obj.area[3]], "left");	    
	    
	    // Triangle
	    var tXPad = 24;
    	var tYPad = 12;
		var tX = obj.area[2] - tXPad;
		var tY = 8;
		var tW = 16;
		var tH = tY;	    
	    var tA = [tX, tYPad, tW, tH];
	    g.fillTriangle(tA, Math.toRadians(180));
    });
    
    cmbCabDesignerSpeaker.setLocalLookAndFeel(cmbCabDesignerLAF);
    cmbCabDesignerMic.setLocalLookAndFeel(cmbCabDesignerLAF);
    
    const btnCabDesignerSpeakerLAF = Content.createLocalLookAndFeel();
    	
    const pnlCabDesignerSpeakerIcon = Content.getComponent("pnlCabDesignerSpeakerIcon");
    
    pnlCabDesignerSpeakerIcon.setPaintRoutine(function(g)
    {
		var x = 0; var y = 0; var w = this.getWidth(); var h = this.getHeight();
		var area = [x, y, w, h];
		var pad = 1;		
		var line = 1.5;
		var p = Content.createPath();
		p.loadFromData(PathData.pathSpeaker);
		g.setColour(ColourData.clrLightgrey);	
	    
	    g.drawPath(p, [x + pad, y + pad, w - (2 * pad), h - (2 * pad)], line);
	    
	    
    });
    
    const pnlCabDesignerMicrophoneIcon = Content.getComponent("pnlCabDesignerMicrophoneIcon");

	pnlCabDesignerMicrophoneIcon.setPaintRoutine(function(g)
    {		
		var x = 0; var y = 0; var w = this.getWidth(); var h = this.getHeight();
		var area = [x, y, w, h];
		var line = 1.5;
		var baseW = w / 2; var baseH = Math.round(h * 0.4375);
		var baseX = w / 2 - Math.round(baseW / 2); var baseY = Math.round(h * 0.6875) - Math.round(baseH / 2); 		
		var topW = Math.round(w * 0.36); var topH = Math.round(h * 0.58);
		var topX = w / 2 - Math.round(topW / 2); var topY = Math.round(h * 0.36) - Math.round(topH / 2);
		var p = Content.createPath();		
		g.setColour(ColourData.clrLightgrey);
		
		// base
	    p.loadFromData(PathData.pathMicrophoneA);
	    g.drawPath(p, [baseX, baseY, baseW, baseH], line);
	    p.clear();
	    
	    // top
	    p.loadFromData(PathData.pathMicrophoneB);
	    g.drawPath(p, [topX, topY, topW, topH], line);
    });
    
    
}