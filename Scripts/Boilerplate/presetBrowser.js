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
    const presetHandler = Engine.createUserPresetHandler();    
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