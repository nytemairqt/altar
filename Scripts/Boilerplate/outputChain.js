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

    const btnShowPostprocess = Content.getComponent("btnShowPostprocess");
    const pnlPostprocess = Content.getComponent("pnlPostprocess");
    Engine.addModuleStateToUserPreset("postprocessEQ");
        
    const btnLimiter = Content.getComponent("btnLimiter");
    const limiterDCBlocker = Synth.getEffect("limiterDCBlocker");
    const limiter = Synth.getEffect("limiter");           
        
    inline function onbtnShowPostprocessControl(component, value)
    {
        pnlPostprocess.set("visible", value);
    }

    btnShowPostprocess.setControlCallback(onbtnShowPostprocessControl);           
    
    pnlPostprocess.setMouseCallback(function(event)
    {
        var x = bounds[0];
        var y = bounds[1];
        var w = bounds[2];
        var h = bounds[3];
        
        if (event.mouseDownX < x || event.mouseDownX > (x + w) || event.mouseDownY < y || event.mouseDownY > (y + h)) 
        {
            btnShowPostprocess.setValue(0);
            btnShowPostprocess.changed();
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
    
    inline function onbtnLimiterControl(component, value)
    {
	    limiter.setBypassed(1-value);
	    limiterDCBlocker.setBypassed(1-value);
    }
    
    btnLimiter.setControlCallback(onbtnLimiterControl);

    // Look and feel
    
    const padLeft = 154;
    const padTop = 50;
    const padRight = 308;
    const padBottom = 57;
    
    const bounds = [padLeft, padTop, pnlPostprocess.getWidth() - padRight, pnlPostprocess.getHeight() - padBottom];

    pnlPostprocess.setPaintRoutine(function(g)
    {        
        g.setColour(Colours.withAlpha(ColourData.clrComponentBGGrey, 1.0));
        g.fillRoundedRectangle(bounds, 2.0);
        g.setColour(ColourData.clrMidgrey);
        g.drawRoundedRectangle(bounds, 2.0, 3.0);                        
    });
    
}