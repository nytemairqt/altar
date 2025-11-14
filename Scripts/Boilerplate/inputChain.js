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
	const p = Content.createPath();

    const inputGain = Synth.getEffect("inputGain");
    const knbInputGain = Content.getComponent("knbInputGain");

    const gate = Synth.getEffect("gate");
    const btnGate = Content.getComponent("btnGate");
    const knbGateThreshold = Content.getComponent("knbGateThreshold");

    const pnlPreProcess = Content.getComponent("pnlPreProcess");
    const btnShowPreProcess = Content.getComponent("btnShowPreProcess");    
    
   	const bounds = [260, 190, 700, 452];

    inline function onbtnShowPreProcessControl(component, value)
    {
        pnlPreProcess.set("visible", value);
    }
    
    btnShowPreProcess.setControlCallback(onbtnShowPreProcessControl);
        
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

    const transpose = Synth.getEffect("transpose");    
    const btnTranspose = Content.getComponent("btnTranspose");
    const knbTranspose = Content.getComponent("knbTranspose");    
    const btnTransposeSnap = Content.getComponent("btnTransposeSnap");         
    
    inline function onbtnTransposeControl(component, value)
    {
		local latency = Engine.getSamplesForMilliSeconds(1); // 1-2 ms general latency in tests		
		
		if (value)
		{			
			local sr = Math.round(Engine.getSampleRate());
			latency = Engine.getSamplesForMilliSeconds(41); // pitch shifter adds quite a bit
		}
		
		transpose.setBypassed(1-value);
		Engine.setLatencySamples(latency);
    }           
    
    btnTranspose.setControlCallback(onbtnTransposeControl);

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

    const octave = Synth.getEffect("octave");
    const btnOctave = Content.getComponent("btnOctave");
    const knbOctave = Content.getComponent("knbOctave");

    // Look and Feel

    pnlPreProcess.loadImage("{PROJECT_FOLDER}bgPreprocess.png", "bg");    
    pnlPreProcess.setPaintRoutine(function(g)
    {
        g.drawImage("bg", [0, 0, this.getWidth(), this.getHeight()], 0, 0);
        g.setColour(Colours.withAlpha(ColourData.clrComponentBGGrey, .8));
        g.fillRoundedRectangle(bounds, 2.0);
        g.setColour(ColourData.clrMidgrey);
        g.drawRoundedRectangle(bounds, 2.0, 3.0);
                
        var y = Content.getComponent("knbPreprocessHpfFreq").get("y") + 80; // inline yuckiness
        var paths = [PathData.pathHPF, PathData.pathLowShelf, PathData.pathPeak, PathData.pathPeak, PathData.pathPeak, PathData.pathHighShelf, PathData.pathLPF];
        var knbs = [Content.getComponent("knbPreprocessHpfFreq"), Content.getComponent("knbPreprocessLowShelfFreq"), Content.getComponent("knbPreprocessLowMidFreq"), Content.getComponent("knbPreprocessMidFreq"), Content.getComponent("knbPreprocessHighMidFreq"), Content.getComponent("knbPreprocessHighShelfFreq"), Content.getComponent("knbPreprocessLpfFreq")];
        var offset = 23;
        g.setColour(ColourData.clrLightgrey);        
        for (i=0; i<knbs.length; i++)
        {
            p.clear();
            p.loadFromData(paths[i]);
            g.drawPath(p, [knbs[i].get("x") + offset, y, 20, 10], 3.0);
        }                
    });
    
}