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
    const cabDesigner = Synth.getEffect("cabDesigner");
    const cabDesignerEQ = Synth.getEffect("cabDesignerEQ");
    const fxSlots = [Synth.getSlotFX("modularA"), Synth.getSlotFX("modularB"), Synth.getSlotFX("modularC"), Synth.getSlotFX("modularD"), Synth.getSlotFX("modularE"), Synth.getSlotFX("modularF"), Synth.getSlotFX("modularG")];                
	const cabFileSave = Synth.getAudioSampleProcessor("cabFileSave");
    const cabDesignerMIDIPlayer = Synth.getMidiPlayer("cabDesignerMIDIPlayer");
    const testAudio = Synth.getChildSynth("testAudio");                    
    const dp = Synth.getDisplayBufferSource("cabDesigner");
    const fft = dp.getDisplayBuffer(0);
    const inputGain = Synth.getEffect("inputGain");
    const outputGain = Synth.getEffect("outputGain");
    const preprocess = Synth.getEffect("preprocess");
    const postprocess = Synth.getEffect("postprocess");
    
    fft.setRingBufferProperties({
		  "BufferLength": 2048,
		  "WindowType": "Blackman Harris",
		  "NumChannels": 1
	});   

    const pnlCabDesigner = Content.getComponent("pnlCabDesigner");
    const btnShowCabDesigner = Content.getComponent("btnShowCabDesigner");
    const btnCabDesignerEQEnable = Content.getComponent("btnCabDesignerEQEnable");
    const btnCabDesignerGenerate = Content.getComponent("btnCabDesignerGenerate");            
    const btnCabDesignerCustomMod = Content.getComponent("btnCabDesignerCustomMod");    
    const btnCabSave = Content.getComponent("btnCabSave");
    const btnOpenCabFolder = Content.getComponent("btnOpenCabFolder");
    const btnCloseCabDesigner = Content.getComponent("btnCloseCabDesigner");    
    const knbCabDesignerMojo = Content.getComponent("knbCabDesignerMojo");
    const knbCabDesignerAge = Content.getComponent("knbCabDesignerAge");
    const fltCabDesignerEQ = Content.getComponent("fltCabDesignerEQ");       
    const fltCabDesignerResponseCurve = Content.getComponent("fltCabDesignerResponseCurve");     
    const cmbCabDesignerSpeaker = Content.getComponent("cmbCabDesignerSpeaker");
    const cmbCabDesignerMic = Content.getComponent("cmbCabDesignerMic");
    const lblCabSaveName = Content.getComponent("lblCabSaveName");   
    const lblCabDesignerCustomModValue = Content.getComponent("lblCabDesignerCustomModValue");

    const modules = Synth.getAllEffects(".*");    
    const impulseSize = 1024;
    const moduleBypassedStates = [];
    const audioFiles = FileSystem.getFolder(FileSystem.AudioFiles);
    reg cabSaveName = "myCab.wav";          

    // Close Cab Designer
    inline function onbtnCloseCabDesignerControl(component, value) { if (value) { btnShowCabDesigner.setValue(0); btnShowCabDesigner.changed(); } }
    btnCloseCabDesigner.setControlCallback(onbtnCloseCabDesignerControl);	  

    // Save Module States
    inline function saveModuleBypassedState()
    {
        moduleBypassedStates.clear();
        for (m in modules) { moduleBypassedStates.push(m.isBypassed()); m.setBypassed(1);}
        cabDesigner.setBypassed(0);
        cabDesignerEQ.setBypassed(1 - btnCabDesignerEQEnable.getValue());
        cabFileSave.setBypassed(0);
        testAudio.setBypassed(1);   
    }

    // Restore Module States
    inline function restoreModuleBypassedState()
    {
        for (i=0; i<moduleBypassedStates.length; i++) { modules[i].setBypassed(moduleBypassedStates[i]); }        
        cabFileSave.setBypassed(1); testAudio.setBypassed(0); 
        inputGain.setBypassed(0); outputGain.setBypassed(0); preprocess.setBypassed(0); postprocess.setBypassed(0); // force
    }
    

    // Add MIDI Note
    inline function addNoteForCabSave(eventListToUse, noteNumber, startQuarter, durationQuarter)
    {
        // Flushes the MIDI list for the impulse player
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

    // Normalize Dirac Delta
    inline function normalizeBuffer(buffer)
    {        
        local bufferNormalized = buffer.clone();
        local peak = 0.0;   
        local v = -1.0;
        
        for (i = 0; i < impulseSize; i++)
        {
            v = Math.abs(bufferNormalized[0][i]);       
            if (v > peak) { peak = v; }                
        }

        if (peak > 0.0)
        {
            local scale = 1.0 / peak;   
            for (i = 0; i < impulseSize; i++) { bufferNormalized[0][i] *= scale; }                               
        }   
        return bufferNormalized;            
    }

    // File-writing for IR
    inline function renderAudioCallback (obj) 
    {        
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

            restoreModuleBypassedState();

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
        btnCabDesignerEQEnable.setValue(value); btnCabDesignerEQEnable.changed();
        pnlCabDesigner.set("visible", value);  
    }

    btnShowCabDesigner.setControlCallback(onbtnShowCabDesignerControl);    


    inline function onbtnCabDesignerCustomModControl(component, value)
    {
        cabDesigner.setAttribute(cabDesigner.CustomMod, value);
        lblCabDesignerCustomModValue.set("text", value ? "Enabled" : "Disabled");
    };

    btnCabDesignerCustomMod.setControlCallback(onbtnCabDesignerCustomModControl);


	// Select speaker
    inline function oncmbCabDesignerSpeakerControl(component, value) { cabDesigner.setAttribute(cabDesigner.SpeakerType, value-1); }
    cmbCabDesignerSpeaker.setControlCallback(oncmbCabDesignerSpeakerControl);    
    
    // Select microphone
    inline function oncmbCabDesignerMicControl(component, value) { cabDesigner.setAttribute(cabDesigner.MicrophoneType, value-1); }        
    cmbCabDesignerMic.setControlCallback(oncmbCabDesignerMicControl);

	// Save cab IR    
    inline function onbtnCabSaveControl(component, value)
    {	
        if (!value) { return; }
                        
        saveModuleBypassedState();
                            
        if (cabDesignerMIDIPlayer.isEmpty()) { cabDesignerMIDIPlayer.setFile("{PROJECT_FOLDER}dirac.mid"); }
        Engine.renderAudio(cabDesignerMIDIPlayer.getEventList(), renderAudioCallback);
    };

    btnCabSave.setControlCallback(onbtnCabSaveControl);

    // Create New Midi file (in case dirac.mid gets deleted)
    inline function createMidi()
    {
        local eventList = [];
        cabDesignerMIDIPlayer.create(4, 4, 1);
        addNoteForCabSave(eventList, 64, 0, 1.0);
        cabDesignerMIDIPlayer.setUseTimestampInTicks(true); 
        cabDesignerMIDIPlayer.flushMessageList(eventList);
        cabDesignerMIDIPlayer.setUseTimestampInTicks(false);
        cabDesignerMIDIPlayer.saveAsMidiFile("{PROJECT_FOLDER}dirac.mid", 0);     
    }

    // Create New Dirac file (in case dirac.wav gets deleted)
    inline function createDirac()
    {        
        local buffer = Buffer.create(impulseSize);
        buffer[0] = 1.0; // dirac
        local file = audioFiles.getChildFile("dirac.wav");
        file.writeAudioFile(buffer, Engine.getSampleRate(), 24);  
    }
    
    // Open Cab Folder
    inline function onbtnOpenCabFolderControl(component, value) { if (value) { audioFiles.show(); } }    
    btnOpenCabFolder.setControlCallback(onbtnOpenCabFolderControl);

    // Sanitize Filename
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

    // Look And Feel

    const LAFButtonCabDesignerCustomMod = Content.createLocalLookAndFeel();
    const LAFButtonCabDesignerGenerate = Content.createLocalLookAndFeel();
    const LAFButtonCabDesignerSave = Content.createLocalLookAndFeel();
    const LAFCabDesignerResponseCurve = Content.createLocalLookAndFeel();
    const btnCabDesignerSpeakerLAF = Content.createLocalLookAndFeel();

    const pnlCabDesignerSpeakerIcon = Content.getComponent("pnlCabDesignerSpeakerIcon");
    const pnlCabDesignerMicrophoneIcon = Content.getComponent("pnlCabDesignerMicrophoneIcon");

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

    inline function drawComboBox(g, obj)
    {
        // BG & Text
        g.setColour(obj.hover ? ColourData.clrMidgrey : ColourData.clrDarkgrey);
        g.fillRoundedRectangle(obj.area, 4.0);      
        g.setColour(ColourData.clrWhite);
        g.drawAlignedText(obj.text, [8, 0, obj.area[2] - 16, obj.area[3]], "left");     
        
        // Triangle
        local tXPad = 24;
        local tYPad = 12;
        local tX = obj.area[2] - tXPad;
        local tY = 8;
        local tW = 16;
        local tH = tY;        
        local tA = [tX, tYPad, tW, tH];
        g.fillTriangle(tA, Math.toRadians(180));
    }
    	
   	cmbCabDesignerLAF.registerFunction("drawComboBox", drawComboBox);        
    cmbCabDesignerSpeaker.setLocalLookAndFeel(cmbCabDesignerLAF);
    cmbCabDesignerMic.setLocalLookAndFeel(cmbCabDesignerLAF);

    inline function drawSpeakerIcon(g)
    {
        local x = 0; local y = 0; local w = this.getWidth(); local h = this.getHeight();
        local area = [x, y, w, h];
        local pad = 5;        
        local line = 1.5;
        local p = Content.createPath();
        p.loadFromData(PathData.pathSpeaker);
        g.setColour(ColourData.clrLightgrey);           
        g.drawPath(p, [x + pad, y + pad, w - (2 * pad), h - (2 * pad)], line);
    }

    pnlCabDesignerSpeakerIcon.setPaintRoutine(drawSpeakerIcon);
        
    inline function drawMicrophoneIcon(g)
    {
        local x = 0; local y = 0; local w = this.getWidth(); local h = this.getHeight();
        local area = [x, y, w, h];
        local line = 1.5;
        local baseW = w / 2; local baseH = Math.round(h * 0.4375);
        local baseX = w / 2 - Math.round(baseW / 2); local baseY = Math.round(h * 0.6875) - Math.round(baseH / 2);      
        local topW = Math.round(w * 0.36); local topH = Math.round(h * 0.58);
        local topX = w / 2 - Math.round(topW / 2); local topY = Math.round(h * 0.36) - Math.round(topH / 2);
        local p = Content.createPath();       
        g.setColour(ColourData.clrLightgrey);
        
        // base
        p.loadFromData(PathData.pathMicrophoneA);
        g.drawPath(p, [baseX, baseY, baseW, baseH], line);
        p.clear();
        
        // top
        p.loadFromData(PathData.pathMicrophoneB);
        g.drawPath(p, [topX, topY, topW, topH], line);
    }

	pnlCabDesignerMicrophoneIcon.setPaintRoutine(drawMicrophoneIcon);

    inline function drawCustomMod(g, obj)
    {
        local padY = 3;
        local sq = 6;
        local x1 = Math.round(obj.area[2] * 0.25); local x2 = Math.round(obj.area[2] * 0.5); local x3 = Math.round(obj.area[2] * 0.75);               
        
        if (obj.value) { g.setColour(obj.over ? ColourData.clrWhite : ColourData.clrLightgrey); }
        else { g.setColour(obj.over ? ColourData.clrLightgrey : ColourData.clrGrey); }
        
        // left
        local y1a = padY; local y1b = Math.round(obj.area[3] * 0.25); local y1c = Math.round(y1b + sq); local y1d = obj.area[3] - padY;
        g.drawLine(x1, x1, y1a, y1b, 1.0); g.drawLine(x1, x1, y1c, y1d, 1.0);
        g.fillRect([x1 - (sq / 2), y1b, sq, sq]);
        
        // center
        local y2a = padY; local y2b = Math.round(obj.area[3] * 0.65); local y2c = Math.round(y2b + sq); local y2d = obj.area[3] - padY;
        g.drawLine(x2, x2, y2a, y2b, 1.0); g.drawLine(x2, x2, y2c, y2d, 1.0);
        g.fillRect([x2 - (sq / 2), y2b, sq, sq]);
                
        // right
        local y3a = padY; local y3b = Math.round(obj.area[3] * 0.4); local y3c = Math.round(y3b + sq); local y3d = obj.area[3] - padY;
        g.drawLine(x3, x3, y3a, y3b, 1.0); g.drawLine(x3, x3, y3c, y3d, 1.0);
        g.fillRect([x3 - (sq / 2), y3b, sq, sq]);
    }
    
    LAFButtonCabDesignerCustomMod.registerFunction("drawToggleButton", drawCustomMod);    
    btnCabDesignerCustomMod.setLocalLookAndFeel(LAFButtonCabDesignerCustomMod);
        
    inline function drawCabGenerate(g, obj)
    {
	    local pad = 3;    	  
       	local ts = 6;  	    	    
       	local x = pad; local y = pad; local w = obj.area[2] - (2 * pad); local h = obj.area[3] - (2 * pad);
       	local area = [x, y, w, h];
       	g.setColour(obj.over ? ColourData.clrWhite : ColourData.clrLightgrey);       
       	
       	local p = Content.createPath();
       	p.clear();
       	p.loadFromData(PathData.pathGenerateCab);
       	g.drawPath(p, area, 2.0);
       	
    };
       
    LAFButtonCabDesignerGenerate.registerFunction("drawToggleButton", drawCabGenerate);    
    btnCabDesignerGenerate.setLocalLookAndFeel(LAFButtonCabDesignerGenerate);

    inline function drawSaveButton(g, obj)
    {
        local pad = 6;
        local x = pad; local y = pad; local w = obj.area[2] - (2 * pad); local h = obj.area[3] - (2 * pad);
        local area = [x, y, w, h];
        local inner = [pad + Math.round(w * 0.208), pad + Math.round(h * 0.08), Math.round(w * 0.604), Math.round(h * 0.381)];

        local p = Content.createPath();
        p.loadFromData(PathData.pathSave);
        g.setColour(obj.over ? ColourData.clrWhite : ColourData.clrLightgrey);          
        g.fillPath(p, area);
        
        g.setColour(ColourData.clrExtradarkgrey);
        g.fillRoundedRectangle(inner, 2.0);
    }
    
    LAFButtonCabDesignerSave.registerFunction("drawToggleButton", drawSaveButton);   
    btnCabSave.setLocalLookAndFeel(LAFButtonCabDesignerSave);
    
    inline function drawAnalyserBackground(g, obj)
    {
        g.fillAll(ColourData.clrComponentBGGrey);
    }

    inline function drawAnalyserPath(g, obj)
    {
        g.setColour(Colours.withAlpha(ColourData.clrMidgrey, .6));
        g.fillPath(obj.path, obj.area);
    }

    LAFCabDesignerResponseCurve.registerFunction("drawAnalyserBackground", drawAnalyserBackground);    
    LAFCabDesignerResponseCurve.registerFunction("drawAnalyserPath", drawAnalyserPath);        
    LAFCabDesignerResponseCurve.registerFunction("drawAnalyserGrid", function(g, obj) {}); // leave empty
    fltCabDesignerResponseCurve.setLocalLookAndFeel(LAFCabDesignerResponseCurve);        
}