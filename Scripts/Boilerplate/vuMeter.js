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

	// Display scaling:
	// - Top 75% of the meter shows -24 dB to 0 dB
	// - Bottom 25% shows minDb to -24 dB (using dB mapping for nicer distribution)
	const VU_TOP_DB = -24.0; // threshold between bottom and top regions
	const VU_MIN_DB = -60.0; // floor for the meter visualisation

	inline function mapPeakToDisplay(v)
	{
		// v is linear gain in [0..1]
		if (v <= 0.0) return 0.0;

		local thr = Engine.getGainFactorForDecibels(VU_TOP_DB); // ~0.0631 for -24 dB

		if (v >= thr)
		{
			// Map [thr..1] linearly into [0.25..1.0]
			local t0 = (v - thr) / (1.0 - thr);
			return 0.25 + 0.75 * t0;
		}
		else
		{
			// Map [-60 dB .. -24 dB] into [0.0 .. 0.25] using dB for better visual spread
			local db = Engine.getDecibelsForGainFactor(Math.max(v, 0.000001));
			local t1 = (db - VU_MIN_DB) / (VU_TOP_DB - VU_MIN_DB); // 0 at minDb, 1 at -24dB
			t1 = Math.max(0.0, Math.min(1.0, t1));
			return 0.25 * t1;
		}
	}
	
	// VU Meters
	LAFVuInput.registerFunction("drawMatrixPeakMeter", function(g, obj)
	{
		var a = obj.area;
		var padL = 10;
		var thickness = a[2] / 2 - (padL * 2);
		var center = a[2] / 2;
		var stripL = [padL , 0, thickness + (padL / 2), obj.area[3]];
		var stripR = [padL + center - (padL / 2), 0, thickness + (padL / 2), obj.area[3]];

		// Remap peaks for display
		var pL = mapPeakToDisplay(obj.peaks[0]);
		var pR = mapPeakToDisplay(obj.peaks[1]);
		
		Console.print(Engine.getDecibelsForGainFactor(obj.peaks[0]));

		var a1 = [padL + (center * 0), a[3] * (1 - pL), thickness + (padL / 2), a[3] * pL];
		var a2 = [padL + (center * 1) - (padL / 2), a[3] * (1 - pR), thickness + (padL / 2), a[3] * pR];
		
		g.drawImage("bg", obj.area, 0, 0);		
		g.setColour(Colours.withAlpha(ColourData.clrGrey, .15));
		g.drawRoundedRectangle(stripL, 0.0, 2.0);
		g.drawRoundedRectangle(stripR, 0.0, 2.0);
		
		// Left channel						
		g.setGradientFill([ColourData.clrGrey, 0, a[3], ColourData.clrWhite, 0, a[3] - thickness - (a[3] - thickness) * pL]);				
		g.fillRoundedRectangle(a1, 0);		
		
		// Right channel		
		g.setGradientFill([ColourData.clrGrey, 0, a[3], ColourData.clrWhite, 0, a[3] - thickness - (a[3] - thickness) * pR]);			
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

		// Remap peaks for display
		var pL = mapPeakToDisplay(obj.peaks[0]);
		var pR = mapPeakToDisplay(obj.peaks[1]);

		var a1 = [padL + (center * 0), a[3] * (1 - pL), thickness + (padL / 2), a[3] * pL];
		var a2 = [padL + (center * 1) - (padL / 2), a[3] * (1 - pR), thickness + (padL / 2), a[3] * pR];

		g.drawImage("bg", obj.area, 0, 0);						
		
		// guiding lines
		g.setColour(Colours.withAlpha(ColourData.clrGrey, .15));
		g.drawRoundedRectangle(stripL, 0.0, 2.0);
		g.drawRoundedRectangle(stripR, 0.0, 2.0);
		
		// Left channel						
		g.setGradientFill([ColourData.clrGrey, 0, a[3], ColourData.clrWhite, 0, a[3] - thickness - (a[3] - thickness) * pL]);			
		g.fillRoundedRectangle(a1, 0);		
		
		// Right channel		
		g.setGradientFill([ColourData.clrGrey, 0, a[3], ColourData.clrWhite, 0, a[3] - thickness - (a[3] - thickness) * pR]);			
		g.fillRoundedRectangle(a2, 0);
		
		g.setColour(ColourData.clrMidgrey);
		g.drawRect(a, 2.0);									
	});
	
	fltVuMeterInput.setLocalLookAndFeel(LAFVuInput);
	fltVuMeterOutput.setLocalLookAndFeel(LAFVuOutput);
}