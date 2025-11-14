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
    const controls = [Content.getComponent("knbOverdriveMode"), Content.getComponent("knbOverdriveDrive"), Content.getComponent("knbOverdriveTone"), Content.getComponent("knbOverdriveFoldAmount"), Content.getComponent("knbOverdriveBits"), Content.getComponent("knbOverdriveSRReduction"), Content.getComponent("knbOverdriveCircuitBend"), Content.getComponent("btnOverdriveCircuitBendTrigger"), Content.getComponent("knbOverdriveMix"), Content.getComponent("knbOverdriveOutputGain"), Content.getComponent("knbOverdriveCircuitBendFreqHidden"),];    
    const fxSlots = [Synth.getSlotFX("modularA"), Synth.getSlotFX("modularB"), Synth.getSlotFX("modularC"), Synth.getSlotFX("modularD"), Synth.getSlotFX("modularE"), Synth.getSlotFX("modularF"), Synth.getSlotFX("modularG")];        
    const btnOverdriveCircuitBendTrigger = Content.getComponent("btnOverdriveCircuitBendTrigger");        

    inline function onControl(component, value)
    {                
        local attribute = component.get("text");
        
        for (slot in fxSlots)
            if (slot.getCurrentEffectId() == "overdrive")
            {
                local effect = slot.getCurrentEffect();
                local index = effect.getAttributeIndex(attribute);
                effect.setAttribute(index, value);                
            }
            
        // conditional UI changes
        if (attribute == "Mode")
        {    
        	if (value == 3) 
        	{ 
        		controls[1].set("visible", false); // hide drive
        		controls[2].set("visible", false); // hide tone
        		controls[3].set("visible", true); // show wavefold     
        		controls[4].set("visible", false); // hide bits
        		controls[5].set("visible", false); // hide SR reduction
        	}        	
        	else if (value == 4 || value == 5) 
        	{ 
        		controls[1].set("visible", false); // hide drive
        		controls[2].set("visible", false); // hide tone
        		controls[3].set("visible", false); // hide wavefold     
        		controls[4].set("visible", true); // show bits
        		controls[5].set("visible", true); // show SR reduction
        	}
        	else // default
        	{
	        	controls[1].set("visible", true); // show drive
	        	controls[2].set("visible", true); // hide tone
        		controls[3].set("visible", false); // hide wavefold     
        		controls[4].set("visible", false); // hide bits
        		controls[5].set("visible", false); // hide SR reduction
        	}        	
        }
    }   
	
    for (control in controls) { control.setControlCallback(onControl); }
        
    // Look And Feel

    const pad = 8;
    const bounds = [pad, pad, 850 - pad * 2, 400 - pad * 2];    

    const pnlOverdrive = Content.getComponent("pnlOverdrive");
    pnlOverdrive.loadImage("{PROJECT_FOLDER}bgOverdrive.jpg", "bg");
    pnlOverdrive.loadImage("{PROJECT_FOLDER}trim.png", "trim");

    const LAFButtonTriggerCircuitBend = Content.createLocalLookAndFeel();

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

    LAFButtonTriggerCircuitBend.registerFunction("drawToggleButton", function(g, obj)
    {
        if (obj.value) { g.setColour(obj.over ? ColourData.clrWhite : ColourData.clrLightgrey); }
        else { g.setColour(obj.over ? ColourData.clrLightgrey : ColourData.clrGrey); }
        var w = obj.area[2];
        var h = obj.area[3];
        var p = Content.createPath();
        p.loadFromData(PathData.pathLightningBolt);
        g.rotate(Math.toRadians(-20), [w / 2, h / 2]);
        g.fillPath(p, obj.area);
    });
    
    btnOverdriveCircuitBendTrigger.setLocalLookAndFeel(LAFButtonTriggerCircuitBend);
    
}