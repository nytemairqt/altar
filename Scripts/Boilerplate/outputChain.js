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
    const btnPostProcessEQ = Content.getComponent("btnPostProcessEQ");
    const knbPostProcessEQHighFreq = Content.getComponent("knbPostProcessEQHighFreq");
    const knbPostProcessEQHighGain = Content.getComponent("knbPostProcessEQHighGain");
    const knbPostProcessEQHighMidGain = Content.getComponent("knbPostProcessEQHighMidGain");
    const knbPostProcessEQHighMidFreq = Content.getComponent("knbPostProcessEQHighMidFreq");
    const knbPostProcessEQHighMidQ = Content.getComponent("knbPostProcessEQHighMidQ");
    const knbPostProcessEQLowMidFreq = Content.getComponent("knbPostProcessEQLowMidFreq");
    const knbPostProcessEQLowMidGain = Content.getComponent("knbPostProcessEQLowMidGain");
    const knbPostProcessEQLowMidQ = Content.getComponent("knbPostProcessEQLowMidQ");
    const knbPostProcessEQLowFreq = Content.getComponent("knbPostProcessEQLowFreq");
    const knbPostProcessEQLowGain = Content.getComponent("knbPostProcessEQLowGain");
    const knbPostProcessEQHighPass = Content.getComponent("knbPostProcessEQHighPass");
    const knbPostProcessEQLowPass = Content.getComponent("knbPostProcessEQLowPass");
    const btnPostProcessComp = Content.getComponent("btnPostProcessComp");
    const knbPostProcessCompThreshold = Content.getComponent("knbPostProcessCompThreshold");
    const knbPostProcessCompAttack = Content.getComponent("knbPostProcessCompAttack");
    const knbPostProcessCompRelease = Content.getComponent("knbPostProcessCompRelease");
    const knbPostProcessCompRatio = Content.getComponent("knbPostProcessCompRatio");
    const btnPostProcessCompMakeup = Content.getComponent("btnPostProcessCompMakeup");

    inline function onbtnShowPostProcessControl(component, value)
    {
        pnlPostProcess.set("visible", value);
    }

    btnShowPostProcess.setControlCallback(onbtnShowPostProcessControl);

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

    // Chug
    const chug = Synth.getEffect("chug");
    const btnChug = Content.getComponent("btnChug");
    const knbChugThreshold = Content.getComponent("knbChugThreshold");
    const knbChugFreq = Content.getComponent("knbChugFreq");

    inline function onknbChugControl(component, value)
    {
        chug.setAttribute(chug.Threshold, 1-value);
    }

    knbChugThreshold.setControlCallback(onknbChugControl);
}