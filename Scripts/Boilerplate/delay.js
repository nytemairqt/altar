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

namespace Delay
{   
    const var controls = [Content.getComponent("knbDelayMode"), Content.getComponent("knbDelayMix"), Content.getComponent("knbDelayDelayTime"), Content.getComponent("knbDelayDelayTimeSynced"), Content.getComponent("knbDelayFeedback"), Content.getComponent("knbDelayStereoWidth"), Content.getComponent("knbDelayModulation"), Content.getComponent("knbDelayDamping"), Content.getComponent("btnDelayTempoSync"), Content.getComponent("knbDelayGlitchMode")];
    const fxSlots = [Synth.getSlotFX("modularA"), Synth.getSlotFX("modularB"), Synth.getSlotFX("modularC"), Synth.getSlotFX("modularD"), Synth.getSlotFX("modularE"), Synth.getSlotFX("modularF"), Synth.getSlotFX("modularG")];        
    const pnlDelay = Content.getComponent("pnlDelay");    

    inline function onControl(component, value)
    {
        local attribute = component.get("text");
        
        for (slot in fxSlots)
            if (slot.getCurrentEffectId() == "delay")
            {
                local effect = slot.getCurrentEffect();
                local index = effect.getAttributeIndex(attribute);
                effect.setAttribute(index, value);                
            }

        // conditional UI changes
        if (attribute == "TempoSync") { controls[2].set("visible", 1-value); controls[3].set("visible", value); }
        if (attribute == "DelayMode")
        {
	        if (value == 2) { controls[6].set("visible", false); controls[9].set("visible", true); }
	        else { controls[6].set("visible", true); controls[9].set("visible", false); }
        }
    }   
    
    for (control in controls) { control.setControlCallback(onControl); }

    // Look And Feel
    const pad = 8;
    const bounds = [pad, pad, 850 - pad * 2, 400 - pad * 2];
    pnlDelay.loadImage("{PROJECT_FOLDER}bgDelay.jpg", "bg");
    pnlDelay.loadImage("{PROJECT_FOLDER}trim.png", "trim");
    pnlDelay.setPaintRoutine(function(g)
    {               
        var stripHeight = 140;
        g.drawImage("bg", bounds, 0, 0);
        g.drawImage("trim", bounds, 0, 0);
        g.setColour(ColourData.clrComponentBGGrey);
        g.fillRoundedRectangle([pad, this.getHeight() / 2 - (stripHeight / 2), this.getWidth() - pad * 2, stripHeight], 2.0);
        g.setColour(ColourData.clrDarkgrey);
        g.drawRoundedRectangle(bounds, 0.0, 3.0);                
        g.drawRoundedRectangle([pad, this.getHeight() / 2 - (stripHeight / 2), this.getWidth() - pad * 2, stripHeight], 2.0, 2.0);
    });
}