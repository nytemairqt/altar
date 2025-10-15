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
    // Output Gain
	const outputGain = Synth.getEffect("outputGain");
    const knbOutputGain = Content.getComponent("knbOutputGain");

    // Postprocess
    const btnShowPostProcess = Content.getComponent("btnShowPostProcess");
    const pnlPostProcess = Content.getComponent("pnlPostProcess");
    
    const bounds = [150, 250, 800, 450];

    inline function onbtnShowPostProcessControl(component, value)
    {
        pnlPostProcess.set("visible", value);
    }

    btnShowPostProcess.setControlCallback(onbtnShowPostProcessControl);

    pnlPostProcess.setPaintRoutine(function(g)
    {
        g.setColour(Colours.withAlpha(Colours.black, 1.0));
        g.fillRoundedRectangle(bounds, 2.0);
    });

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
    
    // Whistle
    const whistle = Synth.getEffect("whistle");
    const knbEQWhistle = Content.getComponent("knbEQWhistle");

    inline function onknbEQWhistleControl(component, value)
    {
        local A = 0 * whistle.BandOffset + whistle.Gain;    
        local B = 1 * whistle.BandOffset + whistle.Gain;    
        local scaledA = -0.0 - (5.0 * value);
        local scaledB = -0.0 - (8.0 * value);    
        whistle.setAttribute(A, scaledA); 
        whistle.setAttribute(B, scaledB); 
    };

    knbEQWhistle.setControlCallback(onknbEQWhistleControl);    
    
    // Lofi
    const lofi = Synth.getEffect("lofi");
    const knbLofiStrength = Content.getComponent("knbLofiStrength");
    
    inline function expLerp(minF, maxF, t)
    {
        return minF * Math.pow(maxF / minF, t);
    }
    
    inline function onknbLofiStrengthControl(component, value)
    {
	    local A = 0 * lofi.BandOffset + lofi.Freq;
	    local B = 1 * lofi.BandOffset + lofi.Freq;
	    local fHP = expLerp(20.0, 800.0, value);
	    local fLP = expLerp(20000.0, 2000.0, value);
	    
	    lofi.setAttribute(A, fHP);
	    lofi.setAttribute(B, fLP);
	    
    }
    
    knbLofiStrength.setControlCallback(onknbLofiStrengthControl);
    
}