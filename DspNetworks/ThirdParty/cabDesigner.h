// HISE Third Party Node: Cab Designer (simplified)

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
            mojoGainDB[i] = 0.0f;
        }

        // Instantiate Speakers & Microphones
        addSpeaker(speakerA); addSpeaker(speakerB); addSpeaker(speakerC); addSpeaker(speakerD); addSpeaker(speakerE); addSpeaker(speakerF);              
        addMic(micA); addMic(micB); addMic(micC); addMic(micD);addMic(micE); addMic(micF); addMic(micG); addMic(micH);

        generateRandomMojo();
    }

    void prepare(PrepareSpecs specs)
    {
        sr = (float)specs.sampleRate;

        for (int ch = 0; ch < 2; ++ch)
        {
            for (int i = 0; i < 16; ++i) { spState[ch][i].reset(); micState[ch][i].reset(); }
            for (int b = 0; b < kMojoBands; ++b) mojoState[ch][b].reset();
            lowShelf[ch].reset(); highShelf[ch].reset();
        }

        applySpeaker();
        applyMic();
        updateMojo();
        updateCabAge();
    }

    template <typename T>
    void process(T& data)
    {
        const int nS = data.getNumSamples();
        const int nC = jmin(2, (int)data.getNumChannels());

        dyn<float> c0, c1;
        int idx = 0;
        for (auto ch : data)
        {
            if      (idx == 0) c0 = data.toChannelData(ch);
            else if (idx == 1) c1 = data.toChannelData(ch);
            ++idx;
        }
        if (nC == 1) c1 = c0;

        for (int s = 0; s < nS; ++s)
        {
            float x0 = c0[s];
            float x1 = (nC > 1 ? c1[s] : x0);

            // 1) Speaker
            for (int i = 0; i < spCount; ++i) { x0 = spState[0][i].process(x0); if (nC > 1) x1 = spState[1][i].process(x1); }

            // 2) Microphone
            for (int i = 0; i < micCount; ++i) { x0 = micState[0][i].process(x0); if (nC > 1) x1 = micState[1][i].process(x1); }

            // 3) Mojo
            for (int b = 0; b < kMojoBands; ++b) { x0 = mojoState[0][b].process(x0); if (nC > 1) x1 = mojoState[1][b].process(x1); }

            // 4) Cab Age
            x0 = lowShelf[0].process(x0); x0 = highShelf[0].process(x0);
            if (nC > 1) { x1 = lowShelf[1].process(x1); x1 = highShelf[1].process(x1); }

            c0[s] = x0;
            if (nC > 1) c1[s] = x1;
        }
    }

    void reset() {}

    template <int P>
    void setParameter(double v)
    {
        switch (P)
        {
            case 0: { const int ni = (int)jlimit(0.0, (double)(spModules - 1), v); if (ni != spIndex) { spIndex = ni; applySpeaker(); } break; }
            case 1: { const int ni = (int)jlimit(0.0, (double)(micModules - 1), v); if (ni != micIndex) { micIndex = ni; applyMic(); } break; }
            case 2: mojoStrength = (float)jlimit(0.0, 1.0, v); updateMojo(); break;
            case 3: { const bool trig = (v > 0.5); if (trig && !genTrig) { generateRandomMojo(); updateMojo(); } genTrig = trig; break; }
            case 4: cabAge = (float)jlimit(0.0, 1.0, v); updateCabAge(); break;
        }
    }

    void createParameters(ParameterDataList& data)
    {
        { parameter::data p("SpeakerType", { 0.0, (double)(spModules ? spModules - 1 : 0) }); registerCallback<0>(p); p.setDefaultValue(0.0); data.add(std::move(p)); }
        { parameter::data p("MicrophoneType", { 0.0, (double)(micModules ? micModules - 1 : 0) }); registerCallback<1>(p); p.setDefaultValue(0.0); data.add(std::move(p)); }
        { parameter::data p("MojoStrength", { 0.0, 1.0 }); registerCallback<2>(p); p.setDefaultValue(0.5); data.add(std::move(p)); }
        { parameter::data p("GenerateMojo", { 0.0, 1.0 }); registerCallback<3>(p); p.setDefaultValue(0.0); data.add(std::move(p)); }
        { parameter::data p("CabAge", { 0.0, 1.0 }); registerCallback<4>(p); p.setDefaultValue(0.0); data.add(std::move(p)); }
    }

    // Simple adders from compile-time arrays
    template <size_t N> int addSpeaker(const FilterSpec (&arr)[N]) { EQModule m; m.set(arr, (int)N); return addSpeakerModule(m); }
    template <size_t N> int addMic(const FilterSpec (&arr)[N])     { EQModule m; m.set(arr, (int)N); return addMicModule(m); }

private:
    // Small biquad
    struct Biquad
    {
        float x1 = 0.0f, x2 = 0.0f, y1 = 0.0f, y2 = 0.0f;
        float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f, a1 = 0.0f, a2 = 0.0f;
        void reset() { x1 = x2 = y1 = y2 = 0.0f; }
        float process(float in) { const float out = b0 * in + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2; x2 = x1; x1 = in; y2 = y1; y1 = out; return out; }
    };

    // Private utility
    static constexpr FilterSpec FS(FilterType t, float f, float g, float q) { return { t, f, g, q, true }; }
    static inline void setNorm(Biquad& st, float b0, float b1, float b2, float a0, float a1, float a2)
    {
        const float ia0 = 1.0f / a0;
        st.b0 = b0 * ia0; st.b1 = b1 * ia0; st.b2 = b2 * ia0; st.a1 = a1 * ia0; st.a2 = a2 * ia0;
    }

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
            mojoGainDB[i] = r * kMojoMaxDb * mojoStrength;
        }
    }

    void updateMojo()
    {
        for (int i = 0; i < kMojoBands; ++i)
        {
            const float f = mojoFreq[i];
            const float g = mojoGainDB[i];
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

    // Limits and state
    EQModule sp[16], mic[16];     
    Biquad spState[2][16], micState[2][16]; 
    int spModules = 0, micModules = 0; 
    int spIndex = 0, micIndex = 0;
    int spCount = 0, micCount = 0;

    static constexpr int   kMojoBands = 200;
    static constexpr float kMojoMin   = 50.0f;
    static constexpr float kMojoMax   = 13000.0f;
    static constexpr float kMojoQ     = 8.0f;
    static constexpr float kMojoMaxDb = 3.5f;

    Biquad mojoState[2][kMojoBands];
    float  mojoFreq[kMojoBands];
    float  mojoGainDB[kMojoBands];
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

    // ---------------------- Module definitions ----------------------
    // Speakers

    // AL30 Modded
    inline static const FilterSpec speakerA[] = {
        FS(HighPass,    48.0f,      0.0f,    0.707f),
        FS(HighPass,    48.0f,      0.0f,    0.707f),
        FS(LowShelf,    90.0f,      4.0f,    1.0f),
        FS(Peak,        200.0f,    -4.0f,    3.4f),
        FS(Peak,        800.0f,    -3.0f,    4.0f),
        FS(Peak,        1300.0f,   -7.0f,    6.0f),
        FS(Peak,        1600.0f,   -8.0f,    6.0f),
        FS(Peak,        2400.0f,    5.0f,   5.0f),
        FS(HighShelf,   3500.0f,    3.0f,    1.0f),
        FS(LowPass,     5500.0f,    0.0f,    0.707f),
        FS(LowPass,     5500.0f,    0.0f,    0.707f),
        FS(LowPass,     5500.0f,    0.0f,    0.707f)
    };
    
    // AL30
    inline static const FilterSpec speakerB[] = {
        FS(HighPass,    48.0f,      0.0f,    0.707f),
        FS(HighPass,    48.0f,      0.0f,    0.707f),        
        FS(Peak,        200.0f,    -4.0f,    3.4f),
        FS(Peak,        800.0f,    -3.0f,    4.0f),
        FS(Peak,        1300.0f,   -7.0f,    6.0f),
        FS(Peak,        1600.0f,   -8.0f,    6.0f),
        FS(Peak,        2400.0f,    5.0f,   5.0f),
        FS(HighShelf,   3500.0f,    5.0f,    1.0f),
        FS(LowPass,     5500.0f,    0.0f,    0.707f),
        FS(LowPass,     5500.0f,    0.0f,    0.707f),
        FS(LowPass,     5500.0f,    0.0f,    0.707f)
    };

    // ALK100
    inline static const FilterSpec speakerC[] = {
        FS(HighPass,    60.0f,      0.0f,    0.707f),
        FS(HighPass,    60.0f,      0.0f,    0.707f),    
        FS(Peak,        206.0f,    -3.0f,    4.0f),
        FS(Peak,        800.0f,    -4.0f,    4.0f),
        FS(Peak,        960.0f,     3.0f,    4.0f),
        FS(Peak,        1400.0f,   -4.0f,    4.0f),
        FS(Peak,        1600.0f,    3.0f,    4.0f),
        FS(Peak,        1800.0f,   -5.0f,    4.0f),
        FS(Peak,        2500.0f,    3.5f,    3.0f),
        FS(Peak,        3000.0f,   -5.0f,    4.0f),
        FS(Peak,        3500.0f,    3.0f,    4.0f),
        FS(LowPass,     6400.0f,    0.0f,    0.707f),
        FS(LowPass,     6400.0f,    0.0f,    0.707f),
        FS(LowPass,     6400.0f,    0.0f,    0.707f)        
    };

    // AL65
    inline static const FilterSpec speakerD[] = {
        FS(HighPass,    60.0f,      0.0f,    0.707f),
        FS(HighPass,    60.0f,      0.0f,    0.707f),    
        FS(Peak,        750.0f,     2.0f,    0.7f),
        FS(Peak,        1500.0f,   -5.0f,    3.0f),
        FS(Peak,        2400.0f,    4.0f,    4.0f),
        FS(Peak,        3200.0f,    3.5f,    3.5f),
        FS(Peak,        4500.0f,   -3.0f,    3.0f),
        FS(HighShelf,   5000.0f,   -10.0f,   3.0f),
        FS(LowPass,     6000.0f,    0.0f,    0.707f),
        FS(LowPass,     6000.0f,    0.0f,    0.707f),
        FS(LowPass,     6000.0f,    0.0f,    0.707f),
    };

    // AL77
    inline static const FilterSpec speakerE[] = {
        FS(HighPass,    80.0f,      0.0f,    0.707f),
        FS(HighPass,    80.0f,      0.0f,    0.707f),    
        FS(Peak,        230.0f,    -2.0f,    3.0f),
        FS(Peak,        320.0f,    -2.0f,    3.0f),
        FS(Peak,        600.0f,     3.0f,    3.0f),
        FS(Peak,        900.0f,    -1.0f,    5.0f),
        FS(Peak,        1500.0f,   -4.0f,    3.0f),
        FS(Peak,        2200.0f,    3.0f,    3.0f),
        FS(HighShelf,   2200.0f,    5.0f,    1.0f),
        FS(LowPass,     5800.0f,    0.0f,    0.707f),
        FS(LowPass,     6400.0f,    0.0f,    0.707f),
        FS(LowPass,     6400.0f,    0.0f,    0.707f),
    };

    // AL12M
    inline static const FilterSpec speakerF[] = {
        FS(HighPass,    60.0f,      0.0f,    0.707f),
        FS(HighPass,    60.0f,      0.0f,    0.707f),    
        FS(Peak,        600.0f,     2.0f,    0.7f),
        FS(Peak,        1500.0f,   -5.0f,    3.0f),
        FS(Peak,        2500.0f,    6.0f,    3.0f),
        FS(Peak,        3300.0f,    4.0f,    3.0f),
        FS(Peak,        4500.0f,   -3.0f,    3.0f),
        FS(LowPass,     5800.0f,    0.0f,    0.707f),
        FS(LowPass,     6400.0f,    0.0f,    0.707f),
        FS(LowPass,     6400.0f,    0.0f,    0.707f),
    };

    // Microphones

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

    // AL184
    inline static const FilterSpec micE[] = {
        FS(HighPass,    30.0f,      0.0f,   0.707f),    
        FS(LowShelf,    60.0f,      2.0f,   1.0f),    
        FS(Peak,        500.0f,    -2.0f,   0.5f),        
        FS(Peak,        1300.0f,    1.5f,   0.5f),   
        FS(HighShelf,   13500.0f,  -3.0f,   1.0f),
        FS(LowPass,     18000.0f,   0.0f,   0.707f),
        FS(LowPass,     18000.0f,   0.0f,   0.707f),
    };
};
}