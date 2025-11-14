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

namespace Reverb
{   
    const controls = [Content.getComponent("knbReverbMix"), Content.getComponent("knbReverbPreDelay"), Content.getComponent("knbReverbRoomSize"), Content.getComponent("knbReverbDecay"), Content.getComponent("knbReverbDampingFrequency"), Content.getComponent("knbReverbChorusDepth")];
    const fxSlots = [Synth.getSlotFX("modularA"), Synth.getSlotFX("modularB"), Synth.getSlotFX("modularC"), Synth.getSlotFX("modularD"), Synth.getSlotFX("modularE"), Synth.getSlotFX("modularF"), Synth.getSlotFX("modularG")];                
    const pnlReverb = Content.getComponent("pnlReverb");
    const lblReverbBeware = Content.getComponent("lblReverbBeware");

    inline function onControl(component, value)
    {
        local attribute = component.get("text");
        
        for (slot in fxSlots)
            if (slot.getCurrentEffectId() == "reverb")
            {
                local effect = slot.getCurrentEffect();
                local index = effect.getAttributeIndex(attribute);
                effect.setAttribute(index, value);                
            }
            
       if (component == controls[3]) // decay "beware" warning
       {
	       if (value > 0.6) { lblReverbBeware.set("visible", true); }
	       else { lblReverbBeware.set("visible", false); }               		
       }
       
    }   
    
    for (control in controls) { control.setControlCallback(onControl); }    
    
    // Look And Feel
    const pad = 8;
    const bounds = [pad, pad, 850 - pad * 2, 400 - pad * 2];
    pnlReverb.loadImage("{PROJECT_FOLDER}bgReverb.jpg", "bg");
    pnlReverb.loadImage("{PROJECT_FOLDER}trim.png", "trim");
    pnlReverb.setPaintRoutine(function(g)
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