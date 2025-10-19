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

namespace InputChain
{
	const clrGrey = 0xFF808080;       
	const clrWhite = 0xFFFFFFFF;
	const clrExtradarkgrey = 0xFF171717;
	const clrLightgrey = 0xFFD3D3D3; 
	
	const p = Content.createPath();

    // Input Gain
    const inputGain = Synth.getEffect("inputGain");
    const knbInputGain = Content.getComponent("knbInputGain");

    // Gate
    const gate = Synth.getEffect("gate");
    const btnGate = Content.getComponent("btnGate");
    const knbGateThreshold = Content.getComponent("knbGateThreshold");

    // Preprocess
    const pnlPreProcess = Content.getComponent("pnlPreProcess");
    const btnShowPreProcess = Content.getComponent("btnShowPreProcess");    
    
   	const bounds = [260, 190, 700, 452];

    inline function onbtnShowPreProcessControl(component, value)
    {
        pnlPreProcess.set("visible", value);
    }
    
    btnShowPreProcess.setControlCallback(onbtnShowPreProcessControl);

    pnlPreProcess.setPaintRoutine(function(g)
    {
        g.setColour(clrExtradarkgrey);
        g.fillRoundedRectangle(bounds, 2.0);
        g.setColour(clrGrey);
        g.drawRoundedRectangle(bounds, 2.0, 2.0);
        
        var y = Content.getComponent("knbPreprocessHpfFreq").get("y") + 80; // inline yuckiness
        var paths = [PathData.pathHPF, PathData.pathLowShelf, PathData.pathPeak, PathData.pathPeak, PathData.pathPeak, PathData.pathHighShelf, PathData.pathLPF];
        var knbs = [Content.getComponent("knbPreprocessHpfFreq"), Content.getComponent("knbPreprocessLowShelfFreq"), Content.getComponent("knbPreprocessLowMidFreq"), Content.getComponent("knbPreprocessMidFreq"), Content.getComponent("knbPreprocessHighMidFreq"), Content.getComponent("knbPreprocessHighShelfFreq"), Content.getComponent("knbPreprocessLpfFreq")];
        var offset = 23;
        
        g.setColour(clrLightgrey);
        
        for (i=0; i<knbs.length; i++)
        {
	        p.clear();
	        p.loadFromData(paths[i]);
	        g.drawPath(p, [knbs[i].get("x") + offset, y, 20, 10], 3.0);
        }                
    });

    pnlPreProcess.setMouseCallback(function(event)
    {
        var x = bounds[0];
        var y = bounds[1];
        var w = bounds[2];
        var h = bounds[3];
        
        if (event.mouseDownX < x || event.mouseDownX > (x + w) || event.mouseDownY < y || event.mouseDownY > (y + h)) 
        {
            btnShowPreProcess.setValue(0);
            btnShowPreProcess.changed();
        }   
    });

    // Pitch-Shifter
    const transpose = Synth.getEffect("transpose");
    const knbTranspose = Content.getComponent("knbTranspose");
    const btnTranspose = Content.getComponent("btnTranspose");
    const btnTransposeSnap = Content.getComponent("btnTransposeSnap");

    inline function onknbTransposeControl(component, value)
    {                    
        local newPitch = Math.pow(2.0, value / 12.0);               
        transpose.setAttribute(transpose.FreqRatio, newPitch);
    }

    inline function onbtnTransposeSnapControl(component, value)
    {
        knbTranspose.set("stepSize", value ? 1.0 : 0.01);
    }

    knbTranspose.setControlCallback(onknbTransposeControl);
    btnTransposeSnap.setControlCallback(onbtnTransposeSnapControl);    

    // Octave
    const octave = Synth.getEffect("octave");
    const btnOctave = Content.getComponent("btnOctave");
    const knbOctave = Content.getComponent("knbOctave");
    
}