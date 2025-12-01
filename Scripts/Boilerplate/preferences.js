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
	const version = "0.3b";

	const btnShowPreferences = Content.getComponent("btnShowPreferences");
	const pnlPreferences = Content.getComponent("pnlPreferences");
	const fltPreferences = Content.getComponent("fltPreferences");	
	const fltPreferencesLAF = Content.createLocalLookAndFeel();	

	const btnWitness = Content.getComponent("btnWitness");		
	const witnessString = "dj&#fhfa))-118jkgfajge7g7*&JJFJK((9fsauifa7*F&*F837uUSJGKgs9g997da97ggd7gs7gs7&G*&&D*G*&*&gdiagdg87&(&--===F=G*SD8g9dgdigdsgG*G8d8g2gQ!!8e9g89sgd8))SF=++)hgejg7*&T*#jheahgjdhag&#*R&&&t7t32t78dstd78s7td87z7t7&FEGAF&SFdglkg2ly8t9g89sg8*G*Dg8ygsdg77ZGg7&&#&T&E(G(D&G7d7g7dg72gi8gd8sgdgzkgkuKgukGugufdusgewg___gdsg898sdg8zg&&&Gdg-_g9asg8-_G8dsgs-648gT";
	const targetWord = "  witness  ";
	const displayLength = 29;
	reg currentDisplayText = "";
	reg frameCounter = 0;		

	const witnessTimer = Engine.createTimerObject();

	inline function applyLeetSpeak(word)
	{
	    local result = "";
	    
	    for (i = 0; i < word.length; i++)
	    {
	        local char = word.substring(i, i + 1);
	        local newChar = char;
	        	        
	        if (char == "i") { if (Math.random() < 0.15) { newChar = "1"; } }
	        else if (char == "e") { if (Math.random() < 0.15) { newChar = "3"; } }
	        else if (char == "s") { if (Math.random() < 0.15) { newChar = "$"; } }
	        
	        result += newChar;
	    }
	    
	    return result;
	}

	inline function initializeText()
	{
	    local tempDisplay = "";
	    
	    for (i = 0; i < displayLength; i++)
	    {
	        local randomIndex = Math.randInt(0, witnessString.length - 1);
	        tempDisplay += witnessString.substring(randomIndex, randomIndex + 1);
	    }

	    currentDisplayText = tempDisplay;
	}

	inline function createCenteredText(length)
	{
	    local result = "";
	    local centerPos = Math.floor((length - targetWord.length) / 2);
	    local modifiedWord = applyLeetSpeak(targetWord);
	    local i;
	    
	    for (i = 0; i < length; i++)
	    {
	        if (i >= centerPos && i < centerPos + targetWord.length)
	        {
	            local wordIndex = i - centerPos;
	            result += modifiedWord.substring(wordIndex, wordIndex + 1);
	        }
	        else
	        {
	            local randomIndex = Math.randInt(0, witnessString.length - 1);
	            result += witnessString.substring(randomIndex, randomIndex + 1);
	        }
	    }
	    
	    return result;
	}

	inline function updateGlitchCharacters(text, length)
	{
	    if (text.length < length) { return text; }
	    
	    local textArray = [];
	    local centerPos = Math.floor((length - targetWord.length) / 2);
	    local i;
	    
	    for (i = 0; i < length; i++) { textArray[i] = text.substring(i, i + 1); }
	    
	    if (Math.random() < 0.1)
	    {
	        local modifiedWord = applyLeetSpeak(targetWord);
	        for (i = 0; i < targetWord.length; i++)
	        {
	            local arrayIndex = centerPos + i;
	            if (arrayIndex >= 0 && arrayIndex < length)
	            {
	                textArray[arrayIndex] = modifiedWord.substring(i, i + 1);
	            }
	        }
	    }
	    
	    local validPositions = [];
	    local validCount = 0;
	    
	    for (i = 0; i < length; i++)
	    {
	        if (i < centerPos || i >= centerPos + targetWord.length)
	        {
	            validPositions[validCount] = i;
	            validCount++;
	        }
	    }
	    
	    if (validCount > 0)
	    {
	        local changeIndex = validPositions[Math.randInt(0, validCount - 1)];
	        local randomIndex = Math.randInt(0, witnessString.length - 1);
	        textArray[changeIndex] = witnessString.substring(randomIndex, randomIndex + 1);
	    }
	    
	    local result = "";
	    for (i = 0; i < length; i++) { result += textArray[i]; }	   
	    return result;
	}

	inline function witnessTimerCallback()
	{
	    frameCounter++;
	    currentDisplayText = updateGlitchCharacters(currentDisplayText, displayLength);
	    btnWitness.set("text", currentDisplayText);	    
	}
	
	currentDisplayText = createCenteredText(displayLength);
	witnessTimer.setTimerCallback(witnessTimerCallback);
	witnessTimer.startTimer(70);

	inline function onbtnWitnessControl(component, value)
	{
	    if (!value) { return; }
	    Engine.openWebsite("https://iamlamprey.com/altar");
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
	    // HISE defaults to a CPU spike reporting system, getCpuUsage() isn't actually showing CPU usage
	    // instead it represents a 512-sample buffer fill amount for 10ms audio @ 44100 
	    // in other words, dividing by 10 gives us a smoother representation of the actual CPU usage of the plugin    	    
	    lblCpuUsage.set("text", "CPU Usage: " + Math.round(Engine.getCpuUsage() / 10) + "%");
    });
    
    cpuUsageTimer.startTimer(300);
	
	// Look and feel
	
	const bounds = [250, 50, 440, 300];		
	
	const start = -Math.PI * 0.75;			
	
	pnlPreferences.setPaintRoutine(function(g)
    {        
        g.setColour(ColourData.clrExtradarkgrey);
        g.fillRoundedRectangle(bounds, 2.0);
        g.setColour(ColourData.clrGrey);
        g.drawRoundedRectangle(bounds, 2.0, 2.0);
        
        g.setColour(Colours.white);
        g.drawAlignedText("Version: " + version, [bounds[0], bounds[3], bounds[2], 20], "centred");
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