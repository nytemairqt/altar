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

namespace Oversampling
{
	
	const fxSlots = [
	        Synth.getSlotFX("modularA"),
	        Synth.getSlotFX("modularB"),
	        Synth.getSlotFX("modularC"),
	        Synth.getSlotFX("modularD"),
	        Synth.getSlotFX("modularE"),
	        Synth.getSlotFX("modularF"),
	        Synth.getSlotFX("modularG")
	    ];

	const cmbOversampling = Content.getComponent("cmbOversampling");
	const cmbOversamplingLAF = Content.createLocalLookAndFeel();
	
	inline function oncmbOversamplingControl(component, value)
	{
		for (slot in fxSlots)
			if (slot.getCurrentEffectId() == "amp" || slot.getCurrentEffectId() == "overdrive")
			{
				local effect = slot.getCurrentEffect();
				effect.setAttribute(effect.Oversampling, value-1.0);				
			}		
	}
	
	cmbOversampling.setControlCallback(oncmbOversamplingControl);

	// Look and feel
	
	cmbOversamplingLAF.registerFunction("drawComboBox", function(g, obj)
    {	        	
	    g.setColour(obj.hover ? ColourData.clrMidgrey : ColourData.clrDarkgrey);
	    g.fillRoundedRectangle(obj.area, 4.0);	    
	    g.setColour(ColourData.clrWhite);
	    g.drawAlignedText(obj.text, [8, 0, obj.area[2] - 16, obj.area[3]], "left");	    
	    
	    var tXPad = 24;
       	var tYPad = 12;
   		var tX = obj.area[2] - tXPad;
   		var tY = 8;
   		var tW = 16;
   		var tH = tY;	    
	    var tA = [tX, tYPad, tW, tH];
	    g.fillTriangle(tA, Math.toRadians(180));
    });
    
    cmbOversampling.setLocalLookAndFeel(cmbOversamplingLAF);
}