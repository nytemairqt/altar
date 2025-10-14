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
    const fltPresetBrowser = Content.getComponent("fltPresetBrowser");
    
    inline function onbtnShowPresetBrowserControl(component, value)
    {
	    fltPresetBrowser.set("visible", value);
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