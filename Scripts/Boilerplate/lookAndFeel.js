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
	
	
	const LAFSliderNEAT = Content.createLocalLookAndFeel();
	
	
	/* Colours & Path */
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
	
	/* Utility Functions */
	
	inline function reduced(obj, amount)
	{
	    return [amount, amount, obj.area[2] - 2*amount, obj.area[3] - 2* amount];
	}
	
	// Main Slider
	
	LAFSliderNEAT.registerFunction("drawRotarySlider", function(g, obj)
	{
	    var ringWidth = obj.area[2] / 16;    
	    
	    // Background    
	    g.setColour(0x33000000);
	    g.fillEllipse(reduced(obj, ringWidth * 2.0));
	    
	    // Arc
	    var sliderRing2 = Content.createPath();
	    var sliderRing3 = Content.createPath();
	    sliderRing2.startNewSubPath(0.5, 1.0);
	    sliderRing2.addArc([0.0, 0.0, 1.0, 1.0], -Math.PI*0.75, Math.PI * 0.75);
	    sliderRing3.startNewSubPath(0.0, 0.0);
	    sliderRing3.startNewSubPath(1.0, 1.0);  
	    var start = -Math.PI*0.75;
	
	    // Unfilled ring
	    sliderRing3.addArc([0.0, 0.0, 1.0, 1.0], start, Math.max(start, start + Math.PI * 1.5 * obj.valueNormalized));
	    g.setColour(obj.hover ? 0xFF292929 : 0xFF262626);
	    g.drawPath(sliderRing2, reduced(obj, ringWidth), ringWidth * 2);
	    g.setColour(obj.hover ? clrOffWhite : Colours.lightgrey);
	    g.drawPath(sliderRing3, reduced(obj, ringWidth), ringWidth * (1.6));
	    g.rotate((1.0 - (obj.valueNormalized - 0.02)) * -1.5 * Math.PI, [obj.area[2] / 2, obj.area[3] / 2]);  
	    
	    // Center Ellipse        
	    g.setColour(0xFF1C1C1C);
	    g.fillEllipse(reduced(obj, obj.area[2] * .86));
	
	    // Value line
	    g.setColour(Colours.lightgrey);        
	    g.drawLine(obj.area[2] * .65, obj.area[2] * .83, obj.area[3] * .65, obj.area[3] * .83, 3);     
	});
	
	
	// Look And Feel Assignment
	const var knbLookAndFeel = [Content.getComponent("knbInputGain"),
	                            Content.getComponent("knbGateThreshold"),
	                            Content.getComponent("knbPitch"),
	                            Content.getComponent("knbOctave"),
	                            Content.getComponent("knbChugStrength"),
	                            Content.getComponent("knbPickStrength"),
	                            Content.getComponent("knbGrit"),
	                            Content.getComponent("knbEQWhistle"),
	                            Content.getComponent("knbOutputGain"),  
	                            Content.getComponent("knbAmpMode"),
	                            Content.getComponent("knbAmpInput"),
	                            Content.getComponent("knbAmpLow"),
	                            Content.getComponent("knbAmpMid"),
	                            Content.getComponent("knbAmpHigh"),
	                            Content.getComponent("knbAmpPresence"),
	                            Content.getComponent("knbAmpOutput"),
	                            Content.getComponent("knbCabAAxis"),
	                            Content.getComponent("knbCabADistance"),
	                            Content.getComponent("knbCabADelay"),
	                            Content.getComponent("knbCabAPan"),
	                            Content.getComponent("knbCabAGain"),
	                            Content.getComponent("knbCabBAxis"),
	                            Content.getComponent("knbCabBDistance"),
	                            Content.getComponent("knbCabBDelay"),
	                            Content.getComponent("knbCabBPan"),
	                            Content.getComponent("knbCabBGain"),
	                            Content.getComponent("knbCabMix"),
	                            Content.getComponent("knbReverbMix"),
	                            Content.getComponent("knbReverbFeedback"),
	                            Content.getComponent("knbReverbBrightness"),                            
	                            ];                                                        
	
	
	for (k in knbLookAndFeel)
	    k.setLocalLookAndFeel(LAFSliderNEAT);
}