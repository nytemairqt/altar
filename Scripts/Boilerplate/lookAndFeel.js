/*
    Copyright 2025 iamlamprey

    This file is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This file is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with This file. If not, see <http://www.gnu.org/licenses/>.
*/



namespace LookAndFeel
{			
	const LAFKnob = Content.createLocalLookAndFeel();
	const LAFKnobProcess = Content.createLocalLookAndFeel();
	const LAFButtonToggle = Content.createLocalLookAndFeel();
	const LAFButtonBypass = Content.createLocalLookAndFeel();
	const LAFButtonMenu = Content.createLocalLookAndFeel();
	const LAFButtonPrev = Content.createLocalLookAndFeel();	
	const LAFButtonNext = Content.createLocalLookAndFeel();
	const LAFButtonTunerMonitor = Content.createLocalLookAndFeel();      
	const LAFButtonInvertPhase = Content.createLocalLookAndFeel();
	const LAFButtonOpenCabDesigner = Content.createLocalLookAndFeel();
	const LAFButtonOpenCabFolder = Content.createLocalLookAndFeel();
	const LAFButtonEQFirst = Content.createLocalLookAndFeel();
	const LAFVuMeter = Content.createLocalLookAndFeel();	
	const LAFButtonTransposeSnap = Content.createLocalLookAndFeel();
		
	const start = -Math.PI * 0.75;	
	
	inline function reduced(obj, amount) { return [amount, amount, obj.area[2] - 2 * amount, obj.area[3] - 2 * amount]; }		
	
	inline function basicSlider(g, obj, text)
	{
		local x = obj.area[0];
	    local y = obj.area[1];
	    local w = obj.area[2];
	    local h = obj.area[3];	
	    local wKnb = text == ["processEQ"] || text == ["processComp"] ? 36 : 48;
	    local hKnb = text == ["processEQ"] || text == ["processComp"] ? 36 : 48;
	    local xKnb = Math.round((w - wKnb) / 2);
	    local yKnb = text == ["processEQ"] || text == ["processComp"] ? 4 : 8;
	    local areaKnob = [xKnb, yKnb, wKnb, hKnb];	
	    local ringWidth = wKnb / 16;
	    	
	    // Background
	    g.setColour(ColourData.clrExtradarkgrey);
	    g.fillEllipse(areaKnob);
	
	    // Unfilled Ring	    
	    local unfilled = Content.createPath();	    
	    unfilled.startNewSubPath(0.5, 1.0);
	    unfilled.addArc([0.0, 0.0, 1.0, 1.0], -Math.PI * 0.75, Math.PI * 0.75);	    	    
	    g.setColour(obj.hover ? ColourData.clrDarkgrey : ColourData.clrExtradarkgrey);
	    g.drawPath(unfilled, areaKnob, ringWidth * 2);
	    
	    // Filled Ring
	    local filled = Content.createPath();
	    filled.startNewSubPath(0.0, 0.0);
	   	filled.startNewSubPath(1.0, 1.0);
	    filled.addArc([0.0, 0.0, 1.0, 1.0], start, Math.max(start, start + Math.PI * 1.5 * obj.valueNormalized));
	    g.setColour(obj.hover ? ColourData.clrOffWhite : ColourData.clrLightgrey);
	    g.drawPath(filled, areaKnob, ringWidth * 1.50);
		    
		// Value Text (Before Rotating)
		local textToDisplay = "";			
		local textOffset = 30;		
		if (text == ["processEQ"]) { textOffset = 10; }		
		
		if (text == ["processEQ"]) { textToDisplay = obj.suffix == "%" ? Math.round(obj.value * 100) + "%" : obj.valueSuffixString; }
		else if (text == ["processComp"]) { textToDisplay = obj.suffix == "%" ? Math.round(obj.value * 100) + "%" : obj.valueSuffixString; }
		else if (text == [""]) { textToDisplay = obj.suffix == "%" ? Math.round(obj.value * 100) + "%" : obj.valueSuffixString;}
		else { textToDisplay = text[obj.value]; }		
		g.drawAlignedText(textToDisplay, [0, hKnb + textOffset, w, 20], "centred");
						    	
	    // Value Line (Rotated)
	    local angle = (1.0 - (obj.valueNormalized - 0.02)) * -1.5 * Math.PI;
	    local pivot = [xKnb + wKnb / 2, yKnb + hKnb / 2];
	    g.rotate(angle, pivot);		   
	    local lineX = .63;
	    local lineY = .83; 
	    g.drawLine(
	        xKnb + wKnb * lineX,	        
	        xKnb + wKnb * lineY,
	        yKnb + hKnb * lineX,
	        yKnb + hKnb * lineY,
	        4
	    );
	}
	
	// Main Slider	
	LAFKnob.registerFunction("drawRotarySlider", function(g, obj)
	{
		if (obj.text == "processEQ" || obj.text == "processComp")
		{
			return basicSlider(g, obj, [obj.text]); break;
		}		
		else
			switch (obj.id)
			{
				case "knbOverdriveMode": return basicSlider(g, obj, ["Fuzz", "Screamer", "RAT", "Bitcrusher", "Glitch", "Wavefolder"]); break;
				case "knbAmpMode": return basicSlider(g, obj, ["Clean", "Drive", "NAM"]); break;
				case "knbDelayMode": return basicSlider(g, obj, ["Normal", "Reverse", "Glitch"]); break;
				// FIX ME: ADD DELAY GLITCH MODES (AFTER FIXING MODULATION)
				default: return basicSlider(g, obj, [""]); break;
			}
	});		
	
	// Main Toggle Button
	
	LAFButtonToggle.registerFunction("drawToggleButton", function(g, obj)
	{
	    var x = obj.area[0];
	    var y = obj.area[1];
	    var w = obj.area[2];
	    var h = obj.area[3];
	    var r = 2; // reduce amount in px
	    g.setColour(ColourData.clrSliderBG);		    
	    g.fillEllipse(obj.area);
		
		if (obj.value) { g.setColour(obj.over ? ColourData.clrWhite : ColourData.clrLightgrey); }
		else { g.setColour(obj.over ? ColourData.clrLightgrey : ColourData.clrGrey); }
		g.fillEllipse([x + r, y + r, w - (2*r), h - (2*r)]);
		
	});	

	// Bypass Button
	LAFButtonBypass.registerFunction("drawToggleButton", function(g, obj)
	{
		if (obj.value) { g.setColour(obj.over ? ColourData.clrWhite : ColourData.clrLightgrey); }
		else { g.setColour(obj.over ? ColourData.clrLightgrey : ColourData.clrGrey); }
		var p = Content.createPath();		
		p.loadFromData(PathData.pathBypassButton);
		g.drawPath(p, [obj.area[0] + 2, obj.area[1] + 2, obj.area[2] - 4, obj.area[3] - 4], 2);
		g.drawLine(obj.area[2] / 2, obj.area[2] / 2, 0, obj.area[3] / 2, 2.0); 
	});

	// Next & Prev
	LAFButtonPrev.registerFunction("drawToggleButton", function(g, obj)
	{	
		g.setColour(obj.over ? ColourData.clrWhite : Colours.lightgrey);
	    g.fillTriangle(obj.area, Math.toRadians(270));  
	});
	
	LAFButtonNext.registerFunction("drawToggleButton", function(g, obj)
	{	
		g.setColour(obj.over ? ColourData.clrWhite : Colours.lightgrey);
		g.fillTriangle(obj.area, Math.toRadians(90));   
	});
	
	// Menu Buttons
	
	LAFButtonMenu.registerFunction("drawToggleButton", function(g, obj)
	{
		var linePad = 26;
		var lineY = 26;
		
		if (obj.value) { g.setColour(obj.over ? ColourData.clrWhite : ColourData.clrLightgrey); }
		else { g.setColour(obj.over ? ColourData.clrLightgrey : ColourData.clrGrey); }
			
		g.drawAlignedText(obj.text, obj.area, "centred");	
		
		if (obj.id == "btnShowPreProcess" || obj.id == "btnShowPostProcess") { lineY = 36; linePad = 26; }
		else if (obj.id == "btnShowPresetBrowser") {lineY = 26; linePad = 165;} // special case, since we need to change the text
		else { lineY = 26; linePad = 26; }					
		
		if (obj.value)
		{
			g.drawLine(linePad, obj.area[2] - linePad, lineY, lineY, 2.0);
		}
	});
		
	
	
	// Preprocess/Postprocess EQ First Button
	LAFButtonEQFirst.registerFunction("drawToggleButton", function(g, obj)
	{				
		g.setColour(obj.over ? ColourData.clrWhite : ColourData.clrLightgrey);

		if (obj.value) { g.drawAlignedText("EQ > Comp", obj.area, "centred"); }
		else { g.drawAlignedText("EQ < Comp", obj.area, "centred"); }
	});
	
	
	
	// Transpose Snape	
	LAFButtonTransposeSnap.registerFunction("drawToggleButton", function(g, obj)
	{
		if (obj.value) { g.setColour(obj.over ? ColourData.clrWhite : ColourData.clrLightgrey); }
		else { g.setColour(obj.over ? ColourData.clrLightgrey : ColourData.clrGrey); }
		
		var p = Content.createPath();
		p.loadFromData(PathData.pathChain);
		g.fillPath(p, [obj.area[0] + 2, obj.area[1] + 2, obj.area[2] - 4, obj.area[3] - 4]);						
		
	});
	
	// VU Meters
	LAFVuMeter.registerFunction("drawMatrixPeakMeter", function(g, obj)
	{
		var a = obj.area;
		var padL = 10;
		var thickness = a[2] / 2 - (padL * 2);
		var center = a[2] / 2;
				
		g.setColour(ColourData.clrExtradarkgrey);		
		g.fillRect(a);
		g.setColour(ColourData.clrDarkgrey);
		g.drawRect(a, 2.0);
		
		// left channel				
		var a1 = [padL + (center * 0), a[3] * (1 - obj.peaks[0]), thickness + (padL / 2), a[3] - a[3] * (1 - obj.peaks[0])];
		g.setGradientFill([clrGrey, 0, a[3], clrWhite, 0, a[3] - thickness - (a[3] - thickness) * obj.peaks[0]]);			
		g.fillRoundedRectangle(a1, 0);
		
		// right channel
		var a2 = [padL + (center * 1) - (padL / 2), a[3] * (1 - obj.peaks[1]), thickness + (padL / 2), a[3] - a[3] * (1 - obj.peaks[1])];
		g.setGradientFill([clrGrey, 0, a[3], clrWhite, 0, a[3] - thickness - (a[3] - thickness) * obj.peaks[0]]);			
		g.fillRoundedRectangle(a2, 0);				

	});

	// Tuner Monitor Button
    LAFButtonTunerMonitor.registerFunction("drawToggleButton", function(g, obj)
    {
        if (obj.value) { g.setColour(obj.over ? ColourData.clrWhite : ColourData.clrLightgrey); }
        else { g.setColour(obj.over ? ColourData.clrLightgrey : ColourData.clrGrey); }
        
        var p = Content.createPath();
        p.loadFromData(PathData.pathHeadphones);
        g.fillPath(p, [obj.area[0] + 2, obj.area[1] + 2, obj.area[2] - 4, obj.area[3] - 4]);                        
        
    });

    // Invert Phase
	LAFButtonInvertPhase.registerFunction("drawToggleButton", function(g, obj)
	{
		if (obj.value) { g.setColour(obj.over ? ColourData.clrWhite : ColourData.clrLightgrey); }
		else { g.setColour(obj.over ? ColourData.clrLightgrey : ColourData.clrGrey); }

		g.drawEllipse([4, 4, obj.area[2] - 8, obj.area[3] - 8], 2.0);
		g.drawLine(2, obj.area[2] - 2, obj.area[3] - 2, 2, 2.0);

	});
	
	// Open Cab Designer
	LAFButtonOpenCabDesigner.registerFunction("drawToggleButton", function(g, obj)
	{
		if (obj.value) { g.setColour(obj.over ? ColourData.clrWhite : ColourData.clrLightgrey); }
		else { g.setColour(obj.over ? ColourData.clrLightgrey : ColourData.clrGrey); }

		var p = Content.createPath();
        p.loadFromData(PathData.pathToolbox);
        
        // 64 x 64
        var padPath = Math.round(obj.area[2] * 0.03);
        var padBlock = Math.round(obj.area[2] * 0.078);
        var blockSize = Math.round(obj.area[2] * 0.1875);
        
        g.fillPath(p, [obj.area[0] + padPath, obj.area[1] + padPath, obj.area[2] - padPath * 2, obj.area[3] - padPath * 2]);          
        g.fillRoundedRectangle([obj.area[2] / 2 - padBlock, obj.area[3] / 2 - padBlock, blockSize, blockSize], 2.0);
	});
	
	// Open Cab Folder
	LAFButtonOpenCabFolder.registerFunction("drawToggleButton", function(g, obj)
	{
		if (obj.value) { g.setColour(obj.over ? ColourData.clrWhite : ColourData.clrLightgrey); }
		else { g.setColour(obj.over ? ColourData.clrLightgrey : ColourData.clrGrey); }

		var p = Content.createPath();
        p.loadFromData(PathData.pathFolder);
        var padX = Math.round(obj.area[2] * 0.093);
        var padY = Math.round(obj.area[2] * 0.1875);
        g.fillPath(p, [obj.area[0] + padX, obj.area[1] + padY, obj.area[2] - padX * 2, obj.area[3] - padY * 2]);                  
	});
	
	
	
	// Look And Feel Assignment
	const knbMainLAF = [Content.getComponent("knbInputGain"), Content.getComponent("knbGateThreshold"), Content.getComponent("knbTranspose"), Content.getComponent("knbOctave"), Content.getComponent("knbChugStrength"), Content.getComponent("knbPickStrength"), Content.getComponent("knbWhistle"), Content.getComponent("knbOutputGain"), Content.getComponent("knbLofi"), Content.getComponent("knbOverdriveMode"),  Content.getComponent("knbOverdriveDrive"),  Content.getComponent("knbOverdriveTone"),  Content.getComponent("knbOverdriveBits"),  Content.getComponent("knbOverdriveSRReduction"),  Content.getComponent("knbOverdriveFoldAmount"),  Content.getComponent("knbOverdriveMix"),  Content.getComponent("knbOverdriveOutputGain"), Content.getComponent("knbAmpMode"), Content.getComponent("knbAmpInput"), Content.getComponent("knbAmpLow"), Content.getComponent("knbAmpMid"), Content.getComponent("knbAmpHigh"), Content.getComponent("knbAmpPresence"), Content.getComponent("knbAmpOutput"), Content.getComponent("knbCabAAxis"), Content.getComponent("knbCabADistance"), Content.getComponent("knbCabADelay"), Content.getComponent("knbCabAPan"), Content.getComponent("knbCabAGain"), Content.getComponent("knbCabBAxis"), Content.getComponent("knbCabBDistance"), Content.getComponent("knbCabBDelay"), Content.getComponent("knbCabBPan"), Content.getComponent("knbCabBGain"), Content.getComponent("knbCabMix"),	    Content.getComponent("knbReverbMix"),  Content.getComponent("knbReverbPreDelay"),  Content.getComponent("knbReverbRoomSize"),  Content.getComponent("knbReverbDecay"),  Content.getComponent("knbReverbDampingFrequency"),  Content.getComponent("knbReverbChorusDepth"),      Content.getComponent("knbDelayMix"),  Content.getComponent("knbDelayMode"),  Content.getComponent("knbDelayDelayTime"),  Content.getComponent("knbDelayDelayTimeSynced"),  Content.getComponent("knbDelayFeedback"),  Content.getComponent("knbDelayModulation"),  Content.getComponent("knbDelayStereoWidth"),  Content.getComponent("knbDelayDamping"), Content.getComponent("knbChorusMix"),  Content.getComponent("knbChorusRate"),  Content.getComponent("knbChorusDepth"),  Content.getComponent("knbChorusTone"),  Content.getComponent("knbChorusVoices"),  Content.getComponent("knbChorusFeedback"),  Content.getComponent("knbChorusDelayTime"), 	                             Content.getComponent("knbRingmodMix"),  Content.getComponent("knbRingmodFrequency"),  Content.getComponent("knbRingmodDepth"),  Content.getComponent("knbRingmodMode"),  Content.getComponent("knbRingmodLFORate"),  Content.getComponent("knbRingmodLFODepth"),  Content.getComponent("knbRingmodFilterFrequency"), Content.getComponent("knbClickTempo"), Content.getComponent("knbClickGain"),];	
	const btnToggleLAF = [Content.getComponent("btnTransposeSnap"), ];
	const btnMenuLAF = [Content.getComponent("btnShowPreferences"), Content.getComponent("btnShowTuner"), Content.getComponent("btnShowClick"), Content.getComponent("btnShowPresetBrowser"), Content.getComponent("btnPresetPrev"), Content.getComponent("btnPresetNext"), Content.getComponent("btnShowPreProcess"), Content.getComponent("btnShowPostProcess"),];
	const btnBypassLAF = [Content.getComponent("btnGate"), Content.getComponent("btnTranspose"), Content.getComponent("btnOctave"), Content.getComponent("btnChug"), Content.getComponent("btnPick"), Content.getComponent("btnWhistle"), Content.getComponent("btnLimiter"), Content.getComponent("btnLofi"), Content.getComponent("btnCabAEnable"), Content.getComponent("btnCabBEnable"), Content.getComponent("btnPreProcessEQEnable"), Content.getComponent("btnPreProcessCompEnable"), Content.getComponent("btnPreProcessClipperEnable"), Content.getComponent("btnPostProcessEQEnable"), Content.getComponent("btnPostProcessCompEnable"), Content.getComponent("btnTuner"), Content.getComponent("btnClick")];
	const btnPrevLAF = [Content.getComponent("btnPresetPrev"), Content.getComponent("btnCabALoadPrev"), Content.getComponent("btnCabBLoadPrev")];	
	const btnNextLAF = [Content.getComponent("btnPresetNext"), Content.getComponent("btnCabALoadNext"), Content.getComponent("btnCabBLoadNext")];	
	const fltVuMeterLAF = [Content.getComponent("fltVuMeterInput"), Content.getComponent("fltVuMeterOutput")];
	
	const btnTransposeSnap = [Content.getComponent("btnTransposeSnap")];
	const btnInvertPhaseLAF = [Content.getComponent("btnCabAPhase"), Content.getComponent("btnCabBPhase")];
	const knbPreprocessLAF = [Content.getComponent("btnPreProcessEQFirst"), Content.getComponent("knbPreprocessHpfFreq"), Content.getComponent("knbPreprocessLowShelfFreq"), Content.getComponent("knbPreprocessLowShelfGain"), Content.getComponent("knbPreprocessLowShelfQ"), Content.getComponent("knbPreprocessLowMidFreq"), Content.getComponent("knbPreprocessLowMidGain"), Content.getComponent("knbPreprocessLowMidQ"), Content.getComponent("knbPreprocessMidFreq"), Content.getComponent("knbPreprocessMidGain"), Content.getComponent("knbPreprocessMidQ"), Content.getComponent("knbPreprocessHighMidFreq"), Content.getComponent("knbPreprocessHighMidGain"), Content.getComponent("knbPreprocessHighMidQ"), Content.getComponent("knbPreprocessHighShelfFreq"), Content.getComponent("knbPreprocessHighShelfGain"), Content.getComponent("knbPreprocessHighShelfQ"), Content.getComponent("knbPreprocessLpfFreq"), Content.getComponent("knbPreprocessCompThreshold"), Content.getComponent("knbPreprocessCompRatio"), Content.getComponent("knbPreprocessCompRelease"), Content.getComponent("knbPreprocessCompAttack"), Content.getComponent("knbPreprocessCompMix"), Content.getComponent("knbPreprocessCompKnee"), Content.getComponent("knbPreprocessCompMakeup"), Content.getComponent("knbPreprocessClipperGain")];
	const knbPostprocessLAF = [Content.getComponent("btnPostProcessEQFirst"), Content.getComponent("knbPostprocessHpfFreq"), Content.getComponent("knbPostprocessLowShelfFreq"), Content.getComponent("knbPostprocessLowShelfGain"), Content.getComponent("knbPostprocessLowShelfQ"), Content.getComponent("knbPostprocessLowMidFreq"), Content.getComponent("knbPostprocessLowMidGain"), Content.getComponent("knbPostprocessLowMidQ"), Content.getComponent("knbPostprocessMidFreq"), Content.getComponent("knbPostprocessMidGain"), Content.getComponent("knbPostprocessMidQ"), Content.getComponent("knbPostprocessHighMidFreq"), Content.getComponent("knbPostprocessHighMidGain"), Content.getComponent("knbPostprocessHighMidQ"), Content.getComponent("knbPostprocessHighShelfFreq"), Content.getComponent("knbPostprocessHighShelfGain"), Content.getComponent("knbPostprocessHighShelfQ"), Content.getComponent("knbPostprocessLpfFreq"), Content.getComponent("knbPostprocessCompThreshold"), Content.getComponent("knbPostprocessCompRatio"), Content.getComponent("knbPostprocessCompRelease"), Content.getComponent("knbPostprocessCompAttack"), Content.getComponent("knbPostprocessCompMix"), Content.getComponent("knbPostprocessCompKnee"), Content.getComponent("knbPostprocessCompMakeup")];
	const btnEQFirstLAF = [Content.getComponent("btnPreProcessEQFirst"), Content.getComponent("btnPostProcessEQFirst")];
	const btnTunerMonitorLAF = [Content.getComponent("btnTunerMonitor")];  
	const btnShowCabDesignerLAF = [Content.getComponent("btnShowCabDesigner")];
	const btnOpenCabFolderLAF = [Content.getComponent("btnOpenCabFolder")];

	for (k in knbMainLAF) { k.setLocalLookAndFeel(LAFKnob); }	  
	for (k in knbPreprocessLAF) { k.setLocalLookAndFeel(LAFKnob); }  
	for (k in knbPostprocessLAF) { k.setLocalLookAndFeel(LAFKnob); }  
	for (b in btnToggleLAF) { b.setLocalLookAndFeel(LAFButtonToggle); }
	for (b in btnMenuLAF) { b.setLocalLookAndFeel(LAFButtonMenu); }
	for (b in btnBypassLAF) { b.setLocalLookAndFeel(LAFButtonBypass); }
	for (b in btnPrevLAF) { b.setLocalLookAndFeel(LAFButtonPrev); }
	for (b in btnNextLAF) { b.setLocalLookAndFeel(LAFButtonNext); }	
	for (b in btnEQFirstLAF) { b.setLocalLookAndFeel(LAFButtonEQFirst); }
	for (b in btnTunerMonitorLAF) { b.setLocalLookAndFeel(LAFButtonTunerMonitor); }
	for (b in btnTransposeSnap) { b.setLocalLookAndFeel(LAFButtonTransposeSnap); }
	for (b in btnInvertPhaseLAF) { b.setLocalLookAndFeel(LAFButtonInvertPhase); }
	for (b in btnShowCabDesignerLAF) { b.setLocalLookAndFeel(LAFButtonOpenCabDesigner); }
	for (b in btnOpenCabFolderLAF) { b.setLocalLookAndFeel(LAFButtonOpenCabFolder); }
	for (f in fltVuMeterLAF) { f.setLocalLookAndFeel(LAFVuMeter); }	
}
