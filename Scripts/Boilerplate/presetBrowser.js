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

namespace PresetBrowser
{		    
    const btnShowPresetBrowser = Content.getComponent("btnShowPresetBrowser");
	const btnPresetPrev = Content.getComponent("btnPresetPrev");
    const btnPresetNext = Content.getComponent("btnPresetNext");
    const pnlPresetBrowser = Content.getComponent("pnlPresetBrowser");
    const fltPresetBrowser = Content.getComponent("fltPresetBrowser");
    const knbGateThreshold = Content.getComponent("knbGateThreshold");
    const pnlCabALoader = Content.getComponent("pnlCabALoader");
    const pnlCabBLoader = Content.getComponent("pnlCabBLoader");
    const presetHandler = Engine.createUserPresetHandler();    
    const fxSlots = [Synth.getSlotFX("modularA"), Synth.getSlotFX("modularB"), Synth.getSlotFX("modularC"), Synth.getSlotFX("modularD"), Synth.getSlotFX("modularE"), Synth.getSlotFX("modularF"), Synth.getSlotFX("modularG")];    
    const bounds = [0, 50, 1150, 650];


    reg gateValue = knbGateThreshold.get("defaultValue");
    
    presetHandler.setPreCallback(function(presetData)
    {
	   gateValue = knbGateThreshold.getValue(); 
    });
    
    presetHandler.setPostCallback(function(presetData)
    {
		if (!presetHandler.isInternalPresetLoad())
		{
		    knbGateThreshold.setValue(gateValue);
		    knbGateThreshold.changed();		    
	    }

        for (slot in fxSlots)
        {
            if (slot.getCurrentEffectId() == "cab")
            {
                var id = slot.getCurrentEffect().getId();
                var ref = Synth.getAudioSampleProcessor(id);
                var cabAFile = ref.getAudioFile(0).getCurrentlyLoadedFile(); var cabAText = cabAFile;
                var cabBFile = ref.getAudioFile(1).getCurrentlyLoadedFile(); var cabBText = cabBFile;
                                
                if (cabAFile.contains("{PROJECT_FOLDER}"))
                {
                    var cabAStart = cabAFile.indexOf("}") + 1;
                    var cabAEnd = cabAFile.length;
                    cabAText = cabAFile.substring(cabAStart, cabAEnd);
                }   
                else if (cabAFile.contains("\\")) 
                {
                    var cabAStart = cabAFile.lastIndexOf("\\") + 1;
                    var cabAEnd = cabAFile.length;
                    cabAText = cabAFile.substring(cabAStart, cabAEnd);
                }
                if (cabBFile.contains("{PROJECT_FOLDER}"))
                {
                    var cabBStart = cabBFile.indexOf("}") + 1;
                    var cabBEnd = cabBFile.length;
                    cabBText = cabBFile.substring(cabBStart, cabBEnd);
                } 
                else if (cabBFile.contains("\\")) 
                {
                    var cabBStart = cabBFile.lastIndexOf("\\") + 1;
                    var cabBEnd = cabBFile.length;
                    cabBText = cabBFile.substring(cabBStart, cabBEnd);
                }

                pnlCabALoader.set("text", cabAText); pnlCabALoader.repaint();
                pnlCabBLoader.set("text", cabBText); pnlCabBLoader.repaint();                
            }
        }
    });
            
	pnlPresetBrowser.setMouseCallback(function(event)
    {
        var x = bounds[0];
        var y = bounds[1];
        var w = bounds[2];
        var h = bounds[3];
        
        if (event.mouseDownX < x || event.mouseDownX > (x + w) || event.mouseDownY < y || event.mouseDownY > (y + h)) 
        {
            btnShowPresetBrowser.setValue(0);
            btnShowPresetBrowser.changed();
        }   
    });
    
    inline function onbtnShowPresetBrowserControl(component, value)
    {
	    pnlPresetBrowser.set("visible", value);
    }
    
    btnShowPresetBrowser.setControlCallback(onbtnShowPresetBrowserControl);
    
    inline function onbtnPresetCycleControl(component, value)
    {
		if (!value) { return; } 

	    switch (component)
	    {
		    case btnPresetPrev: Engine.loadPreviousUserPreset(true); break;
		    case btnPresetNext: Engine.loadNextUserPreset(true); break;
	    }
    }
    
    btnPresetPrev.setControlCallback(onbtnPresetCycleControl);
    btnPresetNext.setControlCallback(onbtnPresetCycleControl);
    
    
    
}