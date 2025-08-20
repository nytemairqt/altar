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

// DSP MODULES

// Pre FX
const gate = Synth.getEffect("gate");

// Amp
const pitchShifterFixed = Synth.getEffect("pitchShifterFixed");
const preSculpt = Synth.getEffect("preSculpt");
const ampFixed = Synth.getEffect("ampFixed");
const postSculpt = Synth.getEffect("postSculpt");

// Cab
const var cabConvolution = Synth.getAudioSampleProcessor("cabConvolution");
const cabEQMain = Synth.getEffect("cabEQMain");
const cabEQDetails = Synth.getEffect("cabEQDetails");
const cabAxis = Synth.getEffect("cabAxis");
const eqWhistle = Synth.getEffect("eqWhistle");

// Pedals
const reverbFixed = Synth.getEffect("reverbFixed");

// Synths (Debug) 
const cabMIDIPlayer = Synth.getMidiPlayer("cabMIDIPlayer");
const cabFileSave = Synth.getAudioSampleProcessor("cabFileSave");