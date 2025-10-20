/*
    Copyright 2025 iamlamprey

    This file is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This file is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with This file. If not, see <http://www.gnu.org/licenses/>.
*/



namespace VuMeter
{				
	const LAFVuInput = Content.createLocalLookAndFeel();		
	const LAFVuOutput = Content.createLocalLookAndFeel();			

	const fltVuMeterInput = Content.getComponent("fltVuMeterInput");
	const fltVuMeterOutput = Content.getComponent("fltVuMeterOutput");
	
	LAFVuInput.loadImage("{PROJECT_FOLDER}bgVuInput.jpg", "bg");
	LAFVuOutput.loadImage("{PROJECT_FOLDER}bgVuOutput.jpg", "bg");
	
	// VU Meters
	LAFVuInput.registerFunction("drawMatrixPeakMeter", function(g, obj)
	{
		var a = obj.area;
		var padL = 10;
		var thickness = a[2] / 2 - (padL * 2);
		var center = a[2] / 2;
		var stripL = [padL , 0, thickness + (padL / 2), obj.area[3]];
		var stripR = [padL + center - (padL / 2), 0, thickness + (padL / 2), obj.area[3]];
		var a1 = [padL + (center * 0), a[3] * (1 - obj.peaks[0]), thickness + (padL / 2), a[3] - a[3] * (1 - obj.peaks[0])];
		var a2 = [padL + (center * 1) - (padL / 2), a[3] * (1 - obj.peaks[1]), thickness + (padL / 2), a[3] - a[3] * (1 - obj.peaks[1])];
		g.drawImage("bg", obj.area, 0, 0);						

		g.setColour(Colours.withAlpha(ColourData.clrDarkgrey, .5));
		g.drawRoundedRectangle(stripL, 0.0, 2.0);
		g.drawRoundedRectangle(stripR, 0.0, 2.0);
		
		// left channel						
		g.setGradientFill([ColourData.clrGrey, 0, a[3], ColourData.clrWhite, 0, a[3] - thickness - (a[3] - thickness) * obj.peaks[0]]);			
		g.fillRoundedRectangle(a1, 0);		
		
		// right channel		
		g.setGradientFill([ColourData.clrGrey, 0, a[3], ColourData.clrWhite, 0, a[3] - thickness - (a[3] - thickness) * obj.peaks[0]]);			
		g.fillRoundedRectangle(a2, 0);
		
		g.setColour(ColourData.clrMidgrey);
		g.drawRect(a, 2.0);									
	});
			
	LAFVuOutput.registerFunction("drawMatrixPeakMeter", function(g, obj)
	{
		var a = obj.area;
		var padL = 10;
		var thickness = a[2] / 2 - (padL * 2);
		var center = a[2] / 2;
		var stripL = [padL , 0, thickness + (padL / 2), obj.area[3]];
		var stripR = [padL + center - (padL / 2), 0, thickness + (padL / 2), obj.area[3]];
		var a1 = [padL + (center * 0), a[3] * (1 - obj.peaks[0]), thickness + (padL / 2), a[3] - a[3] * (1 - obj.peaks[0])];
		var a2 = [padL + (center * 1) - (padL / 2), a[3] * (1 - obj.peaks[1]), thickness + (padL / 2), a[3] - a[3] * (1 - obj.peaks[1])];
		g.drawImage("bg", obj.area, 0, 0);						
		
		// guiding lines
		g.setColour(Colours.withAlpha(ColourData.clrDarkgrey, .5));
		g.drawRoundedRectangle(stripL, 0.0, 2.0);
		g.drawRoundedRectangle(stripR, 0.0, 2.0);
		
		// left channel						
		g.setGradientFill([ColourData.clrGrey, 0, a[3], ColourData.clrWhite, 0, a[3] - thickness - (a[3] - thickness) * obj.peaks[0]]);			
		g.fillRoundedRectangle(a1, 0);		
		
		// right channel		
		g.setGradientFill([ColourData.clrGrey, 0, a[3], ColourData.clrWhite, 0, a[3] - thickness - (a[3] - thickness) * obj.peaks[0]]);			
		g.fillRoundedRectangle(a2, 0);
		
		g.setColour(ColourData.clrMidgrey);
		g.drawRect(a, 2.0);									
	});
	
	fltVuMeterInput.setLocalLookAndFeel(LAFVuInput);
	fltVuMeterOutput.setLocalLookAndFeel(LAFVuOutput);
}
