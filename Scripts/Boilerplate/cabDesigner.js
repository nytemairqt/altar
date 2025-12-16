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
    const cabDesignerInput = Synth.getEffect("cabDesignerInput");
    const cabDesignerRecorder = Synth.getEffect("cabDesignerRecorder");
    const fxSlots = [Synth.getSlotFX("modularA"), Synth.getSlotFX("modularB"), Synth.getSlotFX("modularC"), Synth.getSlotFX("modularD"), Synth.getSlotFX("modularE"), Synth.getSlotFX("modularF"), Synth.getSlotFX("modularG")];                	    
    const testAudio = Synth.getChildSynth("testAudio");                            
    const inputGain = Synth.getEffect("inputGain");
    const outputGain = Synth.getEffect("outputGain");        

    const pnlCabDesigner = Content.getComponent("pnlCabDesigner");
    const btnShowCabDesigner = Content.getComponent("btnShowCabDesigner");
    const btnCabDesignerEQEnable = Content.getComponent("btnCabDesignerEQEnable");
    const btnCabDesignerGenerate = Content.getComponent("btnCabDesignerGenerate");            
    const btnCabDesignerMixReady = Content.getComponent("btnCabDesignerMixReady");    
    const btnCabDesignerSave = Content.getComponent("btnCabDesignerSave");
    const btnOpenCabFolder = Content.getComponent("btnOpenCabFolder");
    const btnCloseCabDesigner = Content.getComponent("btnCloseCabDesigner");    
    
    const knbCabDesignerMojo = Content.getComponent("knbCabDesignerMojo");
    const knbCabDesignerAge = Content.getComponent("knbCabDesignerAge");
    const fltCabDesignerEQ = Content.getComponent("fltCabDesignerEQ");       
    const fltCabDesignerResponseCurve = Content.getComponent("fltCabDesignerResponseCurve");     
    const cmbCabDesignerSpeaker = Content.getComponent("cmbCabDesignerSpeaker");
    const cmbCabDesignerMic = Content.getComponent("cmbCabDesignerMic");
    const lblCabDesignerSave = Content.getComponent("lblCabDesignerSave");   
    const lblCabDesignerMixReadyValue = Content.getComponent("lblCabDesignerMixReadyValue");    
            
    const audioFiles = FileSystem.getFolder(FileSystem.AudioFiles);    

    const impulseSize = 1024;    
    global cabRecord = false;
    global cabBuffer = [];  
    reg pendingCabFile = undefined;
            
    const timerCabSave = Engine.createTimerObject();

    inline function timerCabSaveCallback()
    {
        cabRecord = false;

        // safety: snapshot and clear pending file
        local fileToSave = pendingCabFile;
        pendingCabFile = undefined;
        
        lblCabDesignerSave.set("text", "Save");
        btnCabDesignerSave.set("enabled", true);

        // if we have data, write it now
        if (isDefined(fileToSave) && cabBuffer.length > 0)
        {
            local audioData = reconstructBuffer();
            renderAudio(audioData, fileToSave);
        }
        else
        {
            Console.print("No recorded data available after capture window.");
            cabBuffer.clear();
        }

        timerCabSave.stopTimer();
    }

    timerCabSave.setTimerCallback(timerCabSaveCallback);


    inline function onbtnCabDesignerSaveControl(component, value)
    {
        if (!value) return;

        // force enable
        cabDesignerRecorder.setBypassed(0);

        FileSystem.browse(FileSystem.AudioFiles, true, "*.wav", function(result)
        {
            // cancelled by user
            if (!result || result.toString(0) == "")
            {
                Console.print("File browser cancelled.");
                cabRecord = false; 
                cabBuffer.clear(); 
                return;
            }

            if (!result.hasWriteAccess())
            {
                Console.print("File does not have write-access. Cancelling.");
                cabRecord = false; 
                cabBuffer.clear(); 
                return;
            }

            // prevent saving over in-use file
            for (slot in fxSlots)
            {
                if (slot.getCurrentEffectId() == "cab")
                {
                    var id = slot.getCurrentEffect().getId();
                    var ref = Synth.getAudioSampleProcessor(id);
                    var cabAFile = ref.getAudioFile(0).getCurrentlyLoadedFile();
                    var cabBFile = ref.getAudioFile(1).getCurrentlyLoadedFile();
                    if (cabAFile == result.toString(0) || cabBFile == result.toString(0))
                    {
                        Console.print("File in use by cab, please choose a different filename.");
                        Engine.showErrorMessage("File in use by cab, please choose a different filename.", false);
                        cabRecord = false; 
                        cabBuffer.clear(); 
                        return;
                    }
                }
            }

            cabBuffer.clear();
            pendingCabFile = result;
            cabRecord = true;

            // trigger dirac
            Synth.addNoteOn(1, 64, 64, 0);
            Synth.addNoteOff(1, 64, 20000);
                    
            lblCabDesignerSave.set("text", "Saving...");
            btnCabDesignerSave.set("enabled", false);
            timerCabSave.startTimer(1000); // give the audio thread time to capture
        });
    }    

    btnCabDesignerSave.setControlCallback(onbtnCabDesignerSaveControl);    
	
    inline function reconstructBuffer()
    {
        if (cabBuffer.length == 0) { return; }
        
        local blockLen = cabBuffer[0][0].length;
        local numBlocks = cabBuffer.length;
        local totalLen = numBlocks * blockLen;

        local audioData = [Buffer.create(totalLen), Buffer.create(totalLen)];
        
        for (b = 0; b < numBlocks; b++)
        {
            // l
            {
                local srcL = cabBuffer[b][0];
                local dstL = Buffer.referTo(audioData[0], b * srcL.length, srcL.length);
                srcL >> dstL;
            }
            // r
            {
                local srcR = cabBuffer[b][1];
                local dstR = Buffer.referTo(audioData[1], b * srcR.length, srcR.length);
                srcR >> dstR;
            }
        }

        cabBuffer.clear();                     
        return audioData;
    }

    reg writeBuffer;
    	
    inline function renderAudio(audioData, fileToSave)
    {        
        writeBuffer = Buffer.referTo(audioData[0]);
        local trimStart = 0;
        local threshold = 0.05;
        local pad = 16;
        
        for (i = 0; i < writeBuffer.length; i++)
        {
            if (Math.abs(writeBuffer[i]) >= threshold)
            {
                trimStart = i;
                break;
            }            
        }    

        trimStart -= pad;        

        local output = writeBuffer.getSlice(trimStart, impulseSize); // trim
        if (output.length < impulseSize) { Console.print("Error trimming, cancelling."); cabRecord = false; cabBuffer.clear(); return; }         
        output.normalise(-0.1); // normalize        
                
        fileToSave.writeAudioFile(output, Engine.getSampleRate(), 24); // mono, use [L, R] for stereo
        Engine.loadAudioFilesIntoPool(); // refresh cab list        
        cabRecord = false;        
    }        

    inline function onbtnCloseCabDesignerControl(component, value) { if (value) { btnShowCabDesigner.setValue(0); btnShowCabDesigner.changed(); } }
    btnCloseCabDesigner.setControlCallback(onbtnCloseCabDesignerControl);	      

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
        cabDesignerInput.setBypassed(1-value);
        cabDesignerRecorder.setBypassed(1-value);
        cabRecord = false;
        cabBuffer.clear();  
        testAudio.setBypassed(value); // make sure this doesn't interfere with cab recording
        btnCabDesignerEQEnable.setValue(value); btnCabDesignerEQEnable.changed();
        pnlCabDesigner.set("visible", value);  
    }

    btnShowCabDesigner.setControlCallback(onbtnShowCabDesignerControl);    

    inline function onbtnCabDesignerMixReadyControl(component, value)
    {	
        cabDesigner.setAttribute(cabDesigner.MixReady, value);
        lblCabDesignerMixReadyValue.set("text", value ? "Enabled" : "Disabled");
    };

    btnCabDesignerMixReady.setControlCallback(onbtnCabDesignerMixReadyControl);    

    inline function oncmbCabDesignerSpeakerControl(component, value) { cabDesigner.setAttribute(cabDesigner.SpeakerType, value-1); }
    cmbCabDesignerSpeaker.setControlCallback(oncmbCabDesignerSpeakerControl);    
    
    inline function oncmbCabDesignerMicControl(component, value) { cabDesigner.setAttribute(cabDesigner.MicrophoneType, value-1); }        
    cmbCabDesignerMic.setControlCallback(oncmbCabDesignerMicControl);

    // Create New Dirac file (in case dirac.wav gets deleted)
    inline function createDirac()
    {        
        local buffer = Buffer.create(impulseSize);
        buffer[0] = 1.0; // dirac
        local file = audioFiles.getChildFile("dirac.wav");
        file.writeAudioFile(buffer, Engine.getSampleRate(), 24);  
    }
    
    inline function onbtnOpenCabFolderControl(component, value) { if (value) { audioFiles.show(); } }    
    btnOpenCabFolder.setControlCallback(onbtnOpenCabFolderControl);

    // Look And Feel

    const LAFButtonCabDesignerMixReady = Content.createLocalLookAndFeel();
    const LAFButtonCabDesignerGenerate = Content.createLocalLookAndFeel();
    const LAFButtonCabDesignerSave = Content.createLocalLookAndFeel();
    const LAFCabDesignerResponseCurve = Content.createLocalLookAndFeel();
    const btnCabDesignerSpeakerLAF = Content.createLocalLookAndFeel();

    const pnlCabDesignerSpeakerIcon = Content.getComponent("pnlCabDesignerSpeakerIcon");
    const pnlCabDesignerMicrophoneIcon = Content.getComponent("pnlCabDesignerMicrophoneIcon");
    
    const bufferSource = Synth.getDisplayBufferSource("cabDesigner");
    const bufferDisplay = bufferSource.getDisplayBuffer(0);    
    bufferDisplay.setRingBufferProperties({"BufferLength": 2048, "WindowType": "Hann"});

    // "Rectangle", "Hamming", "Hann", "Blackman Harris", "Triangle", "FlatTop", "Kaiser"

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

    inline function drawMixReady(g, obj)
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
    
    LAFButtonCabDesignerMixReady.registerFunction("drawToggleButton", drawMixReady);    
    btnCabDesignerMixReady.setLocalLookAndFeel(LAFButtonCabDesignerMixReady);
        
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
    btnCabDesignerSave.setLocalLookAndFeel(LAFButtonCabDesignerSave);

    reg testBool = false;
    reg analyserPath;
    reg myCoolPath;
    
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