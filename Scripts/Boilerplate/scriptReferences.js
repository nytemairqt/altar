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

// Master FX
const inputGain = Synth.getEffect("inputGain");
const outputGain = Synth.getEffect("outputGain");
const gate = Synth.getEffect("gate");
const limiter = Synth.getEffect("limiter");
const tuner = Synth.getEffect("tuner");

// Amp
const pitchShifterFixed = Synth.getEffect("pitchShifterFixed");
const preSculpt = Synth.getEffect("preSculpt");
const ampFixed = Synth.getEffect("ampFixed");
const postSculpt = Synth.getEffect("postSculpt");

// Cab
const cabConvolution = Synth.getAudioSampleProcessor("cabConvolution");
const cabEQMain = Synth.getEffect("cabEQMain");
const cabEQDetails = Synth.getEffect("cabEQDetails");
const cabEQCustom = Synth.getEffect("cabEQCustom");
const cabAxis = Synth.getEffect("cabAxis");
const eqWhistle = Synth.getEffect("eqWhistle");
const cabEQCustomBlankState = "186.3ocMNsrCBBCDbqDN3A+WjeAPHwCZTKw6U5FoI0tZenxWutfxdZlLyrybvScXHPdPj2NbGAwx7pj+IV+.1tADqx6TWpOVkBQ5FTNbWEBnFDhr8oaMFaD8ANp.3qoos1otX+oKxp0lH4kQUDYOYkjdP1SubLNu.NaBF16OhjrzHZM.U8Fq9v7rB7eOQonwccmJ5Mu4x3lkTx2gU8JmCsSCXALFchuddPRzomHe36uXwHW7WrXVD9hSykzK";

// Pedals
const reverbFixed = Synth.getEffect("reverbFixed");

// Synths (Debug) 
const cabMIDIPlayer = Synth.getMidiPlayer("cabMIDIPlayer");
const cabFileSave = Synth.getAudioSampleProcessor("cabFileSave");
const testAudio = Synth.getAudioSampleProcessor("testAudio");

const modules = [Synth.getEffect("gate"),
    Synth.getEffect("pitchShifterFixed"),
    Synth.getEffect("preSculpt"),
    Synth.getEffect("ampFixed"),
    Synth.getEffect("postSculpt"),
    Synth.getEffect("cabEQMain"),
    Synth.getEffect("cabEQDetails"),
    Synth.getEffect("cabAxis"),
    Synth.getEffect("eqWhistle"),
    Synth.getEffect("reverbFixed")];
    
// ===============================
// UI ELEMENTS

// Knobs
const knbInputGain = Content.getComponent("knbInputGain");
const knbOutputGain = Content.getComponent("knbOutputGain");
const knbGateThreshold = Content.getComponent("knbGateThreshold");

const knbCleanInput = Content.getComponent("knbCleanInput");
const knbCleanOutput = Content.getComponent("knbCleanOutput");
const knbDirtyInput = Content.getComponent("knbDirtyInput");
const knbDirtyOutput = Content.getComponent("knbDirtyOutput");

const knbPitch = Content.getComponent("knbPitch");

const knbCabAxis = Content.getComponent("knbCabAxis");
const knbCabPresence = Content.getComponent("knbCabPresence");
const knbEQWhistle = Content.getComponent("knbEQWhistle");

const var knbReverbMix = Content.getComponent("knbReverbMix");
const var knbReverbBrightness = Content.getComponent("knbReverbBrightness");
const var knbReverbFeedback = Content.getComponent("knbReverbFeedback");

// Buttons

const btnAmpMode = Content.getComponent("btnAmpMode");
const btnOversampling = Content.getComponent("btnOversampling");
const btnCabGenerate = Content.getComponent("btnCabGenerate");
const btnCabSave = Content.getComponent("btnCabSave");
const btnOpenCabFolder = Content.getComponent("btnOpenCabFolder");
const btnShowOverdrive = Content.getComponent("btnShowOverdrive");
const btnShowAmp = Content.getComponent("btnShowAmp");
const btnShowCab = Content.getComponent("btnShowCab");
const btnShowReverb = Content.getComponent("btnShowReverb");
const btnShowDelay = Content.getComponent("btnShowDelay");
const btnShowChorus = Content.getComponent("btnShowChorus");
const btnShowRingMod = Content.getComponent("btnShowRingMod");
const btnShowTuner = Content.getComponent("btnShowTuner");
const btnTunerMonitor = Content.getComponent("btnTunerMonitor");
const btnLimiter = Content.getComponent("btnLimiter");
const btnPitch = Content.getComponent("btnPitch");
const btnGate = Content.getComponent("btnGate");

// Comboboxes

// Labels

const lblCabSaveName = Content.getComponent("lblCabSaveName");
const lblTuner = Content.getComponent("lblTuner");



// Panels

const pnlOverdrive = Content.getComponent("pnlOverdrive");
const pnlAmp = Content.getComponent("pnlAmp");
const pnlCab = Content.getComponent("pnlCab");
const pnlTuner = Content.getComponent("pnlTuner");
const pnlReverb = Content.getComponent("pnlReverb");
const pnlDelay = Content.getComponent("pnlDelay");
const pnlChorus = Content.getComponent("pnlChorus");
const pnlRingMod = Content.getComponent("pnlRingMod");

// ===============================
// MISC VARIABLES
var eventList = [];
const impulseSize = 1024;
const moduleBypassedStates = [];
const audioFiles = FileSystem.getFolder(FileSystem.AudioFiles);
reg cabSaveName = "myCab.wav";

// INITIAL SETUP
Engine.loadAudioFilesIntoPool();
cabMIDIPlayer.create(4, 4, 1);