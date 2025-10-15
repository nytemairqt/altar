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
	const LAFSlider = Content.createLocalLookAndFeel();
	const LAFButtonToggle = Content.createLocalLookAndFeel();
	const LAFButtonMenu = Content.createLocalLookAndFeel();	
	
	
	var path = Content.createPath();
	
	const clrRhapsodyBlue = 0xFF1D1D21;
	const clrExtradarkblue = 0xFF191933;
	const clrOffWhite = 0xFFEDEDED;
	
	// Old NEAT Player Colours
	const clrBggrey = 0xFF121212;    
	const clrExtradarkgrey = 0xFF171717;
	const clrDarkgrey = 0xFF252525;   
	const clrMidgrey = 0xFF555555;
	const clrGrey = 0xFF808080;        
	const clrLightgrey = 0xFFD3D3D3;    
	const clrWhite = 0xFFFFFFFF;
	const clrLightblue = 0xFFADD8E6;
	const clrBlack = 0xFF000000;  
	const clrKeyPurple = 0xFFCC96FF;
	const start = -Math.PI * 0.75;	
	
	inline function reduced(obj, amount) { return [amount, amount, obj.area[2] - 2 * amount, obj.area[3] - 2 * amount]; }
	
	// Main Slider	
	
	LAFSlider.registerFunction("drawRotarySlider", function(g, obj)
	{
	    var x = obj.area[0];
	    var y = obj.area[1];
	    var w = obj.area[2];
	    var h = obj.area[3];
	
	    // Knob rect (48x48), centered horizontally at the top
	    var wKnb = 48;
	    var hKnb = 48;
	    var xKnb = Math.round((w - wKnb) / 2); // center horizontally
	    var yKnb = 8;                           // top padding
	
	    var areaKnob = [xKnb, yKnb, wKnb, hKnb];	
	    var ringWidth = wKnb / 16;
	    
	
	    // Background
	    g.setColour(0x33000000);
	    g.fillEllipse(areaKnob);
	
	    // Unfilled Ring
	    
	    var unfilled = Content.createPath();	    
	    unfilled.startNewSubPath(0.5, 1.0);
	    unfilled.addArc([0.0, 0.0, 1.0, 1.0], -Math.PI * 0.75, Math.PI * 0.75);	    	    
	    g.setColour(obj.hover ? 0xFF292929 : 0xFF262626);
	    g.drawPath(unfilled, areaKnob, ringWidth * 2);
	    
	    // Filled Ring
	    var filled = Content.createPath();
	    filled.startNewSubPath(0.0, 0.0);
	   	filled.startNewSubPath(1.0, 1.0);
	    filled.addArc([0.0, 0.0, 1.0, 1.0], start, Math.max(start, start + Math.PI * 1.5 * obj.valueNormalized));
	    g.setColour(obj.hover ? clrOffWhite : Colours.lightgrey);
	    g.drawPath(filled, areaKnob, ringWidth * 1.50);
		    
		// Value Text (Before Rotating)
	    g.drawAlignedText(obj.valueSuffixString, [0, hKnb + 30, w, 20], "centred");
	
	    // Value Line (Rotated)
	    var angle = (1.0 - (obj.valueNormalized - 0.02)) * -1.5 * Math.PI;
	    var pivot = [xKnb + wKnb / 2, yKnb + hKnb / 2];
	    g.rotate(angle, pivot);		   
	    var lineX = .63;
	    var lineY = .83; 
	    g.drawLine(
	        xKnb + wKnb * lineX,	        
	        xKnb + wKnb * lineY,
	        yKnb + hKnb * lineX,
	        yKnb + hKnb * lineY,
	        4
	    );
	});	
	
	// Main Toggle Button
	
	LAFButtonToggle.registerFunction("drawToggleButton", function(g, obj)
	{
	    var x = obj.area[0];
	    var y = obj.area[1];
	    var w = obj.area[2];
	    var h = obj.area[3];
	    var r = 2; // reduce amount in px
	    g.setColour(0x33000000);		    
	    g.fillEllipse(obj.area);
		
		if (obj.value)
		    g.setColour(obj.over ? clrWhite : clrLightgrey);
		else
			g.setColour(obj.over ? clrGrey : clrMidgrey);
		g.fillEllipse([x + r, y + r, w - (2*r), h - (2*r)]);
		
	});	
	
	// Menu Buttons
	
	LAFButtonMenu.registerFunction("drawToggleButton", function(g, obj)
	{
		if (obj.value)
		    g.setColour(obj.over ? clrWhite : clrLightgrey);
		else
			g.setColour(obj.over ? clrLightgrey : clrGrey);
			//g.setColour(obj.over ? clrGrey : clrMidgrey);
			
		g.drawAlignedText(obj.text, obj.area, "centred");
	});
		
		
	
	
	// Look And Feel Assignment
	const knbLookAndFeel = [Content.getComponent("knbInputGain"), Content.getComponent("knbGateThreshold"), Content.getComponent("knbTranspose"), Content.getComponent("knbOctave"), Content.getComponent("knbChugStrength"), Content.getComponent("knbPickStrength"), Content.getComponent("knbWhistle"), Content.getComponent("knbOutputGain"), Content.getComponent("knbLofi"), Content.getComponent("knbOverdriveMode"),  Content.getComponent("knbOverdriveDrive"),  Content.getComponent("knbOverdriveTone"),  Content.getComponent("knbOverdriveBits"),  Content.getComponent("knbOverdriveSRReduction"),  Content.getComponent("knbOverdriveFoldAmount"),  Content.getComponent("knbOverdriveMix"),  Content.getComponent("knbOverdriveOutputGain"), Content.getComponent("knbAmpMode"), Content.getComponent("knbAmpInput"), Content.getComponent("knbAmpLow"), Content.getComponent("knbAmpMid"), Content.getComponent("knbAmpHigh"), Content.getComponent("knbAmpPresence"), Content.getComponent("knbAmpOutput"), Content.getComponent("knbCabAAxis"), Content.getComponent("knbCabADistance"), Content.getComponent("knbCabADelay"), Content.getComponent("knbCabAPan"), Content.getComponent("knbCabAGain"), Content.getComponent("knbCabBAxis"), Content.getComponent("knbCabBDistance"), Content.getComponent("knbCabBDelay"), Content.getComponent("knbCabBPan"), Content.getComponent("knbCabBGain"), Content.getComponent("knbCabMix"),	    Content.getComponent("knbReverbMix"),  Content.getComponent("knbReverbPreDelay"),  Content.getComponent("knbReverbRoomSize"),  Content.getComponent("knbReverbDecay"),  Content.getComponent("knbReverbDampingFrequency"),  Content.getComponent("knbReverbChorusDepth"),      Content.getComponent("knbDelayMix"),  Content.getComponent("knbDelayMode"),  Content.getComponent("knbDelayDelayTime"),  Content.getComponent("knbDelayDelayTimeSynced"),  Content.getComponent("knbDelayFeedback"),  Content.getComponent("knbDelayModulation"),  Content.getComponent("knbDelayStereoWidth"),  Content.getComponent("knbDelayDamping"), Content.getComponent("knbChorusMix"),  Content.getComponent("knbChorusRate"),  Content.getComponent("knbChorusDepth"),  Content.getComponent("knbChorusTone"),  Content.getComponent("knbChorusVoices"),  Content.getComponent("knbChorusFeedback"),  Content.getComponent("knbChorusDelayTime"), 	                             Content.getComponent("knbRingmodMix"),  Content.getComponent("knbRingmodFrequency"),  Content.getComponent("knbRingmodDepth"),  Content.getComponent("knbRingmodMode"),  Content.getComponent("knbRingmodLFORate"),  Content.getComponent("knbRingmodLFODepth"),  Content.getComponent("knbRingmodFilterFrequency"), ];	
	const btnToggleLookAndFeel = [Content.getComponent("btnGate"), Content.getComponent("btnTranspose"), Content.getComponent("btnTransposeSnap"), Content.getComponent("btnOctave"), Content.getComponent("btnChug"), Content.getComponent("btnPick"), Content.getComponent("btnWhistle"), Content.getComponent("btnLimiter"), Content.getComponent("btnLofi")];
	const btnMenu = [Content.getComponent("btnShowPreferences"), Content.getComponent("btnShowTuner"), Content.getComponent("btnShowClick"), Content.getComponent("btnShowPresetBrowser"), Content.getComponent("btnPresetPrev"), Content.getComponent("btnPresetNext"), ];
	
	for (k in knbLookAndFeel) { k.setLocalLookAndFeel(LAFSlider); }	    
	for (b in btnToggleLookAndFeel) { b.setLocalLookAndFeel(LAFButtonToggle); }
	for (b in btnMenu) { b.setLocalLookAndFeel(LAFButtonMenu); }

}
