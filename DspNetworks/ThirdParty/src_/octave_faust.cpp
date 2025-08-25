/* ------------------------------------------------------------
author: "Grame"
copyright: "(c)GRAME 2006"
license: "BSD"
name: "pitchShifter"
version: "1.0"
Code generated with Faust 2.81.2 (https://faust.grame.fr)
Compilation options: -lang cpp -rui -nvi -ct 1 -cn _octave_faust -scn ::faust::dsp -es 1 -mcd 16 -mdd 1024 -mdy 33 -uim -single -ftz 0
------------------------------------------------------------ */

#ifndef  ___octave_faust_H__
#define  ___octave_faust_H__

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif 

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <math.h>

#ifndef FAUSTCLASS 
#define FAUSTCLASS _octave_faust
#endif

#ifdef __APPLE__ 
#define exp10f __exp10f
#define exp10 __exp10
#endif

#if defined(_WIN32)
#define RESTRICT __restrict
#else
#define RESTRICT __restrict__
#endif

static float _octave_faust_faustpower2_f(float value) {
	return value * value;
}

struct _octave_faust final : public ::faust::dsp {
	
	FAUSTFLOAT fHslider0;
	int fSampleRate;
	float fConst0;
	FAUSTFLOAT fHslider1;
	float fRec1[2];
	FAUSTFLOAT fHslider2;
	int IOTA0;
	float fVec0[131072];
	FAUSTFLOAT fHslider3;
	float fRec0[3];
	float fVec1[131072];
	float fRec2[3];
	
	_octave_faust() {
	}
	
	void metadata(Meta* m) { 
		m->declare("author", "Grame");
		m->declare("compile_options", "-lang cpp -rui -nvi -ct 1 -cn _octave_faust -scn ::faust::dsp -es 1 -mcd 16 -mdd 1024 -mdy 33 -uim -single -ftz 0");
		m->declare("copyright", "(c)GRAME 2006");
		m->declare("delays.lib/name", "Faust Delay Library");
		m->declare("delays.lib/version", "1.2.0");
		m->declare("filename", "octave_faust.dsp");
		m->declare("filters.lib/fir:author", "Julius O. Smith III");
		m->declare("filters.lib/fir:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/fir:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/iir:author", "Julius O. Smith III");
		m->declare("filters.lib/iir:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/iir:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/lowpass0_highpass1", "MIT-style STK-4.3 license");
		m->declare("filters.lib/name", "Faust Filters Library");
		m->declare("filters.lib/resonlp:author", "Julius O. Smith III");
		m->declare("filters.lib/resonlp:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/resonlp:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/tf2:author", "Julius O. Smith III");
		m->declare("filters.lib/tf2:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/tf2:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/tf2s:author", "Julius O. Smith III");
		m->declare("filters.lib/tf2s:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/tf2s:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/version", "1.7.1");
		m->declare("license", "BSD");
		m->declare("maths.lib/author", "GRAME");
		m->declare("maths.lib/copyright", "GRAME");
		m->declare("maths.lib/license", "LGPL with exception");
		m->declare("maths.lib/name", "Faust Math Library");
		m->declare("maths.lib/version", "2.8.1");
		m->declare("misceffects.lib/name", "Misc Effects Library");
		m->declare("misceffects.lib/version", "2.5.1");
		m->declare("name", "pitchShifter");
		m->declare("platform.lib/name", "Generic Platform Library");
		m->declare("platform.lib/version", "1.3.0");
		m->declare("version", "1.0");
	}

	static constexpr int getStaticNumInputs() {
		return 2;
	}
	static constexpr int getStaticNumOutputs() {
		return 2;
	}
	int getNumInputs() {
		return 2;
	}
	int getNumOutputs() {
		return 2;
	}
	
	static void classInit(int sample_rate) {
	}
	
	void instanceConstants(int sample_rate) {
		fSampleRate = sample_rate;
		fConst0 = 3.1415927f / std::min<float>(1.92e+05f, std::max<float>(1.0f, float(fSampleRate)));
	}
	
	void instanceResetUserInterface() {
		fHslider0 = FAUSTFLOAT(3e+02f);
		fHslider1 = FAUSTFLOAT(1e+03f);
		fHslider2 = FAUSTFLOAT(16.0f);
		fHslider3 = FAUSTFLOAT(1.0f);
	}
	
	void instanceClear() {
		for (int l0 = 0; l0 < 2; l0 = l0 + 1) {
			fRec1[l0] = 0.0f;
		}
		IOTA0 = 0;
		for (int l1 = 0; l1 < 131072; l1 = l1 + 1) {
			fVec0[l1] = 0.0f;
		}
		for (int l2 = 0; l2 < 3; l2 = l2 + 1) {
			fRec0[l2] = 0.0f;
		}
		for (int l3 = 0; l3 < 131072; l3 = l3 + 1) {
			fVec1[l3] = 0.0f;
		}
		for (int l4 = 0; l4 < 3; l4 = l4 + 1) {
			fRec2[l4] = 0.0f;
		}
	}
	
	void init(int sample_rate) {
		classInit(sample_rate);
		instanceInit(sample_rate);
	}
	
	void instanceInit(int sample_rate) {
		instanceConstants(sample_rate);
		instanceResetUserInterface();
		instanceClear();
	}
	
	_octave_faust* clone() {
		return new _octave_faust();
	}
	
	int getSampleRate() {
		return fSampleRate;
	}
	
	void buildUserInterface(UI* ui_interface) {
		ui_interface->openVerticalBox("pitchShifter");
		ui_interface->addHorizontalSlider("Freq", &fHslider0, FAUSTFLOAT(3e+02f), FAUSTFLOAT(2e+01f), FAUSTFLOAT(1e+03f), FAUSTFLOAT(1.0f));
		ui_interface->declare(&fHslider3, "style", "knob");
		ui_interface->addHorizontalSlider("Mix", &fHslider3, FAUSTFLOAT(1.0f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->openVerticalBox("Pitch Shifter");
		ui_interface->addHorizontalSlider("window", &fHslider1, FAUSTFLOAT(1e+03f), FAUSTFLOAT(5e+01f), FAUSTFLOAT(1e+04f), FAUSTFLOAT(1.0f));
		ui_interface->addHorizontalSlider("xfade", &fHslider2, FAUSTFLOAT(16.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(1e+04f), FAUSTFLOAT(1.0f));
		ui_interface->closeBox();
		ui_interface->closeBox();
	}
	
	void compute(int count, FAUSTFLOAT** RESTRICT inputs, FAUSTFLOAT** RESTRICT outputs) {
		FAUSTFLOAT* input0 = inputs[0];
		FAUSTFLOAT* input1 = inputs[1];
		FAUSTFLOAT* output0 = outputs[0];
		FAUSTFLOAT* output1 = outputs[1];
		float fSlow0 = std::tan(fConst0 * std::max<float>(2e+01f, std::min<float>(1e+03f, float(fHslider0))));
		float fSlow1 = 2.0f * (1.0f - 1.0f / _octave_faust_faustpower2_f(fSlow0));
		float fSlow2 = 1.0f / fSlow0;
		float fSlow3 = (fSlow2 + -2.0f) / fSlow0 + 1.0f;
		float fSlow4 = 1.0f / ((fSlow2 + 2.0f) / fSlow0 + 1.0f);
		float fSlow5 = std::max<float>(5e+01f, std::min<float>(1e+04f, float(fHslider1)));
		float fSlow6 = 1.0f / std::max<float>(1.0f, std::min<float>(1e+04f, float(fHslider2)));
		float fSlow7 = std::max<float>(0.0f, std::min<float>(1.0f, float(fHslider3)));
		for (int i0 = 0; i0 < count; i0 = i0 + 1) {
			fRec1[0] = std::fmod(fSlow5 + fRec1[1] + 0.5f, fSlow5);
			float fTemp0 = std::min<float>(fSlow6 * fRec1[0], 1.0f);
			float fTemp1 = 1.0f - fTemp0;
			float fTemp2 = float(input0[i0]);
			fVec0[IOTA0 & 131071] = fTemp2;
			float fTemp3 = fSlow5 + fRec1[0];
			int iTemp4 = int(fTemp3);
			int iTemp5 = std::min<int>(65537, std::max<int>(0, iTemp4 + 1));
			float fTemp6 = std::floor(fTemp3);
			float fTemp7 = fSlow5 + (fRec1[0] - fTemp6);
			float fTemp8 = 1.0f - fRec1[0];
			float fTemp9 = fTemp6 + fTemp8 - fSlow5;
			int iTemp10 = std::min<int>(65537, std::max<int>(0, iTemp4));
			int iTemp11 = int(fRec1[0]);
			int iTemp12 = std::min<int>(65537, std::max<int>(0, iTemp11 + 1));
			float fTemp13 = std::floor(fRec1[0]);
			float fTemp14 = fRec1[0] - fTemp13;
			float fTemp15 = fTemp13 + fTemp8;
			int iTemp16 = std::min<int>(65537, std::max<int>(0, iTemp11));
			fRec0[0] = fSlow7 * ((fVec0[(IOTA0 - iTemp16) & 131071] * fTemp15 + fTemp14 * fVec0[(IOTA0 - iTemp12) & 131071]) * fTemp0 + (fVec0[(IOTA0 - iTemp10) & 131071] * fTemp9 + fTemp7 * fVec0[(IOTA0 - iTemp5) & 131071]) * fTemp1) - fSlow4 * (fSlow3 * fRec0[2] + fSlow1 * fRec0[1]);
			output0[i0] = FAUSTFLOAT(fTemp2 + fSlow4 * (fRec0[2] + fRec0[0] + 2.0f * fRec0[1]));
			float fTemp17 = float(input1[i0]);
			fVec1[IOTA0 & 131071] = fTemp17;
			fRec2[0] = fSlow7 * (fTemp0 * (fVec1[(IOTA0 - iTemp16) & 131071] * fTemp15 + fTemp14 * fVec1[(IOTA0 - iTemp12) & 131071]) + fTemp1 * (fTemp9 * fVec1[(IOTA0 - iTemp10) & 131071] + fTemp7 * fVec1[(IOTA0 - iTemp5) & 131071])) - fSlow4 * (fSlow3 * fRec2[2] + fSlow1 * fRec2[1]);
			output1[i0] = FAUSTFLOAT(fTemp17 + fSlow4 * (fRec2[2] + fRec2[0] + 2.0f * fRec2[1]));
			fRec1[1] = fRec1[0];
			IOTA0 = IOTA0 + 1;
			fRec0[2] = fRec0[1];
			fRec0[1] = fRec0[0];
			fRec2[2] = fRec2[1];
			fRec2[1] = fRec2[0];
		}
	}

};

#ifdef FAUST_UIMACROS
	
	#define FAUST_FILE_NAME "octave_faust.dsp"
	#define FAUST_CLASS_NAME "_octave_faust"
	#define FAUST_COMPILATION_OPIONS "-lang cpp -rui -nvi -ct 1 -cn _octave_faust -scn ::faust::dsp -es 1 -mcd 16 -mdd 1024 -mdy 33 -uim -single -ftz 0"
	#define FAUST_INPUTS 2
	#define FAUST_OUTPUTS 2
	#define FAUST_ACTIVES 4
	#define FAUST_PASSIVES 0

	FAUST_ADDHORIZONTALSLIDER("Freq", fHslider0, 3e+02f, 2e+01f, 1e+03f, 1.0f);
	FAUST_ADDHORIZONTALSLIDER("Mix", fHslider3, 1.0f, 0.0f, 1.0f, 0.01f);
	FAUST_ADDHORIZONTALSLIDER("Pitch Shifter/window", fHslider1, 1e+03f, 5e+01f, 1e+04f, 1.0f);
	FAUST_ADDHORIZONTALSLIDER("Pitch Shifter/xfade", fHslider2, 16.0f, 1.0f, 1e+04f, 1.0f);

	#define FAUST_LIST_ACTIVES(p) \
		p(HORIZONTALSLIDER, Freq, "Freq", fHslider0, 3e+02f, 2e+01f, 1e+03f, 1.0f) \
		p(HORIZONTALSLIDER, Mix, "Mix", fHslider3, 1.0f, 0.0f, 1.0f, 0.01f) \
		p(HORIZONTALSLIDER, window, "Pitch Shifter/window", fHslider1, 1e+03f, 5e+01f, 1e+04f, 1.0f) \
		p(HORIZONTALSLIDER, xfade, "Pitch Shifter/xfade", fHslider2, 16.0f, 1.0f, 1e+04f, 1.0f) \

	#define FAUST_LIST_PASSIVES(p) \

#endif

#endif
