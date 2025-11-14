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

namespace Ringmod
{   
    const controls = [Content.getComponent("knbRingmodMode"), Content.getComponent("knbRingmodMix"), Content.getComponent("knbRingmodFrequency"), Content.getComponent("knbRingmodDepth"), Content.getComponent("knbRingmodLFORate"), Content.getComponent("knbRingmodLFORateSynced"), Content.getComponent("knbRingmodLFODepth"), Content.getComponent("knbRingmodFilterFrequency"), Content.getComponent("btnRingmodTempoSync"), Content.getComponent("btnRingmodStereoMode")];

    const fxSlots = [Synth.getSlotFX("modularA"), Synth.getSlotFX("modularB"), Synth.getSlotFX("modularC"), Synth.getSlotFX("modularD"), Synth.getSlotFX("modularE"), Synth.getSlotFX("modularF"), Synth.getSlotFX("modularG")];            
    const pnlRingmod = Content.getComponent("pnlRingmod");

    inline function onControl(component, value)
    {
        local attribute = component.get("text");        
        for (slot in fxSlots)
            if (slot.getCurrentEffectId() == "ringmod")
            {
                local effect = slot.getCurrentEffect();
                local index = effect.getAttributeIndex(attribute);
                effect.setAttribute(index, value);                
            }
        if (attribute == "TempoSync") { controls[4].set("visible", 1-value); controls[5].set("visible", value); }
    }   
    
    for (control in controls) { control.setControlCallback(onControl); }
        
    // Look And Feel
    const pad = 8;
    const bounds = [pad, pad, 850 - pad * 2, 400 - pad * 2];
    pnlRingmod.loadImage("{PROJECT_FOLDER}bgRingmod.jpg", "bg");
    pnlRingmod.loadImage("{PROJECT_FOLDER}trim.png", "trim");
    pnlRingmod.setPaintRoutine(function(g)
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