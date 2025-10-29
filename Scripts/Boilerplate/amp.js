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
    const controls = [Content.getComponent("knbAmpMode"), Content.getComponent("knbAmpInput"), Content.getComponent("knbAmpLow"), Content.getComponent("knbAmpMid"), Content.getComponent("knbAmpHigh"), Content.getComponent("knbAmpPresence"), Content.getComponent("knbAmpOutput")];    
    const fxSlots = [Synth.getSlotFX("modularA"), Synth.getSlotFX("modularB"), Synth.getSlotFX("modularC"), Synth.getSlotFX("modularD"), Synth.getSlotFX("modularE"), Synth.getSlotFX("modularF"), Synth.getSlotFX("modularG")];         
    const pnlAmp = Content.getComponent("pnlAmp");
    const pnlAmpNAMLoader = Content.getComponent("pnlAmpNAMLoader");      
    const btnAmpNAMLoaderPrev = Content.getComponent("btnAmpNAMLoaderPrev");
    const btnAmpNAMLoaderNext = Content.getComponent("btnAmpNAMLoaderNext");    
    const btnAmpBrowseNAMTones = Content.getComponent("btnAmpBrowseNAMTones");
    const grm = Engine.getGlobalRoutingManager();  
    const namCable = grm.getCable("nam");
    const pnlTooltip = Content.getComponent("pnlTooltip"); 

    inline function onControl(component, value)
    {
        local attribute = component.get("text");  
        
        for (slot in fxSlots)
            if (slot.getCurrentEffectId() == "amp")
            {
                local effect = slot.getCurrentEffect();
                local index = effect.getAttributeIndex(attribute);
                effect.setAttribute(index, value);                
            }
            
        // conditional UI changes
        if (attribute == "Mode")
        {
            if (value == 2) {pnlAmpNAMLoader.set("visible", true); btnAmpBrowseNAMTones.set("visible", true); }
            else { pnlAmpNAMLoader.set("visible", false); btnAmpBrowseNAMTones.set("visible", false); }         
        }
    }   
    
    for (control in controls) { control.setControlCallback(onControl); }

    // NAM Loader        
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
                if (!result || result.toString(0) == "") { return; }
                pnlAmpNAMLoader.set("text", result.toString(0));
                sendNAMCableData();
                pnlAmpNAMLoader.repaint();
            });
        }       
       	else if (event.hover) { pnlTooltip.set("text", this.get("tooltip")); }       	 
    }    

    inline function onbtnAmpNAMCycleControl(component, value)
    {
	    if (!value) { return; }        

        local path = pnlAmpNAMLoader.get("text");
        local file = FileSystem.fromAbsolutePath(path);
        local parent = file.getParentDirectory();
        local fileList = FileSystem.findFiles(parent, "*.nam, *.json", false);     

        Console.print(fileList.length);   

        for (i=0; i<fileList.length; i++) { if (fileList[i].toString(0) == path) { local index = i; } }

        if (component == btnAmpNAMLoaderPrev)       
        {
            if (index == 0) { index = fileList.length - 1; }
            else (index -= 1);
        }   
        else
        {
            if (index == fileList.length - 1) { index = 0; }
            else (index += 1);
        }

        pnlAmpNAMLoader.set("text", fileList[index].toString(0));
        sendNAMCableData();
        pnlAmpNAMLoader.repaint();
    }
	
	btnAmpNAMLoaderPrev.setControlCallback(onbtnAmpNAMCycleControl);
	btnAmpNAMLoaderNext.setControlCallback(onbtnAmpNAMCycleControl);
	
    // Look And Feel
    const pad = 8;
    const bounds = [pad, pad, 850 - pad * 2, 400 - pad * 2];                      
            
    pnlAmpNAMLoader.setPaintRoutine(function(g)
    {
       var area = [0, 0, this.getWidth(), this.getHeight()];
       g.setColour(ColourData.clrComponentBGGrey);
       g.fillRoundedRectangle(area, 2.0);
       g.setColour(ColourData.clrDarkgrey);
       g.drawRoundedRectangle(area, 2.0, 3.0); 
       g.setColour(ColourData.clrWhite);
       var text = this.get("text");      
       var index = text.lastIndexOf("\\") + 1;
       var substring = text.substring(index, text.length);
       var pad = 24;
       g.drawAlignedText(substring, [pad, 0, this.getWidth() - pad, this.getHeight()], "left") ;
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
    
    pnlAmp.loadImage("{PROJECT_FOLDER}bgAmp.jpg", "bg");   
    pnlAmp.loadImage("{PROJECT_FOLDER}trim.png", "trim");
    
    pnlAmp.setPaintRoutine(function(g)
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
    
    inline function onbtnAmpBrowseNAMTonesControl(component, value)
    {
	    if (value) { Engine.openWebsite("https://www.tone3000.com/"); }	    
    }
    
    btnAmpBrowseNAMTones.setControlCallback(onbtnAmpBrowseNAMTonesControl);
                
}