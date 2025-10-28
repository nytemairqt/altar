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
	const btnWitness = Content.getComponent("btnWitness");		
	
	const witnessString = "dj&#fhfa))-118jkgfajge7g7*&JJFJK((9fsauifa7*F&*F837uUSJGKgs9g997da97ggd7gs7gs7&G*&&D*G*&*&gdiagdg87&(&--===F=G*SD8g9dgdigdsgG*G8d8g2gQ!!8e9g89sgd8))SF=++)hgejg7*&T*#jheahgjdhag&#*R&&&t7t32t78dstd78s7td87z7t7&FEGAF&SFdglkg2ly8t9g89sg8*G*Dg8ygsdg77ZGg7&&#&T&E(G(D&G7d7g7dg72gi8gd8sgdgzkgkuKgukGugufdusgewg___gdsg898sdg8zg&&&Gdg-_g9asg8-_G8dsgs-648gT";
	reg witnessIndexA = Math.randInt(0, witnessString.length - 24);
	reg witnessIndexB = Math.randInt(0, witnessString.length - 36);
	const witnessTimer = Engine.createTimerObject();
	
	inline function witnessTimerCallback()
	{
		witnessIndexA = Math.randInt(0,  witnessString.length - 12);
		witnessIndexB = Math.randInt(0, witnessString.length - 20);	
		
		if (Math.random() > 0.5)
		{
			btnWitness.set("text", "||              witness...             ||");
			btnWitness.set("tooltip", "||              witness...             ||");	
		}
		else
		{
			btnWitness.set("text", witnessString.substring(witnessIndexA, witnessIndexA + 29));
			btnWitness.set("tooltip", witnessString.substring(witnessIndexB, witnessIndexB + 36));
		}
		
	}
	
	witnessTimer.setTimerCallback(witnessTimerCallback);
	
	btnWitness.set("text", witnessString.substring(witnessIndexA, witnessIndexA + 29));
	btnWitness.set("tooltip", witnessString.substring(witnessIndexB, witnessIndexB + 36));
	
	witnessTimer.startTimer(400);
	
	inline function onbtnWitnessControl(component, value)
	{
		if (!value) { return; }
		Engine.openWebsite("https://iamlamprey.com");
	}
	
	btnWitness.setControlCallback(onbtnWitnessControl);
	
	inline function onbtnShowPreferencesControl(component, value)
	{
		pnlPreferences.set("visible", value);
	};
	
	btnShowPreferences.setControlCallback(onbtnShowPreferencesControl);
	
	// CPU USAGE TIMER
	    
    const cpuUsageTimer = Engine.createTimerObject();
    const lblCpuUsage = Content.getComponent("lblCpuUsage");        
    
    cpuUsageTimer.setTimerCallback(function()
    {
	    lblCpuUsage.set("text", "CPU Usage: " + Math.round(Engine.getCpuUsage()) + "%");
    });
    
    cpuUsageTimer.startTimer(300);
	
	// LOOK AND FEEL
	
	const bounds = [75, 50, 440, 250];		
	
	const start = -Math.PI * 0.75;			
	
	pnlPreferences.setPaintRoutine(function(g)
    {        
        g.setColour(ColourData.clrExtradarkgrey);
        g.fillRoundedRectangle(bounds, 2.0);
        g.setColour(ColourData.clrGrey);
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
    	var isPlugin = Engine.isPlugin();
    	
	    // BG & Text
	    g.setColour(obj.hover ? ColourData.clrMidgrey : ColourData.clrDarkgrey);
	    g.fillRoundedRectangle(obj.area, 4.0);	    
	    g.setColour(ColourData.clrWhite);
	    
	    if (!isPlugin) { g.drawAlignedText(obj.text, [8, 0, obj.area[2] - 16, obj.area[3]], "left"); }	    	
	    else { g.drawAlignedText("Disabled in plugin.", [8, 0, obj.area[2] - 16, obj.area[3]], "left"); }
	    
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
    
    
	
}