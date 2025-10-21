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

namespace Overdrive
{            
    // Module Control

    const controls = [Content.getComponent("knbOverdriveMode"), Content.getComponent("knbOverdriveDrive"), Content.getComponent("knbOverdriveCircuitBend"), Content.getComponent("knbOverdriveBits"), Content.getComponent("knbOverdriveSRReduction"), Content.getComponent("knbOverdriveFoldAmount"), Content.getComponent("knbOverdriveMix"), Content.getComponent("knbOverdriveOutputGain")];    
    const fxSlots = [Synth.getSlotFX("modularA"), Synth.getSlotFX("modularB"), Synth.getSlotFX("modularC"), Synth.getSlotFX("modularD"), Synth.getSlotFX("modularE"), Synth.getSlotFX("modularF"), Synth.getSlotFX("modularG")];        

    inline function onControl(component, value)
    {
        local text = component.get("text");
        local idx = text.indexOf("_");
        local mod = text.substring(0, idx);
        local param = text.substring(idx + 1, text.length);        
        
        for (slot in fxSlots)
            if (slot.getCurrentEffectId() == mod)
            {
                local effect = slot.getCurrentEffect();
                local index = effect.getAttributeIndex(param);
                effect.setAttribute(index, value);                
            }
            
        // conditional UI changes
        if (text == "overdrive_Mode")
        {    
            if (value == 3) {controls[4].set("visible", false); controls[5].set("visible", false); controls[6].set("visible", true); }           
            else if (value == 4 || value == 5) {controls[4].set("visible", true); controls[5].set("visible", true); controls[6].set("visible", false); }             
            else {controls[4].set("visible", false); controls[5].set("visible", false); controls[6].set("visible", false); }
        }
    }   
	
    for (control in controls) { control.setControlCallback(onControl); }
        

    // Look And Feel

    const pad = 8;
    const bounds = [pad, pad, 850 - pad * 2, 400 - pad * 2];    

    const pnlOverdrive = Content.getComponent("pnlOverdrive");
    pnlOverdrive.loadImage("{PROJECT_FOLDER}bgOverdrive.jpg", "bg");
    pnlOverdrive.loadImage("{PROJECT_FOLDER}trim.png", "trim");

    pnlOverdrive.setPaintRoutine(function(g)
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