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

namespace InputChain
{
    const inputGain = Synth.getEffect("inputGain");
    const knbInputGain = Content.getComponent("knbInputGain");    

    const transpose = Synth.getEffect("transpose");    
    const btnTranspose = Content.getComponent("btnTranspose");
    const knbTranspose = Content.getComponent("knbTranspose");    
    const btnTransposeSnap = Content.getComponent("btnTransposeSnap");         
    
    inline function onbtnTransposeControl(component, value)
    {
		local latency = Engine.getSamplesForMilliSeconds(1); // 1-2 ms general latency in tests		
		
		if (value)
		{			
			local sr = Math.round(Engine.getSampleRate());
			latency = Engine.getSamplesForMilliSeconds(41); // pitch shifter adds quite a bit
		}
		
		transpose.setBypassed(1-value);
		Engine.setLatencySamples(latency);
    }           
    
    btnTranspose.setControlCallback(onbtnTransposeControl);

    inline function onknbTransposeControl(component, value)
    {                    
        local newPitch = Math.pow(2.0, value / 12.0);               
        transpose.setAttribute(transpose.FreqRatio, newPitch);
    }

    inline function onbtnTransposeSnapControl(component, value)
    {
        knbTranspose.set("stepSize", value ? 1.0 : 0.01);
    }

    knbTranspose.setControlCallback(onknbTransposeControl);
    btnTransposeSnap.setControlCallback(onbtnTransposeSnapControl);    

    const octave = Synth.getEffect("octave");
    const btnOctave = Content.getComponent("btnOctave");
    const knbOctave = Content.getComponent("knbOctave");    
    
}