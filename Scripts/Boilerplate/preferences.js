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

namespace Preferences
{
	const btnShowPreferences = Content.getComponent("btnShowPreferences");
	const pnlPreferences = Content.getComponent("pnlPreferences");
	const fltPreferences = Content.getComponent("fltPreferences");	
	const fltPreferencesLAF = Content.createLocalLookAndFeel();	
	
	const bounds = [75, 50, 440, 250];		
	
	const clrDarkgrey = 0xFF252525;   
	const clrWhite = 0xFFFFFFFF;
	const clrExtradarkgrey = 0xFF171717;
	const clrGrey = 0xFF808080;   
	
	const isPlugin = Engine.isPlugin();
	
	if (isPlugin) { btnShowPreferences.setVisible(false); }
	
	const start = -Math.PI * 0.75;	
	
	inline function onbtnShowPreferencesControl(component, value)
	{
		pnlPreferences.set("visible", value);
	};
	
	btnShowPreferences.setControlCallback(onbtnShowPreferencesControl);
	
	pnlPreferences.setPaintRoutine(function(g)
    {        
        g.setColour(clrExtradarkgrey);
        g.fillRoundedRectangle(bounds, 2.0);
        g.setColour(clrGrey);
        g.drawRoundedRectangle(bounds, 2.0, 2.0);
    });

    pnlPreferences.setMouseCallback(function(event)
    {
        var x = bounds[0];
        var y = bounds[1];
        var w = bounds[2];
        var h = bounds[3];
        
        if (event.mouseDownX < x || event.mouseDownX > (x + w) || event.mouseDownY < y || event.mouseDownY > (y + h)) 
        {
            btnShowPreferences.setValue(0);
            btnShowPreferences.changed();
        }   
    });
    
    fltPreferencesLAF.registerFunction("drawComboBox", function(g, obj)
    {	        	
	    // BG & Text
	    g.setColour(obj.hover ? 0xFF2C2C2C : clrDarkgrey);
	    g.fillRoundedRectangle(obj.area, 4.0);	    
	    g.setColour(clrWhite);
	    g.drawAlignedText(obj.text, [8, 0, obj.area[2] - 16, obj.area[3]], "left");	    
	    
	    // Triangle
	    var tXPad = 24;
       	var tYPad = 12;
   		var tX = obj.area[2] - tXPad;
   		var tY = 8;
   		var tW = 16;
   		var tH = tY;	    
	    var tA = [tX, tYPad, tW, tH];
	    g.fillTriangle(tA, Math.toRadians(180));
    });
    
    fltPreferences.setLocalLookAndFeel(fltPreferencesLAF);
    
    // CPU USAGE TIMER
    
    const cpuUsageTimer = Engine.createTimerObject();
    const lblCpuUsage = Content.getComponent("lblCpuUsage");        
    
    cpuUsageTimer.setTimerCallback(function()
    {
	    lblCpuUsage.set("text", "CPU Usage: " + Math.round(Engine.getCpuUsage()) + "%");
    });
    
    cpuUsageTimer.startTimer(300);
	
}