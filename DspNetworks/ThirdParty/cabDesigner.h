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

#pragma once
#include <JuceHeader.h>

namespace project
{
using namespace juce;
using namespace hise;
using namespace scriptnode;

template <int NV>
struct cabDesigner : public data::base
{
    SNEX_NODE(cabDesigner);

    struct MetadataClass { SN_NODE_ID("cabDesigner"); };

    static constexpr bool isModNode() { return false; }
    static constexpr bool isPolyphonic() { return NV > 1; }
    static constexpr bool hasTail() { return false; }
    static constexpr bool isSuspendedOnSilence() { return false; }
    static constexpr int getFixChannelAmount() { return 2; }
    static constexpr int NumTables = 0;
    static constexpr int NumSliderPacks = 0;
    static constexpr int NumAudioFiles = 0;
    static constexpr int NumFilters = 0;
    static constexpr int NumDisplayBuffers = 0;
    
    static constexpr int kNumSpeakers = 6;
    static constexpr int kNumMics     = 6;

    int handleModulation(double&) { return 0; }
    template <typename T> void processFrame(T&) {}
    void handleHiseEvent(HiseEvent&) {}
    void setExternalData(const ExternalData&, int) {}

    enum FilterType { HighPass, LowPass, Peak, HighShelf, LowShelf };

    struct FilterSpec
    {
        FilterType type { Peak };
        float freq { 1000.0f };
        float gainDB { 0.0f };
        float Q { 0.707f };
        bool enabled { true };
    };

    struct EQModule
    {
        FilterSpec filters[16];
        int count { 0 };
        void set(const FilterSpec* specs, int n)
        {
            count = jlimit(0, 16, n);
            for (int i = 0; i < count; ++i) filters[i] = specs[i];
            for (int i = count; i < 16; ++i) filters[i].enabled = false;
        }
    };

    cabDesigner()
    {
        for (int i = 0; i < kMojoBands; ++i)
        {
            const float r = (float)i / (float)(kMojoBands - 1);
            mojoFreq[i] = kMojoMin * std::pow(kMojoMax / kMojoMin, r);
            mojoSeed[i] = 0.0f; // will be randomized in generateRandomMojo()
        }        
        {
            EQModule linear; 
            addSpeakerModule(linear);
        }
        {
            EQModule linear; 
            addMicModule(linear);
        }
        
        addSpeaker(speakerA); addSpeaker(speakerB); addSpeaker(speakerC); addSpeaker(speakerD); addSpeaker(speakerE);             
        addMic(micA); addMic(micB); addMic(micC); addMic(micD); addMic(micE);

        generateRandomMojo();
    }

    void prepare(PrepareSpecs specs)
    {
        sr = (float)specs.sampleRate;

        for (int ch = 0; ch < 2; ++ch)
        {
            for (int i = 0; i < 16; ++i) { spState[ch][i].reset(); micState[ch][i].reset(); cmState[ch][i].reset(); }
            for (int b = 0; b < kMojoBands; ++b) mojoState[ch][b].reset();
            lowShelf[ch].reset(); highShelf[ch].reset();
        }

        applySpeaker();
        applyCustomMod();
        applyMic();
        updateMojo();
        updateCabAge();
    }

    template <typename T>
    void process(T& data)
    {
        const int numSamples = data.getNumSamples();
        const int numChannels = jmin(2, (int)data.getNumChannels());
        
        dyn<float> chL, chR;
        int idx = 0;
        for (auto ch : data)
        {
            if      (idx == 0) chL = data.toChannelData(ch);
            else if (idx == 1) chR = data.toChannelData(ch);
            ++idx;
        }
        if (numChannels == 1) chR = chL;

        for (int s = 0; s < numSamples; ++s)
        {
            float xL = chL[s];
            float xR = (numChannels > 1 ? chR[s] : xL);

            // speaker
            for (int i = 0; i < spCount; ++i) { xL = spState[0][i].process(xL); if (numChannels > 1) xR = spState[1][i].process(xR); }

            // custom mod (mix-ready on gui)
            if (cmEnabled)
            {
                for (int i = 0; i < cmCount; ++i) { xL = cmState[0][i].process(xL); if (numChannels > 1) xR = cmState[1][i].process(xR); }
            }

            // mic
            for (int i = 0; i < micCount; ++i) { xL = micState[0][i].process(xL); if (numChannels > 1) xR = micState[1][i].process(xR); }

            // mojo
            for (int b = 0; b < kMojoBands; ++b) { xL = mojoState[0][b].process(xL); if (numChannels > 1) xR = mojoState[1][b].process(xR); }

            // cab age
            xL = lowShelf[0].process(xL); xL = highShelf[0].process(xL);
            if (numChannels > 1) { xR = lowShelf[1].process(xR); xR = highShelf[1].process(xR); }

            chL[s] = xL;
            if (numChannels > 1) chR[s] = xR;
        }
    }

    void reset() {}

    template <int P>
    void setParameter(double v)
    {
        switch (P)
        {
            case 0: { const int ni = (int)jlimit(0.0, (double)(spModules - 1), v); if (ni != spIndex) { spIndex = ni; applySpeaker(); } break; }
            case 1: { const bool en = (v > 0.5); if (en != cmEnabled) { cmEnabled = en; applyCustomMod(); } break; }
            case 2: { const int ni = (int)jlimit(0.0, (double)(micModules - 1), v); if (ni != micIndex) { micIndex = ni; applyMic(); } break; }
            case 3: mojoStrength = (float)jlimit(0.0, 1.0, v); updateMojo(); break; 
            case 4: { const bool trig = (v > 0.5); if (trig && !genTrig) { generateRandomMojo(); updateMojo(); } genTrig = trig; break; }
            case 5: cabAge = (float)jlimit(0.0, 1.0, v); updateCabAge(); break;
        }
    }

    void createParameters(ParameterDataList& data)
    {        
        { 
            parameter::data p("SpeakerType", { 0.0, (double)(kNumSpeakers - 1) });   
            registerCallback<0>(p); 
            p.setDefaultValue(0.0); 
            data.add(std::move(p)); 
        }
        { 
            parameter::data p("MixReady", { 0.0, 1.0 });                          
            registerCallback<1>(p); 
            p.setDefaultValue(1.0); 
            data.add(std::move(p)); 
        }
        { 
            parameter::data p("MicrophoneType", { 0.0, (double)(kNumMics - 1) });       
            registerCallback<2>(p); 
            p.setDefaultValue(0.0); 
            data.add(std::move(p)); 
        }
        { 
            parameter::data p("MojoStrength", { 0.0, 1.0 });                          
            registerCallback<3>(p); 
            p.setDefaultValue(0.5); 
            data.add(std::move(p)); 
        }
        { 
            parameter::data p("GenerateMojo", { 0.0, 1.0 });                          
            registerCallback<4>(p); 
            p.setDefaultValue(0.0); 
            data.add(std::move(p)); 
        }
        { 
            parameter::data p("CabAge", { 0.0, 1.0 });                          
            registerCallback<5>(p); 
            p.setDefaultValue(0.0); 
            data.add(std::move(p)); 
        }        
    }
    
    template <size_t N> int addSpeaker(const FilterSpec (&arr)[N]) { EQModule m; m.set(arr, (int)N); return addSpeakerModule(m); }
    template <size_t N> int addMic(const FilterSpec (&arr)[N])     { EQModule m; m.set(arr, (int)N); return addMicModule(m); }

private:

    struct Biquad
    {
        float x1 = 0.0f, x2 = 0.0f, y1 = 0.0f, y2 = 0.0f;
        float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f, a1 = 0.0f, a2 = 0.0f;
        void reset() { x1 = x2 = y1 = y2 = 0.0f; }
        float process(float in) { const float out = b0 * in + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2; x2 = x1; x1 = in; y2 = y1; y1 = out; return out; }
    };
    
    static constexpr FilterSpec FS(FilterType t, float f, float g, float q) { return { t, f, g, q, true }; }
    static inline void setNorm(Biquad& st, float b0, float b1, float b2, float a0, float a1, float a2)
    {
        const float ia0 = 1.0f / a0;
        st.b0 = b0 * ia0; st.b1 = b1 * ia0; st.b2 = b2 * ia0; st.a1 = a1 * ia0; st.a2 = a2 * ia0;
    }

    // more clanker butterworths
    void designPeak(Biquad& st, float f, float Q, float gDB)
    {
        const float w = 2.0f * MathConstants<float>::pi * f / sr;
        const float c = std::cos(w), s = std::sin(w);
        const float A = std::pow(10.0f, gDB / 40.0f);
        const float a = s / (2.0f * Q);
        setNorm(st, 1.0f + a * A, -2.0f * c, 1.0f - a * A, 1.0f + a / A, -2.0f * c, 1.0f - a / A);
    }

    void designHP(Biquad& st, float f, float Q)
    {
        const float w = 2.0f * MathConstants<float>::pi * f / sr;
        const float c = std::cos(w), s = std::sin(w);
        const float a = s / (2.0f * Q);
        setNorm(st, (1.0f + c) * 0.5f, -(1.0f + c), (1.0f + c) * 0.5f, 1.0f + a, -2.0f * c, 1.0f - a);
    }

    void designLP(Biquad& st, float f, float Q)
    {
        const float w = 2.0f * MathConstants<float>::pi * f / sr;
        const float c = std::cos(w), s = std::sin(w);
        const float a = s / (2.0f * Q);
        setNorm(st, (1.0f - c) * 0.5f, 1.0f - c, (1.0f - c) * 0.5f, 1.0f + a, -2.0f * c, 1.0f - a);
    }

    void designHShelf(Biquad& st, float f, float Q, float gDB)
    {
        const float w = 2.0f * MathConstants<float>::pi * f / sr;
        const float c = std::cos(w), s = std::sin(w);
        const float A = std::pow(10.0f, gDB / 40.0f);
        const float b = std::sqrt(A) / Q;
        setNorm(st,
            A * ((A + 1.0f) + (A - 1.0f) * c + b * s),
           -2.0f * A * ((A - 1.0f) + (A + 1.0f) * c),
            A * ((A + 1.0f) + (A - 1.0f) * c - b * s),
            (A + 1.0f) - (A - 1.0f) * c + b * s,
            2.0f * ((A - 1.0f) - (A + 1.0f) * c),
            (A + 1.0f) - (A - 1.0f) * c - b * s
        );
    }

    void designLShelf(Biquad& st, float f, float Q, float gDB)
    {
        const float w = 2.0f * MathConstants<float>::pi * f / sr;
        const float c = std::cos(w), s = std::sin(w);
        const float A = std::pow(10.0f, gDB / 40.0f);
        const float b = std::sqrt(A) / Q;
        setNorm(st,
            A * ((A + 1.0f) - (A - 1.0f) * c + b * s),
            2.0f * A * ((A - 1.0f) - (A + 1.0f) * c),
            A * ((A + 1.0f) - (A - 1.0f) * c - b * s),
            (A + 1.0f) + (A - 1.0f) * c + b * s,
           -2.0f * ((A - 1.0f) + (A + 1.0f) * c),
            (A + 1.0f) + (A - 1.0f) * c - b * s
        );
    }

    void applyEQ(const EQModule& m, Biquad dst[2][16], int& outCount)
    {
        outCount = jlimit(0, 16, m.count);
        for (int i = 0; i < outCount; ++i)
        {
            const auto& f = m.filters[i];
            for (int ch = 0; ch < 2; ++ch)
            {
                switch (f.type)
                {
                    case HighPass:  designHP(dst[ch][i],      f.freq, f.Q); break;
                    case LowPass:   designLP(dst[ch][i],      f.freq, f.Q); break;
                    case Peak:      designPeak(dst[ch][i],    f.freq, f.Q, f.gainDB); break;
                    case HighShelf: designHShelf(dst[ch][i],  f.freq, f.Q, f.gainDB); break;
                    case LowShelf:  designLShelf(dst[ch][i],  f.freq, f.Q, f.gainDB); break;
                }
            }
        }
        for (int i = outCount; i < 16; ++i)
            for (int ch = 0; ch < 2; ++ch) { dst[ch][i].b0 = 1.0f; dst[ch][i].b1 = dst[ch][i].b2 = 0.0f; dst[ch][i].a1 = dst[ch][i].a2 = 0.0f; }
    }

    void applySpeaker() { const int idx = jlimit(0, jmax(0, spModules - 1), spIndex); applyEQ(sp[idx], spState, spCount); }
    void applyMic()     { const int idx = jlimit(0, jmax(0, micModules - 1), micIndex); applyEQ(mic[idx], micState, micCount); }

    void applyCustomMod()
    {
        if (cmEnabled)
        {
            EQModule m;
            m.set(customMod, (int)(sizeof(customMod) / sizeof(customMod[0])));
            applyEQ(m, cmState, cmCount);
        }
        else
        {
            cmCount = 0;
            for (int i = 0; i < 16; ++i)
                for (int ch = 0; ch < 2; ++ch) { cmState[ch][i].b0 = 1.0f; cmState[ch][i].b1 = cmState[ch][i].b2 = 0.0f; cmState[ch][i].a1 = cmState[ch][i].a2 = 0.0f; }
        }
    }

    void updateCabAge()
    {
        const float lowBoost = cabAge * kCabMaxLowBoost;
        const float highCut  = cabAge * kCabMaxHighCut;
        for (int ch = 0; ch < 2; ++ch)
        {
            designLShelf(lowShelf[ch],  kCabLowFreq,  kCabShelfQ, lowBoost);
            designHShelf(highShelf[ch], kCabHighFreq, kCabShelfQ, highCut);
        }
    }
    
    void generateRandomMojo()
    {
        Random& rng = Random::getSystemRandom();
        for (int i = 0; i < kMojoBands; ++i)
        {
            const float r = rng.nextFloat() * 2.0f - 1.0f;
            mojoSeed[i] = r;
        }
    }

    void updateMojo()
    {
        for (int i = 0; i < kMojoBands; ++i)
        {
            const float f = mojoFreq[i];
            const float g = mojoSeed[i] * kMojoMaxDb * mojoStrength; // strength applied here so it reacts immediately
            const float w = 2.0f * MathConstants<float>::pi * f / sr;
            const float c = std::cos(w), s = std::sin(w);
            const float A = std::pow(10.0f, g / 40.0f);
            const float a = s / (2.0f * kMojoQ);

            const float b0 = 1.0f + a * A;
            const float b1 = -2.0f * c;
            const float b2 = 1.0f - a * A;
            const float a0 = 1.0f + a / A;
            const float a1 = -2.0f * c;
            const float a2 = 1.0f - a / A;

            for (int ch = 0; ch < 2; ++ch)
                setNorm(mojoState[ch][i], b0, b1, b2, a0, a1, a2);
        }
    }

    int addSpeakerModule(const EQModule& m)
    {
        const int idx = jmin(spModules, 16 - 1);
        sp[idx] = m; spModules = jmin(idx + 1, 16);
        return idx;
    }

    int addMicModule(const EQModule& m)
    {
        const int idx = jmin(micModules, 16 - 1);
        mic[idx] = m; micModules = jmin(idx + 1, 16);
        return idx;
    }
    
    EQModule sp[16], mic[16];     
    Biquad spState[2][16], micState[2][16]; 
    int spModules = 0, micModules = 0; 
    int spIndex = 0, micIndex = 0;
    int spCount = 0, micCount = 0;
    
    Biquad cmState[2][16];
    int    cmCount = 0;
    bool   cmEnabled = true;

    static constexpr int   kMojoBands = 200;
    static constexpr float kMojoMin   = 50.0f;
    static constexpr float kMojoMax   = 13000.0f;
    static constexpr float kMojoQ     = 8.0f;
    static constexpr float kMojoMaxDb = 3.5f;

    Biquad mojoState[2][kMojoBands];
    float  mojoFreq[kMojoBands];
    float  mojoSeed[kMojoBands];
    float  mojoStrength = 0.5f;
    bool   genTrig = false;

    Biquad lowShelf[2], highShelf[2];
    float  cabAge = 0.0f;

    static constexpr float kCabLowFreq     = 120.0f;
    static constexpr float kCabHighFreq    = 4000.0f;
    static constexpr float kCabShelfQ      = 0.707f;
    static constexpr float kCabMaxHighCut  = -12.0f;
    static constexpr float kCabMaxLowBoost = 2.5f;

    float sr = 44100.0f;
    
    // speakers
    
    // AL30
    inline static const FilterSpec speakerA[] = {
        FS(HighPass,    93.0f,      0.0f,    0.6f),        
        FS(Peak,        220.0f,    -4.5f,    6.5f),
        FS(Peak,        805.0f,    -2.8f,    6.0f),
        FS(Peak,        1200.0f,   -8.2f,    8.0f),
        FS(Peak,        1500.0f,   -10.0f,   8.0f),
        FS(Peak,        2200.0f,    6.8f,    8.0f),
        FS(Peak,        2400.0f,    3.5f,    4.2f),        
        FS(LowPass,     5000.0f,    0.0f,    0.707f),            
    };

    // ALK100
    inline static const FilterSpec speakerB[] = {        
        FS(HighPass,    100.0f,      0.0f,   0.6f),    
        FS(Peak,        350.0f,    -1.4f,    2.2f),
        FS(Peak,        700.0f,     0.9f,    1.0f),
        FS(Peak,        830.0f,    -1.6f,    3.7f),
        FS(Peak,        1400.0f,   -3.6f,    2.2f),
        FS(Peak,        2300.0f,    5.0f,    2.4f),
        FS(Peak,        3500.0f,    3.5f,    3.4f),        
        FS(LowPass,     5000.0f,    0.0f,    0.707f),        
    };

    // AL65
    inline static const FilterSpec speakerC[] = {        
        FS(HighPass,    97.0f,      0.0f,    0.6f),    
        FS(Peak,        404.0f,    -1.5f,    2.2f),        
        FS(Peak,        1400.0f,   -6.8f,    2.9f),
        FS(Peak,        2300.0f,    7.6f,    6.2f),
        FS(Peak,        3300.0f,    4.9f,    6.3f),
        FS(Peak,        5300.0f,    2.8f,    8.0f),
        FS(LowPass,     5000.0f,    0.0f,    0.707f),
    };

    // AL77
    inline static const FilterSpec speakerD[] = {
        FS(HighPass,    80.0f,      0.0f,    0.6f),        
        FS(Peak,        239.0f,    -1.1f,    4.4f),        
        FS(Peak,        342.0f,    -1.4f,    6.8f),    
        FS(Peak,        502.0f,     1.7f,    5.4f),    
        FS(Peak,        904.0f,    -1.0f,    8.0f),    
        FS(Peak,        1400.0f,   -3.6f,    4.2f),    
        FS(Peak,        2200.0f,    5.5f,    4.9f),    
        FS(Peak,        3000.0f,    4.4f,    5.2f),    
        FS(LowPass,     5000.0f,    0.0f,    0.707f),
    };

    // AL12M
    inline static const FilterSpec speakerE[] = {
        FS(HighPass,    60.0f,      0.0f,    0.6f),
        FS(Peak,        153.0f,     1.5f,    0.4f),
        FS(Peak,        402.0f,    -1.9f,    0.9f),
        FS(Peak,        797.0f,     1.8f,    1.0f),
        FS(Peak,        1600.0f,   -3.8f,    2.2f),
        FS(Peak,        2300.0f,    5.5f,    4.0f),
        FS(Peak,        3300.0f,    4.2f,    3.9f),
        FS(LowPass,     5000.0f,    0.0f,    0.707f),
    };

    // mics

    // AL57
    inline static const FilterSpec micA[] = {
        FS(HighPass,    130.0f,     0.0f,   0.707f),
        FS(HighPass,    130.0f,     0.0f,   0.707f),
        FS(Peak,        6000.0f,    6.0f,   1.0f),
        FS(LowPass,     14000.0f,   0.0f,   0.707f),
        FS(LowPass,     16000.0f,   0.0f,   0.707f),
    };

    // AL421
    inline static const FilterSpec micB[] = {
        FS(HighPass,    70.0f,      0.0f,   0.707f),
        FS(HighPass,    70.0f,      0.0f,   0.707f),
        FS(Peak,        4000.0f,    6.0f,   1.0f),
        FS(HighShelf,   10000.0f,   4.0f,   1.0f),
        FS(LowPass,     14000.0f,   0.0f,   0.707f),
        FS(LowPass,     16000.0f,   0.0f,   0.707f),
    };

    // AL414
    inline static const FilterSpec micC[] = {
        FS(HighPass,    80.0f,      0.0f,   0.707f),
        FS(HighPass,    80.0f,      0.0f,   0.707f),
        FS(Peak,        1200.0f,   -3.0f,   3.0f),
        FS(Peak,        6500.0f,    4.0f,   3.0f),
        FS(Peak,        13000.0f,   5.0f,   2.0f),
        FS(LowPass,     15000.0f,   0.0f,   0.707f),
        FS(LowPass,     16000.0f,   0.0f,   0.707f),
    };

    // AL184
    inline static const FilterSpec micD[] = {
        FS(HighPass,    60.0f,      0.0f,   0.707f),        
        FS(Peak,        8500.0f,    5.0f,   0.8f),        
        FS(LowPass,     16000.0f,   0.0f,   0.707f),
        FS(LowPass,     16000.0f,   0.0f,   0.707f),
    };

    // AL121
    inline static const FilterSpec micE[] = {
        FS(HighPass,    30.0f,      0.0f,   0.707f),    
        FS(LowShelf,    60.0f,      2.0f,   1.0f),    
        FS(Peak,        500.0f,    -2.0f,   0.5f),        
        FS(Peak,        1300.0f,    1.5f,   0.5f),   
        FS(HighShelf,   13500.0f,  -3.0f,   1.0f),
        FS(LowPass,     18000.0f,   0.0f,   0.707f),
        FS(LowPass,     18000.0f,   0.0f,   0.707f),
    };

    // custom mod / mix-ready
    inline static const FilterSpec customMod[] = {
        FS(HighPass,    50.0f,      0.0f,   0.707f),
        FS(HighPass,    50.0f,      0.0f,   0.707f),        
        FS(Peak,        123.0f,     8.0f,   0.4f),  
        FS(Peak,        200.0f,     4.0f,   0.3f),  
        FS(Peak,        498.0f,    -4.4f,   4.9f),  
        FS(Peak,        2400.0f,   -3.6f,   2.9f),  
        FS(Peak,        3900.0f,    2.9f,   0.6f), 
        FS(Peak,        4500.0f,    7.7f,   0.7f),          
        FS(LowPass,     5000.0f,    0.0f,   0.7f),        
        FS(LowPass,     5000.0f,    0.0f,   0.7f),        
        FS(LowPass,     5000.0f,    0.0f,   0.7f),        
    };
};
}