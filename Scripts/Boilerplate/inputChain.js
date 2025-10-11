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
    
   	const bounds = [150, 250, 800, 450];

    inline function onbtnShowPreProcessControl(component, value)
    {
        pnlPreProcess.set("visible", value);
    }
    
    btnShowPreProcess.setControlCallback(onbtnShowPreProcessControl);

    pnlPreProcess.setPaintRoutine(function(g)
    {
        g.setColour(Colours.withAlpha(Colours.black, 1.0));
        g.fillRoundedRectangle(bounds, 2.0);
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
    const pitch = Synth.getEffect("pitch");
    const knbPitch = Content.getComponent("knbPitch");
    const btnPitch = Content.getComponent("btnPitch");
    const btnPitchSnap = Content.getComponent("btnPitchSnap");

    inline function onknbPitchControl(component, value)
    {                    
        local newPitch = Math.pow(2.0, value / 12.0);               
        pitch.setAttribute(pitch.FreqRatio, newPitch);
    }

    inline function onbtnPitchSnapControl(component, value)
    {
        knbPitch.set("stepSize", value ? 1.0 : 0.01);
    }

    knbPitch.setControlCallback(onknbPitchControl);
    btnPitchSnap.setControlCallback(onbtnPitchSnapControl);

    // Grit
    const grit = Synth.getEffect("grit");

    // Octave
    const octave = Synth.getEffect("octave");
    const btnOctave = Content.getComponent("btnOctave");
    const knbOctave = Content.getComponent("knbOctave");
    
}