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
    const btnPreProcessEQ = Content.getComponent("btnPreProcessEQ");
    const knbPreProcessEQHighFreq = Content.getComponent("knbPreProcessEQHighFreq");
    const knbPreProcessEQHighGain = Content.getComponent("knbPreProcessEQHighGain");
    const knbPreProcessEQHighMidGain = Content.getComponent("knbPreProcessEQHighMidGain");
    const knbPreProcessEQHighMidFreq = Content.getComponent("knbPreProcessEQHighMidFreq");
    const knbPreProcessEQHighMidQ = Content.getComponent("knbPreProcessEQHighMidQ");
    const knbPreProcessEQLowMidFreq = Content.getComponent("knbPreProcessEQLowMidFreq");
    const knbPreProcessEQLowMidGain = Content.getComponent("knbPreProcessEQLowMidGain");
    const knbPreProcessEQLowMidQ = Content.getComponent("knbPreProcessEQLowMidQ");
    const knbPreProcessEQLowFreq = Content.getComponent("knbPreProcessEQLowFreq");
    const knbPreProcessEQLowGain = Content.getComponent("knbPreProcessEQLowGain");
    const knbPreProcessEQHighPass = Content.getComponent("knbPreProcessEQHighPass");
    const knbPreProcessEQLowPass = Content.getComponent("knbPreProcessEQLowPass");
    const btnPreProcessComp = Content.getComponent("btnPreProcessComp");
    const knbPreProcessCompThreshold = Content.getComponent("knbPreProcessCompThreshold");
    const knbPreProcessCompAttack = Content.getComponent("knbPreProcessCompAttack");
    const knbPreProcessCompRelease = Content.getComponent("knbPreProcessCompRelease");
    const knbPreProcessCompRatio = Content.getComponent("knbPreProcessCompRatio");
    const btnPreProcessCompMakeup = Content.getComponent("btnPreProcessCompMakeup");

    inline function onbtnShowPreProcessControl(component, value)
    {
        pnlPreProcess.set("visible", value);
    }

    pnlPreProcess.setPaintRoutine(function(g)
    {
        var bounds = [200, 250, 700, 450];

        g.setColour(Colours.withAlpha(Colours.red, 1.0));
        g.fillRoundedRectangle(bounds, 2.0);
    });

    pnlPreProcess.setMouseCallback(function(event)
    {
        var x = 200;
        var y = 250;
        var w = 700;
        var h = 450;
        
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
    const octavePre = Synth.getEffect("octavePre");
    const octavePost = Synth.getEffect("octavePost");
    const btnOctave = Content.getComponent("btnOctave");
    const btnOctavePosition = Content.getComponent("btnOctavePosition");
    const knbOctave = Content.getComponent("knbOctave");
    const knbOctaveFreq = Content.getComponent("knbOctaveFreq");
    
    inline function onbtnOctaveControl(component, value)
    {
        if (value)
        {
            octavePre.setBypassed(btnOctavePosition.getValue());
            octavePost.setBypassed(1-btnOctavePosition.getValue());
            btnOctavePosition.set("enabled", true);
        }
        else
        {
            octavePre.setBypassed(true);
            octavePost.setBypassed(true);
            btnOctavePosition.set("enabled", false);
        }
    }

    inline function onknbOctaveControl(component, value)
    {
        switch (component)
        {
            case knbOctave:
                //octavePre.setAttribute(octavePre.Mix, value);
                //octavePost.setAttribute(octavePost.Mix, value);
                break;      
            case knbOctaveFreq:
                //octavePre.setAttribute(octavePre.Freq, value);
                //octavePost.setAttribute(octavePost.Freq, value);
            break;
        }
    }

    knbOctave.setControlCallback(onknbOctaveControl);
    knbOctaveFreq.setControlCallback(onknbOctaveControl);
    btnOctave.setControlCallback(onbtnOctaveControl);
}