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

namespace Tuner
{		
	const tuner = Synth.getEffect("tuner");
    const btnShowTuner = Content.getComponent("btnShowTuner");
    const btnTunerMonitor = Content.getComponent("btnTunerMonitor");   
    const lblTunerNote = Content.getComponent("lblTunerNote");
    const lblTunerDeviation = Content.getComponent("lblTunerDeviation");
    const pnlTuner = Content.getComponent("pnlTuner");
    const pnlTunerDisplay = Content.getComponent("pnlTunerDisplay");          
    
    reg note = "";
	reg deviation = 0.0;
	reg unit = "C";
	reg inTune = false;
	reg direction = "";
   	
    inline function onbtnShowTunerControl(component, value) { tuner.setBypassed(1-value); pnlTuner.set("visible", value); }        
    btnShowTuner.setControlCallback(onbtnShowTunerControl);
    
    const bounds = [70, 50, 340, 170];

    pnlTuner.setPaintRoutine(function(g)
    {        		
        g.setColour(ColourData.clrExtradarkgrey);
        g.fillRoundedRectangle(bounds, 2.0);
        g.setColour(ColourData.clrGrey);
        g.drawRoundedRectangle(bounds, 2.0, 2.0);
    });

    pnlTuner.setMouseCallback(function(event)
    {
        var x = bounds[0];
        var y = bounds[1];
        var w = bounds[2];
        var h = bounds[3];
        
        if (event.mouseDownX < x || event.mouseDownX > (x + w) || event.mouseDownY < y || event.mouseDownY > (y + h)) 
        {
            btnShowTuner.setValue(0);
            btnShowTuner.changed();
        }
    });
    
    inline function pnlTunerDisplayPaint(g)
    {
	    local lX = 10;	  
	    local pW = 5;  
	    local lH = this.getHeight() / 2;	    	    
	    local targetRect = [this.getWidth() / 2 - lX, this.getHeight() / 2 - lX, lX * 2, lX * 2];
	    local devRect = this.getWidth() * Math.abs(deviation / 100);
	    local pitchRect = [];
	    
	    if (deviation > 0)
	    	pitchRect = [(this.getWidth() / 2 + devRect) - pW, this.getHeight() / 2 - pW, pW * 2, pW * 2];	    	   
	    else
	    	pitchRect = [(this.getWidth() / 2 - devRect) - pW, this.getHeight() / 2 - pW, pW * 2, pW * 2];	    	   	
	    	       
   	    g.setColour(ColourData.clrLightgrey);
   	    g.drawLine(lX, this.getWidth() - lX, lH, lH, 2.0);   
   	    g.setColour(ColourData.clrExtradarkgrey);   	    
   	    g.fillRoundedRectangle(targetRect, 2.0);
   	    g.setColour(ColourData.clrLightgrey);
   	    g.drawRoundedRectangle(targetRect, 2.0, 2.0);
   	       	    
   	    g.setColour(detectedInTune ? ColourData.clrLightblue : ColourData.clrLightgrey);
   	    g.fillRoundedRectangle(pitchRect, 2.0);   	       	    
    }
    
    pnlTunerDisplay.setPaintRoutine(pnlTunerDisplayPaint);

    // Pitch Detection
    const NOTES = ["C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"];
    var referenceTuning = 440; // might include a 432 button or something
    var tuningMode = 'cents'; // 'cents' or 'hz', might not even implement a hz toggle

    const MIN_FREQ = 40.0;   // min frequency to consider, we go LOW TUNE
    const MAX_FREQ = 400.0; // max frequency for fundamental detection
    const HARMONIC_THRESHOLD = 0.3; // min relative amplitude for harmonic detection

    inline function analyzePitch(freq)
    {
        if (!freq || freq < 0 || freq > MAX_FREQ)
        {
            lblTuner.set("text", "TUNE"); // FIX ME: panel reset goes here (when i add it)
            return;
        }

        local midiNote = 69 + 12 * Math.log(freq / referenceTuning) / Math.log(2);
        local closestNote = Math.round(midiNote);
        local distanceCents = (midiNote - closestNote) * 100;

        // if too far off: switch to next note
        if (Math.abs(distanceCents) > 50) 
        {
            closestNote += distanceCents > 0 ? 1 : -1;
            distanceCents = (midiNote - closestNote) * 100;
        }

        // get note name
        local noteIndex = ((closestNote % 12) + 12) % 12;
        local noteName = NOTES[noteIndex];

        // get target frequency
        local targetFreq = referenceTuning * Math.pow(2, (closestNote - 69) / 12);

        // get deviation in Hz if needed
        local distanceHz = freq - targetFreq;

        note = noteName;
        deviation = tuningMode == "cents" ? Math.round(distanceCents) : Math.round(distanceHz);
        unit = tuningMode == "cents" ? "C" : "Hz";
        inTune = Math.abs(distanceCents) < 5; // use cents either way since it's just a bool        
        direction = deviation > 0 ? "+" : "-";

		// repaint UI        
        lblTunerNote.set("text", note);
        lblTunerDeviation.set("text", direction + Math.round(Math.abs(deviation)) + unit);        
        pnlTunerDisplay.repaint();
    }

    /* GLOBAL CABLE */

    const grm = Engine.getGlobalRoutingManager();
    const pitchCable = grm.getCable("pitch");
        
    pitchCable.registerCallback(function(data)
    {
        if (data < 0.0)
            return;
        
        var logMin = Math.log(40.0);
        var logMax = Math.log(400.0);
        var logFreq = logMin + data * (logMax - logMin);
        var pitch = Math.exp(logFreq);  
                
        analyzePitch(pitch);
        
    }, false);
    
    lblTunerNote.set("text", "C");
    lblTunerDeviation.set("text", "+0c");    
    
}