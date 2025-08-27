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


// ===============================
// DSP MODULES

include("Boilerplate/dspProfiles.js");

// Generic
const testAudio = Synth.getAudioSampleProcessor("testAudio");
const modules = Synth.getAllEffects(".*");

// Click
const click = Synth.getChildSynth("click");
const clickMIDI = Synth.getMidiPlayer("clickMIDI");
const btnClick = Content.getComponent("btnClick");
const knbClickGain = Content.getComponent("knbClickGain");
const lblClickDisasbled = Content.getComponent("lblClickDisasbled");

// Input Gain
const inputGain = Synth.getEffect("inputGain");
const knbInputGain = Content.getComponent("knbInputGain");

// Output Gain
const outputGain = Synth.getEffect("outputGain");
const knbOutputGain = Content.getComponent("knbOutputGain");

// Oversampling
const btnOversampling = Content.getComponent("btnOversampling");

// Gate
const gate = Synth.getEffect("gate");
const btnGate = Content.getComponent("btnGate");
const knbGateThreshold = Content.getComponent("knbGateThreshold");

// Limiter
const limiter = Synth.getEffect("limiter");
const btnLimiter = Content.getComponent("btnLimiter");

// Whistle
const whistle = Synth.getEffect("whistle");
const knbEQWhistle = Content.getComponent("knbEQWhistle");

// Tuner
const tuner = Synth.getEffect("tuner");
const btnShowTuner = Content.getComponent("btnShowTuner");
const btnTunerMonitor = Content.getComponent("btnTunerMonitor");
const lblTuner = Content.getComponent("lblTuner");
const pnlTuner = Content.getComponent("pnlTuner");

// Pitch-Shifter
const knbPitch = Content.getComponent("knbPitch");
const btnPitch = Content.getComponent("btnPitch");
const btnPitchSnap = Content.getComponent("btnPitchSnap");

// Amp
const pitch = Synth.getEffect("pitch");
const ampPreSculpt = Synth.getEffect("ampPreSculpt");
const amp = Synth.getEffect("amp");
const ampPostSculpt = Synth.getEffect("ampPostSculpt");
const btnShowAmp = Content.getComponent("btnShowAmp");
const btnAmpMode = Content.getComponent("btnAmpMode");
const knbCleanInput = Content.getComponent("knbCleanInput");
const knbCleanOutput = Content.getComponent("knbCleanOutput");
const knbDirtyInput = Content.getComponent("knbDirtyInput");
const knbDirtyOutput = Content.getComponent("knbDirtyOutput");
const pnlAmp = Content.getComponent("pnlAmp");
const pnlAmpClean = Content.getComponent("pnlAmpClean");
const pnlAmpDirty = Content.getComponent("pnlAmpDirty");

// Cab
const cabScriptFX = Synth.getAudioSampleProcessor("cabScriptFX");
const cab = Synth.getEffect("cab");
const btnCab = Content.getComponent("btnCab");
const btnShowCab = Content.getComponent("btnShowCab");
const btnCabAEnable = Content.getComponent("btnCabAEnable");
const btnCabBEnable = Content.getComponent("btnCabBEnable");
const btnCabAPhase = Content.getComponent("btnCabAPhase");
const btnCabALoadPrev = Content.getComponent("btnCabALoadPrev");
const btnCabALoadNext = Content.getComponent("btnCabALoadNext");
const knbCabAAxis = Content.getComponent("knbCabAAxis");
const knbCabAAngle = Content.getComponent("knbCabAAngle");
const knbCabADistance = Content.getComponent("knbCabADistance");
const knbCabADelay = Content.getComponent("knbCabADelay");
const knbCabAPan = Content.getComponent("knbCabAPan");
const knbCabAGain = Content.getComponent("knbCabAGain");
const pnlCab = Content.getComponent("pnlCab");

// Cab Designer
const cabDesignerSpeaker = Synth.getEffect("cabDesignerSpeaker");
const cabDesignerMojo = Synth.getEffect("cabDesignerMojo");
const cabDesignerMic = Synth.getEffect("cabDesignerMic");
const cabDesignerEQ = Synth.getEffect("cabDesignerEQ");
const cabDesignerFileSave = Synth.getAudioSampleProcessor("cabDesignerFileSave");
const cabDesignerMIDIPlayer = Synth.getMidiPlayer("cabDesignerMIDIPlayer");
const btnShowCabDesigner = Content.getComponent("btnShowCabDesigner");
const btnCabGenerate = Content.getComponent("btnCabGenerate");
const btnCabSave = Content.getComponent("btnCabSave");
const btnOpenCabFolder = Content.getComponent("btnOpenCabFolder");
const cmbCabDesignerSpeaker = Content.getComponent("cmbCabDesignerSpeaker");
const cmbCabDesignerMic = Content.getComponent("cmbCabDesignerMic");
const knbCabDesignerMojo = Content.getComponent("knbCabDesignerMojo");
const knbCabDesignerAge = Content.getComponent("knbCabDesignerAge");
const lblCabSaveName = Content.getComponent("lblCabSaveName");
const fltCabDesignerEQ = Content.getComponent("fltCabDesignerEQ");
const pnlCabDesigner = Content.getComponent("pnlCabDesigner");

// Chug
const chug = Synth.getEffect("chug");
const btnChug = Content.getComponent("btnChug");
const knbChugThreshold = Content.getComponent("knbChugThreshold");
const knbChugFreq = Content.getComponent("knbChugFreq");

// Grit
const grit = Synth.getEffect("grit");

// LoFi
const lofi = Synth.getEffect("lofi");
const btnLofi = Content.getComponent("btnLofi");
const knbLofiLow = Content.getComponent("knbLofiLow");
const knbLofiHigh = Content.getComponent("knbLofiHigh");

// Octave
const octavePre = Synth.getEffect("octavePre");
const octavePost = Synth.getEffect("octavePost");
const btnOctave = Content.getComponent("btnOctave");
const btnOctavePosition = Content.getComponent("btnOctavePosition");
const knbOctave = Content.getComponent("knbOctave");
const knbOctaveFreq = Content.getComponent("knbOctaveFreq");

// Preprocess (Comp & EQ)
// Postprocess (Comp & EQ)
// Overdrive & Fuzz
const btnShowOverdrive = Content.getComponent("btnShowOverdrive");
const pnlOverdrive = Content.getComponent("pnlOverdrive");
// Reverb
const reverb = Synth.getEffect("reverb");
const btnShowReverb = Content.getComponent("btnShowReverb");
const knbReverbMix = Content.getComponent("knbReverbMix");
const knbReverbBrightness = Content.getComponent("knbReverbBrightness");
const knbReverbFeedback = Content.getComponent("knbReverbFeedback");
const pnlReverb = Content.getComponent("pnlReverb");
// Delay
const btnShowDelay = Content.getComponent("btnShowDelay");
const pnlDelay = Content.getComponent("pnlDelay");
// Chorus
const btnShowChorus = Content.getComponent("btnShowChorus");
const pnlChorus = Content.getComponent("pnlChorus");
// Ring Mod
const btnShowRingMod = Content.getComponent("btnShowRingMod");
const pnlRingMod = Content.getComponent("pnlRingMod");
    
// MISC VARIABLES
var eventList = [];
const impulseSize = 1024;
const moduleBypassedStates = [];
const audioFiles = FileSystem.getFolder(FileSystem.AudioFiles);
reg cabSaveName = "myCab.wav";
const isPlugin = Engine.isPlugin();

// INITIAL SETUP
Engine.loadAudioFilesIntoPool();
cabDesignerMIDIPlayer.create(4, 4, 1);

// keep me here
reg clickMIDIList = [  
  "MessageHolder: Type: NoteOn, Channel: 1, Number: 84, Value: 100, EventId: 0, Timestamp: 0, ",
  "MessageHolder: Type: NoteOff, Channel: 1, Number: 84, Value: 64, EventId: 0, Timestamp: 2756, ",
  "MessageHolder: Type: NoteOn, Channel: 1, Number: 84, Value: 100, EventId: 1, Timestamp: 22050, ",
  "MessageHolder: Type: NoteOff, Channel: 1, Number: 84, Value: 64, EventId: 1, Timestamp: 24806, ",
  "MessageHolder: Type: NoteOn, Channel: 1, Number: 84, Value: 100, EventId: 2, Timestamp: 44100, ",
  "MessageHolder: Type: NoteOff, Channel: 1, Number: 84, Value: 64, EventId: 2, Timestamp: 46856, ",
  "MessageHolder: Type: NoteOn, Channel: 1, Number: 84, Value: 100, EventId: 3, Timestamp: 66150, ",
  "MessageHolder: Type: NoteOff, Channel: 1, Number: 84, Value: 64, EventId: 3, Timestamp: 68906, "
];

// Disable App-Only controls
if (isPlugin)
{
	btnClick.set("enabled", false);
	knbClickGain.set("enabled", false);
	lblClickDisasbled.set("visible", true); 
}