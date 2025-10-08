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
    const lblTuner = Content.getComponent("lblTuner");
    const pnlTuner = Content.getComponent("pnlTuner");

    inline function onbtnShowTunerControl(component, value) { pnlTuner.set("visible", value); }    
    inline function onbtnTunerMonitorControl(component, value) { tuner.setAttribute(tuner.Monitor, 1-value); }
    btnShowTuner.setControlCallback(onbtnShowTunerControl);
    btnTunerMonitor.setControlCallback(onbtnTunerMonitorControl);

    pnlTuner.setPaintRoutine(function(g)
    {
        var bounds = [310, 80, 530, 310];

        g.setColour(Colours.withAlpha(Colours.black, 1.0));
        g.fillRoundedRectangle(bounds, 2.0);
    });

    pnlTuner.setMouseCallback(function(event)
    {
        var x = 310;
        var y = 80;
        var w = 530;
        var h = 310;
        
        if (event.mouseDownX < x || event.mouseDownX > (x + w) || event.mouseDownY < y || event.mouseDownY > (y + h)) 
        {
            btnShowTuner.setValue(0);
            btnShowTuner.changed();
        }   
    });

    // Pitch Detection
    const NOTES = ["C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"];
    var referenceTuning = 440; // might include a 432 button or something
    var tuningMode = 'cents'; // 'cents' or 'hz', might not even implement a hz toggle

    const MIN_FREQ = 20.0;   // min frequency to consider, we go LOW TUNE
    const MAX_FREQ = 2000.0; // max frequency for fundamental detection
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

        local note = noteName;
        local deviation = tuningMode == "cents" ? Math.round(distanceCents) : Math.round(distanceHz);
        local unit = tuningMode == "cents" ? "C" : "Hz";
        local inTune = Math.abs(distanceCents) < 5; // use cents either way since it's just a bool        
        local direction = deviation > 0 ? "+" : "-";

        // need a panel to set paint routine here
        lblTuner.set("text", note + " " + direction + Math.round(Math.abs(deviation)) + unit);

        return [note, deviation, unit, freq, targetFreq];
    }

    const dp0 = Synth.getDisplayBufferSource("tuner");
    const dp = dp0.getDisplayBuffer(0);
    const FFT_SIZE = 65536;
    const MAX_LENGTH = 65536;
    const fft = Engine.createFFT();
    fft.setWindowType(fft.Hann);
    fft.setOverlap(0.0);
    var detectedPitch = [];

    dp.setRingBufferProperties({
      "BufferLength": MAX_LENGTH,
      "NumChannels": 2
    });

    inline function binIndexToFreq(idx)
    {
        return (idx / FFT_SIZE) * Engine.getSampleRate();
    }

    inline function findPitchHPS(magBuffer, numHarmonics)
    {
        /* multiplies downsampled versions of the spectrum to enhance fundamental / avoid stupid overtones */
        if (!numHarmonics) numHarmonics = 8;
        
        local spectrum = magBuffer[0];
        local spectrumLength = spectrum.length;
        local hpsSpectrum = Buffer.create(Math.floor(spectrumLength / numHarmonics));                    
        
        for (i = 0; i < hpsSpectrum.length; i++)
        {
            hpsSpectrum[i] = spectrum[i];
        }
        
        for (h = 2; h <= numHarmonics; h++)
        {
            local maxIndex = Math.min(Math.floor(spectrumLength / h), hpsSpectrum.length);
            for (i = 0; i < maxIndex; i++)
            {
                local harmonicIndex = i * h;
                if (harmonicIndex < spectrumLength)
                    hpsSpectrum[i] *= spectrum[harmonicIndex];
            }
        }
        
        // find peak in valid frequency range
        local maxIdx = 0;
        local maxVal = 0;    
        local minBin = Math.floor((MIN_FREQ / Engine.getSampleRate()) * FFT_SIZE);
        local maxBin = Math.floor((MAX_FREQ / Engine.getSampleRate()) * FFT_SIZE);
        
        for (i = minBin; i < Math.min(maxBin, hpsSpectrum.length); i++)
        {
            if (hpsSpectrum[i] > maxVal)
            {
                maxVal = hpsSpectrum[i];
                maxIdx = i;
            }
        }
        
        // grab index
        return binIndexToFreq(maxIdx);
    }

    // executed per slice of the fft
    // the false bool is {normalized | decibels}
    fft.setMagnitudeFunction(function(magBuffer, offset)
    {
        // pass the bins through the HPS
        var f = findPitchHPS(magBuffer, 4);
        detectedPitch = analyzePitch(f); 
    }, false);


    // prepare FFT & setup record buffer to pass to it
    fft.prepare(FFT_SIZE, 2);
    const recordBuffer = [Buffer.create(MAX_LENGTH), Buffer.create(MAX_LENGTH)];

    inline function analyse()
    {
        // Copy the buffer using the thread safe copy method
        // might need this in the HPS function as well...
        dp.copyReadBuffer(recordBuffer);

        // and process
        fft.process(recordBuffer);
    }

    const tunerTimer = Engine.createTimerObject();
    tunerTimer.setTimerCallback(function()
    {
        analyse();
    });

    // definitely dont need this to be too fast
    tunerTimer.startTimer(50);
}