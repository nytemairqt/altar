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

namespace OutputChain
{
	const outputGain = Synth.getEffect("outputGain");
    const knbOutputGain = Content.getComponent("knbOutputGain");

    const btnShowPostProcess = Content.getComponent("btnShowPostProcess");
    const pnlPostProcess = Content.getComponent("pnlPostProcess");
    
    inline function onbtnShowPostProcessControl(component, value)
    {
        pnlPostProcess.set("visible", value);
    }

    btnShowPostProcess.setControlCallback(onbtnShowPostProcessControl);           
    
    pnlPostProcess.setMouseCallback(function(event)
    {
        var x = bounds[0];
        var y = bounds[1];
        var w = bounds[2];
        var h = bounds[3];
        
        if (event.mouseDownX < x || event.mouseDownX > (x + w) || event.mouseDownY < y || event.mouseDownY > (y + h)) 
        {
            btnShowPostProcess.setValue(0);
            btnShowPostProcess.changed();
        }   
    });
    
    const whistle = Synth.getEffect("whistle");
    const knbWhistle = Content.getComponent("knbWhistle");

    inline function onknbEQWhistleControl(component, value)
    {
        local A = 0 * whistle.BandOffset + whistle.Gain;    
        local B = 1 * whistle.BandOffset + whistle.Gain;    
        local scaledA = -0.0 - (5.0 * value);
        local scaledB = -0.0 - (8.0 * value);    
        whistle.setAttribute(A, scaledA); 
        whistle.setAttribute(B, scaledB); 
    };

    knbWhistle.setControlCallback(onknbEQWhistleControl);    
    
    const lofi = Synth.getEffect("lofi");
    const knbLofi = Content.getComponent("knbLofi");
    
    inline function expLerp(minF, maxF, t) // helper function to scale freq
    {
        return minF * Math.pow(maxF / minF, t);
    }
    
    inline function onknbLofiControl(component, value)
    {
	    local A = 0 * lofi.BandOffset + lofi.Freq;
	    local B = 1 * lofi.BandOffset + lofi.Freq;
	    local fHP = expLerp(20.0, 800.0, value);
	    local fLP = expLerp(20000.0, 2000.0, value);
	    
	    lofi.setAttribute(A, fHP);
	    lofi.setAttribute(B, fLP);	    
    }
    
    knbLofi.setControlCallback(onknbLofiControl);

    // Look and feel

    const bounds = [190, 190, 700, 452];
    const clrGrey = 0xFF808080;       
    const clrWhite = 0xFFFFFFFF;
    const clrExtradarkgrey = 0xFF171717;
    const clrLightgrey = 0xFFD3D3D3; 
    const p = Content.createPath();     

    pnlPostProcess.loadImage("{PROJECT_FOLDER}bgPostprocess.png", "bg");    

    pnlPostProcess.setPaintRoutine(function(g)
    {
        g.drawImage("bg", [0, 0, this.getWidth(), this.getHeight()], 0, 0);
        g.setColour(Colours.withAlpha(ColourData.clrComponentBGGrey, .8));
        g.fillRoundedRectangle(bounds, 2.0);
        g.setColour(ColourData.clrMidgrey);
        g.drawRoundedRectangle(bounds, 2.0, 3.0);
        
        var y = Content.getComponent("knbPostprocessHpfFreq").get("y") + 80; // inline yuckiness
        var paths = [PathData.pathHPF, PathData.pathLowShelf, PathData.pathPeak, PathData.pathPeak, PathData.pathPeak, PathData.pathHighShelf, PathData.pathLPF];
        var knbs = [Content.getComponent("knbPostprocessHpfFreq"), Content.getComponent("knbPostprocessLowShelfFreq"), Content.getComponent("knbPostprocessLowMidFreq"), Content.getComponent("knbPostprocessMidFreq"), Content.getComponent("knbPostprocessHighMidFreq"), Content.getComponent("knbPostprocessHighShelfFreq"), Content.getComponent("knbPostprocessLpfFreq")];
        var offset = 23;
        
        g.setColour(clrLightgrey);
        
        for (i=0; i<knbs.length; i++)
        {
            p.clear();
            p.loadFromData(paths[i]);
            g.drawPath(p, [knbs[i].get("x") + offset, y, 20, 10], 3.0);
        }                
    });
    
}