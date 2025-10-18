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

namespace Amp
{    
	const clrDarkgrey = 0xFF252525;   
    const clrWhite = 0xFFFFFFFF;
    const clrExtradarkgrey = 0xFF171717;
    const clrGrey = 0xFF808080;   
    const pad = 8;
    const bounds = [pad, pad, 850 - pad * 2, 400 - pad * 2];
	
	const pnlAmp = Content.getComponent("pnlAmp");
    const pnlAmpNAMLoader = Content.getComponent("pnlAmpNAMLoader");  
    const grm = Engine.getGlobalRoutingManager();  
    const namCable = grm.getCable("nam");     
    const knbAmpMode = Content.getComponent("knbAmpMode");
    const fxSlots = [Synth.getSlotFX("modularA"), Synth.getSlotFX("modularB"), Synth.getSlotFX("modularC"), Synth.getSlotFX("modularD"), Synth.getSlotFX("modularE"), Synth.getSlotFX("modularF"), Synth.getSlotFX("modularG")];

    inline function onknbAmpModeControl(component, value)
    {
        for (slot in fxSlots)
            if (slot.getCurrentEffectId() == "amp")
            {
                local effect = slot.getCurrentEffect();
                effect.setAttribute(effect.Mode, value);
            }
            
        if (value < 2) { pnlAmpNAMLoader.set("enabled", false); }
        else { pnlAmpNAMLoader.set("enabled", true); }        
        
    }  
                
    knbAmpMode.setControlCallback(onknbAmpModeControl);
       
    inline function pnlAmpNAMLoaderDrop(f)
    {
        if(f.drop)
        {
            pnlAmpNAMLoader.set("text", f.fileName);
            sendNAMCableData();
            pnlAmpNAMLoader.repaint();
        }
    }        
    
    inline function pnlAmpNAMLoaderClick(event)
    {
        if (event.clicked && event.rightClick)
        {
            // open file browser
            FileSystem.browse(FileSystem.Documents, false, "*.nam, *.json", function(result)
            {
                pnlAmpNAMLoader.set("text", result.toString(0));
                sendNAMCableData();
                pnlAmpNAMLoader.repaint();
            });
        }        
    }
            
    pnlAmpNAMLoader.setPaintRoutine(function(g)
    {
       g.setColour(Colours.white);
       g.drawRoundedRectangle([0, 0, this.getWidth(), this.getHeight()], 0.0, 1.0);
       var text = this.get("text");      
       var index = text.lastIndexOf("\\") + 1;
       var substring = text.substring(index, text.length);
       g.drawAlignedText(substring, [0, 0, this.getWidth(), this.getHeight()], "centred") ;
    });

    inline function sendNAMCableData()
    {
		local path = pnlAmpNAMLoader.get("text");
        local file = FileSystem.fromAbsolutePath(path);
        local json = file.loadAsObject();
        namCable.sendData(json);
    }
    
    inline function onpnlAmpNAMLoaderCallback(component, value)
    {
	    sendNAMCableData();
    }
    
    pnlAmpNAMLoader.setFileDropCallback("All Callbacks", "*.nam, *.json", pnlAmpNAMLoaderDrop);
    pnlAmpNAMLoader.setMouseCallback(pnlAmpNAMLoaderClick);
    pnlAmpNAMLoader.setControlCallback(onpnlAmpNAMLoaderCallback);
    
    pnlAmp.setPaintRoutine(function(g)
    {
	    g.setColour(clrExtradarkgrey);
        g.fillRoundedRectangle(bounds, 32.0);
        g.setColour(clrDarkgrey);
        g.drawRoundedRectangle(bounds, 32.0, 3.0);
    });
        
}