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

include("Boilerplate/utilityFunctions.js");

// setting up vars
const NOTES = ["C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"];
var referenceTuning = 440; // will be a slider at some point 
var tuningMode = 'cents'; // 'cents' or 'hz'

const MIN_FREQ = 20.0;   // Minimum frequency to consider, we go LOW TUNE
const MAX_FREQ = 2000.0; // Maximum frequency for fundamental detection
const HARMONIC_THRESHOLD = 0.3; // Minimum relative amplitude for harmonic detection

inline function analyzePitch(freq)
{
    if (!freq || freq < 0)
    {
        lblTuner.set("text", "TUNE"); // reset the panel here
        return;
    }

    local midiNote = 69 + 12 * Math.log(freq / referenceTuning) / Math.log(2);

    local closestNote = Math.round(midiNote);
    local distanceCents = (midiNote - closestNote) * 100;

    // If too far off, switch to adjacent note
    if (Math.abs(distanceCents) > 50) 
    {
        closestNote += distanceCents > 0 ? 1 : -1;
        distanceCents = (midiNote - closestNote) * 100;
    }

    // Get note name
    local noteIndex = ((closestNote % 12) + 12) % 12;
    local noteName = NOTES[noteIndex];

    // Calculate target frequency
    local targetFreq = referenceTuning * Math.pow(2, (closestNote - 69) / 12);

    // Calculate deviation in Hz if needed
    local distanceHz = freq - targetFreq;

    local note = noteName;
    local deviation = tuningMode == "cents" ? Math.round(distanceCents) : Math.round(distanceHz);
    local unit = tuningMode == "cents" ? "C" : "Hz";
    local inTune = Math.abs(distanceCents) < 5; // use cents either way since it's just a bool
    
    // Debug 
    /*
    Console.print("---------------");
    Console.print("Note: " + note);
    Console.print("Deviation: " + deviation);
    Console.print("Unit: " + unit);
    Console.print("Frequency: " + freq);
    Console.print("TargetFreq: " + targetFreq);
    Console.print("---------------");
    */

    local direction = deviation > 0 ? "+" : "-";

    // need a panel to set paint routine here
    lblTuner.set("text", note + " " + direction + Math.round(Math.abs(deviation)) + unit);

    return [note, deviation, unit, freq, targetFreq];
}

// Setup display buffer & Instantiate FFT
const dp0 = Synth.getDisplayBufferSource("tuner");
const dp = dp0.getDisplayBuffer(0);
//const var FFT_SIZE = 16384;
//const FFT_SIZE = 32768;
const FFT_SIZE = 65536;
const MAX_LENGTH = 65536;
const fft = Engine.createFFT();
//fft.setWindowType(fft.BlackmanHarris);
fft.setWindowType(fft.Hann);
fft.setOverlap(0.0);
var pitch = [];

// make sure the ring buffer is as big as possible
dp.setRingBufferProperties({
  "BufferLength": MAX_LENGTH,
  "NumChannels": 2
});

// Convert binIndex to its center frequency
inline function binIndexToFreq(idx)
{
    return (idx / FFT_SIZE) * Engine.getSampleRate();
}

// Harmonic Product Spectrum
// Multiplies downsampled versions of the spectrum to enhance fundamental / avoid stupid overtones
inline function findPitchHPS(magBuffer, numHarmonics)
{
    if (!numHarmonics) numHarmonics = 8;
    
    local spectrum = magBuffer[0];
    local spectrumLength = spectrum.length;
    local hpsSpectrum = Buffer.create(Math.floor(spectrumLength / numHarmonics));                    
    
    // Initialize HPS spectrum with original spectrum
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
    
    // Find peak in valid frequency range
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

// Executed per slice of the fft
// the false bool is {normalized | decibels}
fft.setMagnitudeFunction(function(magBuffer, offset)
{
    // pass the bins through the HPS
    var f = findPitchHPS(magBuffer, 4);
    pitch = analyzePitch(f); 
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


// timer object will run our HPS on the signal, and update our Label text in realtime
const tunerTimer = Engine.createTimerObject();

tunerTimer.setTimerCallback(function()
{
    analyse();
});

// definitely dont need this to be too fast
tunerTimer.startTimer(100);