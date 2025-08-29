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

inline function onknbCabDesignerAgeControl(component, value)
{
	local hi = 0 * cabDesignerAge.BandOffset + cabDesignerAge.Gain;
	local lo = 1 * cabDesignerAge.BandOffset + cabDesignerAge.Gain;
	local hiScaled = -0.0 - (5.0 * value);
	local loScaled = -0.0 + (4.0 * value);
	cabDesignerAge.setAttribute(hi, hiScaled);
	cabDesignerAge.setAttribute(lo, loScaled);	
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

inline function onknbProcessControl(component, value)
{
	local point = 0.0; 
	switch (component)
	{
		// PREPROCESS
		// High Pass
		case knbPreProcessEQHighPass:
			point = 0 * preProcessEQ.BandOffset + preProcessEQ.Freq;   
			preProcessEQ.setAttribute(point, value);			
			break;
		// Low Shelf
		case knbPreProcessEQLowFreq:
			point = 1 * preProcessEQ.BandOffset + preProcessEQ.Freq;   
			preProcessEQ.setAttribute(point, value);	
			break;			
		case knbPreProcessEQLowGain:
			point = 1 * preProcessEQ.BandOffset + preProcessEQ.Gain;   
			preProcessEQ.setAttribute(point, value);	
			break;
		// Low Mid
		case knbPreProcessEQLowMidFreq:
			point = 2 * preProcessEQ.BandOffset + preProcessEQ.Freq;
			preProcessEQ.setAttribute(point, value);	
			break;
		case knbPreProcessEQLowMidGain:
			point = 2 * preProcessEQ.BandOffset + preProcessEQ.Gain;
			preProcessEQ.setAttribute(point, value);	
			break;
		case knbPreProcessEQLowMidQ:
			point = 2 * preProcessEQ.BandOffset + preProcessEQ.Q;
			preProcessEQ.setAttribute(point, value);	
			break;
		// High Mid
		case knbPreProcessEQHighMidFreq:
			point = 3 * preProcessEQ.BandOffset + preProcessEQ.Freq;
			preProcessEQ.setAttribute(point, value);	
			break;
		case knbPreProcessEQHighMidGain:
			point = 3 * preProcessEQ.BandOffset + preProcessEQ.Gain;
			preProcessEQ.setAttribute(point, value);	
			break;
		case knbPreProcessEQHighMidQ:
			point = 3 * preProcessEQ.BandOffset + preProcessEQ.Q;
			preProcessEQ.setAttribute(point, value);	
			break;
		// High Shelf
		case knbPreProcessEQHighFreq:
			point = 4 * preProcessEQ.BandOffset + preProcessEQ.Freq;   
			preProcessEQ.setAttribute(point, value);	
			break;			
		case knbPreProcessEQHighGain:
			point = 4 * preProcessEQ.BandOffset + preProcessEQ.Gain;   
			preProcessEQ.setAttribute(point, value);	
			break;
		// Low Pass
		case knbPreProcessEQLowPass:
			point = 5 * preProcessEQ.BandOffset + preProcessEQ.Freq;   
			preProcessEQ.setAttribute(point, value);			
			break;

		// POSTPROCESS
		// High Pass
		case knbPostProcessEQHighPass:
			point = 0 * postProcessEQ.BandOffset + postProcessEQ.Freq;   
			postProcessEQ.setAttribute(point, value);			
			break;
		// Low Shelf
		case knbPostProcessEQLowFreq:
			point = 1 * postProcessEQ.BandOffset + postProcessEQ.Freq;   
			postProcessEQ.setAttribute(point, value);	
			break;			
		case knbPostProcessEQLowGain:
			point = 1 * postProcessEQ.BandOffset + postProcessEQ.Gain;   
			postProcessEQ.setAttribute(point, value);	
			break;
		// Low Mid
		case knbPostProcessEQLowMidFreq:
			point = 2 * postProcessEQ.BandOffset + postProcessEQ.Freq;
			postProcessEQ.setAttribute(point, value);	
			break;
		case knbPostProcessEQLowMidGain:
			point = 2 * postProcessEQ.BandOffset + postProcessEQ.Gain;
			postProcessEQ.setAttribute(point, value);	
			break;
		case knbPostProcessEQLowMidQ:
			point = 2 * postProcessEQ.BandOffset + postProcessEQ.Q;
			postProcessEQ.setAttribute(point, value);	
			break;
		// High Mid
		case knbPostProcessEQHighMidFreq:
			point = 3 * postProcessEQ.BandOffset + postProcessEQ.Freq;
			postProcessEQ.setAttribute(point, value);	
			break;
		case knbPostProcessEQHighMidGain:
			point = 3 * postProcessEQ.BandOffset + postProcessEQ.Gain;
			postProcessEQ.setAttribute(point, value);	
			break;
		case knbPostProcessEQHighMidQ:
			point = 3 * postProcessEQ.BandOffset + postProcessEQ.Q;
			postProcessEQ.setAttribute(point, value);	
			break;
		// High Shelf
		case knbPostProcessEQHighFreq:
			point = 4 * postProcessEQ.BandOffset + postProcessEQ.Freq;   
			postProcessEQ.setAttribute(point, value);	
			break;			
		case knbPostProcessEQHighGain:
			point = 4 * postProcessEQ.BandOffset + postProcessEQ.Gain;   
			postProcessEQ.setAttribute(point, value);	
			break;
		// Low Pass
		case knbPostProcessEQLowPass:
			point = 5 * postProcessEQ.BandOffset + postProcessEQ.Freq;   
			postProcessEQ.setAttribute(point, value);			
			break;
	}
}

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
knbCabDesignerAge.setControlCallback(onknbCabDesignerAgeControl);

knbPreProcessEQHighPass.setControlCallback(onknbProcessControl);
knbPreProcessEQLowFreq.setControlCallback(onknbProcessControl);
knbPreProcessEQLowGain.setControlCallback(onknbProcessControl);
knbPreProcessEQLowMidFreq.setControlCallback(onknbProcessControl);
knbPreProcessEQLowMidGain.setControlCallback(onknbProcessControl);
knbPreProcessEQLowMidQ.setControlCallback(onknbProcessControl);
knbPreProcessEQHighMidFreq.setControlCallback(onknbProcessControl);
knbPreProcessEQHighMidGain.setControlCallback(onknbProcessControl);
knbPreProcessEQHighMidQ.setControlCallback(onknbProcessControl);
knbPreProcessEQHighFreq.setControlCallback(onknbProcessControl);
knbPreProcessEQHighGain.setControlCallback(onknbProcessControl);
knbPreProcessEQLowPass.setControlCallback(onknbProcessControl);
knbPostProcessEQHighPass.setControlCallback(onknbProcessControl);
knbPostProcessEQLowFreq.setControlCallback(onknbProcessControl);
knbPostProcessEQLowGain.setControlCallback(onknbProcessControl);
knbPostProcessEQLowMidFreq.setControlCallback(onknbProcessControl);
knbPostProcessEQLowMidGain.setControlCallback(onknbProcessControl);
knbPostProcessEQLowMidQ.setControlCallback(onknbProcessControl);
knbPostProcessEQHighMidFreq.setControlCallback(onknbProcessControl);
knbPostProcessEQHighMidGain.setControlCallback(onknbProcessControl);
knbPostProcessEQHighMidQ.setControlCallback(onknbProcessControl);
knbPostProcessEQHighFreq.setControlCallback(onknbProcessControl);
knbPostProcessEQHighGain.setControlCallback(onknbProcessControl);
knbPostProcessEQLowPass.setControlCallback(onknbProcessControl);

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
			if (!value)
			{
				btnClick.setValue(0);	
				btnClick.changed();
			}
				
			break;	
		case btnShowPreProcess:
			pnlPreProcess.set("visible", value);
			break;
		case btnShowPostProcess:
			pnlPostProcess.set("visible", value);
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
	cabDesignerAge.setBypassed(1-value);
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
//btnShowOverdrive.setControlCallback(showPanelControl);
//btnShowAmp.setControlCallback(showPanelControl);
//btnShowCab.setControlCallback(showPanelControl);
//btnShowReverb.setControlCallback(showPanelControl);
//btnShowDelay.setControlCallback(showPanelControl);
//btnShowChorus.setControlCallback(showPanelControl);
//btnShowRingMod.setControlCallback(showPanelControl);
btnShowTuner.setControlCallback(showPanelControl);
btnShowPreProcess.setControlCallback(showPanelControl);
btnShowPostProcess.setControlCallback(showPanelControl);

// Panels
// FIX ME
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

pnlTuner.setPaintRoutine(function(g) // move to LAF later
{
	var bounds = [310, 80, 530, 310];

	g.setColour(Colours.withAlpha(Colours.black, 1.0));
	g.fillRoundedRectangle(bounds, 2.0);
});


pnlTuner.setMouseCallback(function(event)
{
	var x = 310;
	var y = 80;
	var w = 530;
	var h = 310;
	
	if (event.mouseDownX < x || event.mouseDownX > (x + w) || event.mouseDownY < y || event.mouseDownY > (y + h)) 
	{
		btnShowTuner.setValue(0);
		btnShowTuner.changed();
	}	
});

pnlPreProcess.setPaintRoutine(function(g)
{
	var bounds = [200, 250, 700, 450];

	g.setColour(Colours.withAlpha(Colours.red, 1.0));
	g.fillRoundedRectangle(bounds, 2.0);
});

pnlPreProcess.setMouseCallback(function(event)
{
	var x = 200;
	var y = 250;
	var w = 700;
	var h = 450;
	
	if (event.mouseDownX < x || event.mouseDownX > (x + w) || event.mouseDownY < y || event.mouseDownY > (y + h)) 
	{
		btnShowPreProcess.setValue(0);
		btnShowPreProcess.changed();
	}	
});

pnlPostProcess.setPaintRoutine(function(g)
{
	var bounds = [200, 250, 700, 450];

	g.setColour(Colours.withAlpha(Colours.green, 1.0));
	g.fillRoundedRectangle(bounds, 2.0);
});

pnlPostProcess.setMouseCallback(function(event)
{
	var x = 200;
	var y = 250;
	var w = 700;
	var h = 450;
	
	if (event.mouseDownX < x || event.mouseDownX > (x + w) || event.mouseDownY < y || event.mouseDownY > (y + h)) 
	{
		btnShowPostProcess.setValue(0);
		btnShowPostProcess.changed();
	}	
});

inline function pnlFxSlotPaint(g)
{
	local area = [0, 0, this.getWidth(), this.getHeight()];
	
	g.setColour(Colours.white);
	g.drawRoundedRectangle(area, 1.0, 1.0);
	//g.drawAlignedText(this.get("text"), area, "centred");
	
	local newText = "";
	
	switch (this.get("text"))
	{
		case "overdrive":
			newText = "OD";
			break;
		case "amp":
			newText = "AMP";
			break;
		case "cab":
			newText = "CAB";
			break;
		case "reverb":
			newText = "RVB";
			break;
		case "delay":
			newText = "DLY";
			break;
		case "chorus":
			newText = "CHR";
			break;
		case "ringmod":
			newText = "RNG";
			break;	
	}
	
	g.drawAlignedText(newText, area, "centred");
					
	if (this.data.isTarget)
		g.drawLine(5, area[2]-5, area[3]-5, area[3]-5, 1.0);
	
	g.setColour(Colours.withAlpha(Colours.white, .3));
	g.fillRoundedRectangle([area[2] - 14, area[3] - 14, 4, 4], 3.0);
	g.fillRoundedRectangle([area[2] - 8, area[3] - 14, 4, 4], 3.0);
	g.fillRoundedRectangle([area[2] - 14, area[3] - 8, 4, 4], 3.0);
	g.fillRoundedRectangle([area[2] - 8, area[3] - 8, 4, 4], 3.0);
}

inline function pnlFxSlotReloadText()
{
	local newText = "";

	for (i=0; i<pnlFxSlots.length; i++)
	{
		pnlFxSlots[i].set("text", fxSlots[i].getCurrentEffectId());
		pnlFxSlots[i].repaint();
	}
}

for (p in pnlFxSlots)
	p.setPaintRoutine(pnlFxSlotPaint);

pnlFxSlotReloadText(); 

inline function repaintPnlFxSlots()
{
	for (p in pnlFxSlots)
	{
		p.data.isTarget = false;
		p.repaint();
	}
}

function pnlFxSlotsDragPaint(g, obj)
{
	var isValid = false;
	isValid = obj.valid;
	isValid |= obj.target == "";
	isValid |= obj.target.length == 0;
	isValid |= obj.target == obj.source;

	// probably ways to improve this
	if (isValid && obj.target != "" && obj.target != obj.source)
	{
		repaintPnlFxSlots();

		var component = Content.getComponent(obj.target);
		component.data.isTarget = true;
		component.repaint();
	}
		
	g.setColour(Colours.withAlpha(Colours.white, 0.2));
	g.fillRoundedRectangle(obj.area, 2.0);
	g.setColour(Colours.withAlpha(Colours.white, 0.6));
	g.fillRoundedRectangle(obj.area, 2.0, 2.0);	
}

function onPnlFxSlotsDrag(isValid, targetName)
{
	if (targetName != "")
		var target = Content.getComponent(targetName);
	
	repaintPnlFxSlots();
		
	if (!pnlFxSlots.contains(target))
		return;
	
	if (target == this)
		return;
		
	var xT = target.get("x");
	var xP = this.get("x");
	
	var currentIndex = pnlFxSlots.indexOf(this);
	var targetIndex = pnlFxSlots.indexOf(target);
	var currentName = this.get("text");
	var targetName  = target.get("text");
	var currentSlot = fxSlots[currentIndex];
	var targetSlot = fxSlots[targetIndex]; 
	
	
	// swap FX
	currentSlot.setEffect(targetName);
	targetSlot.setEffect(currentName);
	
	// get the new effects (after swap)
	var targetEffect = targetSlot.getCurrentEffect(); // pan
	var targetEffectName = targetSlot.getCurrentEffectId(); // pan
	var currentEffect = currentSlot.getCurrentEffect(); // gain
	var currentEffectName = currentSlot.getCurrentEffectId(); // gain
	
	// FIX ME
	// UNCOMMENT ME WHEN UI IS DONE
	/*
	for (k in knbs)
		k.changed();		
	*/
	
	// repaint
	pnlFxSlotReloadText();
}

inline function checkValidPnlFxSlot(targetId)
{
	// called whenever hover over
	
	// update drag image
	Content.refreshDragImage();		
	local pnlFxSlotNames = ["pnlFxSlotA", "pnlFxSlotB", "pnlFxSlotC", "pnlFxSlotD", "pnlFxSlotE", "pnlFxSlotF", "pnlFxSlotG"];	
	return pnlFxSlotNames.contains(targetId);
}

inline function dragPnlFxSlot(event)
{
	if (event.clicked && !event.rightClick && event.mouseDownX > this.getWidth() - 16 && event.mouseDownY > this.getHeight() - 16)
	{
		this.startInternalDrag({
			area: [0, 0, 25, 25],
			paintRoutine: pnlFxSlotsDragPaint,
			dragCallback: onPnlFxSlotsDrag,
			isValid: checkValidPnlFxSlot
		});
	}	
}

for (p in pnlFxSlots)
	p.setMouseCallback(dragPnlFxSlot);

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