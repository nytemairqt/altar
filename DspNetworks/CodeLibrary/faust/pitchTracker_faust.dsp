declare name	"Pitch Tracker"; 

import("stdfaust.lib");
import("analyzers.lib");

a = hslider("n cycles", 1, 0, 1, 1);
b = hslider("Gain", 0, 0.0, 1.0, 0.01) : si.smoo;

//process = pitchTracker(8.0) : si.smoo : si.smoo;
//process = _ * a, _ * a;
//pitch = _ : an.pitchTracker(8.0, 1.0) : _;
//process = _ : pitchTracker(8.0, 1.0) : _, _ : pitchTracker(8.0, 1.0) : _;
process = _, _;