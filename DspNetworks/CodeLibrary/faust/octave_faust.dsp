declare name 		"pitchShifter";
declare version 	"1.0";
declare author 		"Grame";
declare license 	"BSD";
declare copyright 	"(c)GRAME 2006";

 //--------------------------------------
 // very simple real time pitch shifter
 //--------------------------------------
 
import("stdfaust.lib");

pitchshifter = vgroup("Pitch Shifter", ef.transpose(
									hslider("window", 1000, 50, 10000, 1),
									hslider("xfade", 16, 1, 10000, 1),
									-12.0
								  )
				);
				
f = hslider("Freq", 300, 20, 1000, 1.0);			
mix = hslider("Mix [style:knob]", 1.0, 0, 1, 0.01);

q = 0.5;
g = 1;

x = _;
y = pitchshifter * mix : fi.resonlp(f,q,g);

// DONT DELETE ME OMG
process = x, x <: x, x, y, y :> x, x;

