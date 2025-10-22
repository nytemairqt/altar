// Third Party Node Template

#pragma once
#include <JuceHeader.h>

namespace project
{
using namespace juce;
using namespace hise;
using namespace scriptnode;

template <int NV> struct cabDesigner: public data::base
{
    SNEX_NODE(cabDesigner);

    struct MetadataClass
    {
        SN_NODE_ID("cabDesigner");
    };

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

    // ----------------------------- EQ module definitions -----------------------------

    enum FilterType { HighPass, LowPass, Peak, HighShelf, LowShelf };

    struct FilterSpec
    {
        FilterType type { Peak };
        float frequency { 1000.0f };
        float gainDB { 0.0f };
        float Q { 0.707f };
        bool enabled { true };
    };

    static constexpr int maxFiltersPerModule = 16;
    static constexpr int maxSpeakerModules = 16;
    static constexpr int maxMicModules = 16;

    struct EQModule
    {
        FilterSpec filters[maxFiltersPerModule];
        int count { 0 };

        void set(const FilterSpec* specs, int n)
        {
            count = jlimit(0, maxFiltersPerModule, n);
            for (int i = 0; i < count; ++i) filters[i] = specs[i];
            for (int i = count; i < maxFiltersPerModule; ++i) filters[i].enabled = false;
        }

        // Convenience builder for "just pass frequencies, gains (dB), Q" => all Peak filters
        static EQModule makeParametric(const float* freqs, const float* gains, const float* qs, int n)
        {
            EQModule m;
            m.count = jmin(n, maxFiltersPerModule);
            for (int i = 0; i < m.count; ++i)
            {
                m.filters[i] = { Peak, freqs[i], gains[i], qs[i], true };
            }
            for (int i = m.count; i < maxFiltersPerModule; ++i) m.filters[i].enabled = false;
            return m;
        }
    };

    // ----------------------------- DSP state -----------------------------

    struct BiquadState
    {
        float x1 = 0.0f, x2 = 0.0f;
        float y1 = 0.0f, y2 = 0.0f;
        float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f;
        float a1 = 0.0f, a2 = 0.0f;

        void reset() { x1 = x2 = y1 = y2 = 0.0f; }

        float process(float in)
        {
            const float out = b0 * in + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
            x2 = x1; x1 = in; y2 = y1; y1 = out;
            return out;
        }
    };

    // Processing order: Speaker -> Microphone -> CabAge -> Mojo
    BiquadState speakerStates[2][maxFiltersPerModule];
    BiquadState micStates[2][maxFiltersPerModule];
    int currentSpeakerCount = 0;
    int currentMicCount = 0;

    // CabAge shelves
    BiquadState cabLowShelf[2];
    BiquadState cabHighShelf[2];

    // Mojo
    static constexpr int numEQBands = 100;
    static constexpr float minFreq = 50.0f;
    static constexpr float maxFreq = 13000.0f;
    static constexpr float eqQ = 8.0f;
    static constexpr float maxGainRange = 2.0f; // dB
    BiquadState mojoStates[2][numEQBands];
    float frequencies[numEQBands];
    float currentGains[numEQBands];

    // Registries
    EQModule speakerModules[maxSpeakerModules];
    EQModule micModules[maxMicModules];
    int speakerModuleCount = 0;
    int micModuleCount = 0;

    // ----------------------------- Parameters -----------------------------

    float sampleRate = 44100.0f;

    float mojoStrength = 0.5f;
    bool lastGenerateTrigger = false;

    // Indices into registries
    int speakerType = 0;      // [0 .. speakerModuleCount-1]
    int micType = 0;          // [0 .. micModuleCount-1], type 0 = Flat

    // CabAge amount [0..1]
    float cabAge = 0.0f;

    // CabAge tuning
    static constexpr float cabLowFreq = 120.0f;
    static constexpr float cabHighFreq = 4000.0f;
    static constexpr float cabShelfQ = 0.707f;
    static constexpr float cabMaxHighCut = -12.0f; // dB
    static constexpr float cabMaxLowBoost = 2.5f;  // dB

    // ----------------------------- Lifecycle -----------------------------

    cabDesigner()
    {
        for (int i = 0; i < numEQBands; ++i)
        {
            const float r = (float)i / (float)(numEQBands - 1);
            frequencies[i] = minFreq * std::pow(maxFreq / minFreq, r);
            currentGains[i] = 0.0f;
        }

        // Default Speaker module (legacy "Type 0")
        {
            FilterSpec specs[] = {
                { HighPass,  55.0f,  0.0f, 0.707f, true },
                { Peak,     105.0f,  8.0f, 4.0f,   true },
                { Peak,     230.0f,  2.5f, 2.0f,   true },
                { Peak,     520.0f, -5.0f, 0.8f,   true },
                { Peak,    1100.0f,  2.5f, 4.0f,   true },
                { Peak,    1400.0f, -6.0f, 4.0f,   true },
                { Peak,    1700.0f,  3.0f, 4.0f,   true },
                { HighShelf,3000.0f,  6.0f, 0.707f,true },
                { LowPass,  5200.0f,  0.0f, 0.707f,true }
            };
            speakerModules[0].set(specs, (int)(sizeof(specs) / sizeof(FilterSpec)));
            speakerModuleCount = 1;
        }

        // Default Mic module index 0 = Flat (no filters)
        micModules[0].count = 0;
        micModuleCount = 1;

        generateRandomGains();
    }

    void prepare(PrepareSpecs specs)
    {
        sampleRate = (float)specs.sampleRate;

        for (int ch = 0; ch < 2; ++ch)
        {
            for (int i = 0; i < maxFiltersPerModule; ++i)
            {
                speakerStates[ch][i].reset();
                micStates[ch][i].reset();
            }
            cabLowShelf[ch].reset();
            cabHighShelf[ch].reset();

            for (int b = 0; b < numEQBands; ++b)
                mojoStates[ch][b].reset();
        }

        applySpeakerModule();
        applyMicModule();
        updateCabAge();
        updateMojoFilters();
    }

    void reset()
    {
        for (int ch = 0; ch < 2; ++ch)
        {
            for (int i = 0; i < maxFiltersPerModule; ++i)
            {
                speakerStates[ch][i].reset();
                micStates[ch][i].reset();
            }
            cabLowShelf[ch].reset();
            cabHighShelf[ch].reset();

            for (int b = 0; b < numEQBands; ++b)
                mojoStates[ch][b].reset();
        }
    }

    template <typename T> void process(T& data)
    {
        const int numSamples = data.getNumSamples();
        const int actualChannels = jmin(2, (int)data.getNumChannels());

        dyn<float> ch0, ch1;
        int ci = 0;
        for (auto ch : data)
        {
            if (ci == 0) ch0 = data.toChannelData(ch);
            else if (ci == 1) ch1 = data.toChannelData(ch);
            ++ci;
        }
        if (actualChannels == 1) ch1 = ch0;

        for (int s = 0; s < numSamples; ++s)
        {
            float x0 = ch0[s];
            float x1 = (actualChannels > 1) ? ch1[s] : x0;

            // Speaker
            for (int i = 0; i < currentSpeakerCount; ++i)
            {
                x0 = speakerStates[0][i].process(x0);
                if (actualChannels > 1) x1 = speakerStates[1][i].process(x1);
            }

            // Microphone
            for (int i = 0; i < currentMicCount; ++i)
            {
                x0 = micStates[0][i].process(x0);
                if (actualChannels > 1) x1 = micStates[1][i].process(x1);
            }

            // CabAge shelves
            x0 = cabLowShelf[0].process(x0);
            x0 = cabHighShelf[0].process(x0);
            if (actualChannels > 1)
            {
                x1 = cabLowShelf[1].process(x1);
                x1 = cabHighShelf[1].process(x1);
            }

            // Mojo (last)
            for (int b = 0; b < numEQBands; ++b)
            {
                x0 = mojoStates[0][b].process(x0);
                if (actualChannels > 1) x1 = mojoStates[1][b].process(x1);
            }

            ch0[s] = x0;
            if (actualChannels > 1) ch1[s] = x1;
        }
    }

    template <int P> void setParameter(double v)
    {
        switch (P)
        {
            case 0: // MojoStrength
                mojoStrength = (float)jlimit(0.0, 1.0, v);
                updateMojoFilters(); // keep behavior as-is
                break;

            case 1: // GenerateMojo (trigger)
            {
                const bool trig = (v > 0.5);
                if (trig && !lastGenerateTrigger)
                {
                    generateRandomGains();
                    updateMojoFilters();
                }
                lastGenerateTrigger = trig;
                break;
            }

            case 2: // SpeakerType
            {
                const int newIndex = (int)jlimit(0.0, (double)(maxSpeakerModules - 1), v);
                if (newIndex != speakerType)
                {
                    speakerType = newIndex;
                    applySpeakerModule();
                }
                break;
            }

            case 3: // MicrophoneType
            {
                const int newIndex = (int)jlimit(0.0, (double)(maxMicModules - 1), v);
                if (newIndex != micType)
                {
                    micType = newIndex;
                    applyMicModule();
                }
                break;
            }

            case 4: // CabAge
                cabAge = (float)jlimit(0.0, 1.0, v);
                updateCabAge();
                break;
        }
    }

    void createParameters(ParameterDataList& data)
    {
        {
            parameter::data p("MojoStrength", { 0.0, 1.0 });
            registerCallback<0>(p);
            p.setDefaultValue(0.5);
            data.add(std::move(p));
        }
        {
            parameter::data p("GenerateMojo", { 0.0, 1.0 });
            registerCallback<1>(p);
            p.setDefaultValue(0.0);
            data.add(std::move(p));
        }
        {
            parameter::data p("SpeakerType", { 0.0, (double)(maxSpeakerModules - 1) });
            registerCallback<2>(p);
            p.setDefaultValue(0.0);
            data.add(std::move(p));
        }
        {
            parameter::data p("MicrophoneType", { 0.0, (double)(maxMicModules - 1) });
            registerCallback<3>(p);
            p.setDefaultValue(0.0); // 0 = Flat mic
            data.add(std::move(p));
        }
        {
            parameter::data p("CabAge", { 0.0, 1.0 });
            registerCallback<4>(p);
            p.setDefaultValue(0.0);
            data.add(std::move(p));
        }
    }

    // ----------------------------- Public helpers -----------------------------
    // Add new modules easily. Call these in the constructor or from your integration code.

    int addSpeakerModule(const EQModule& m)
    {
        const int idx = jmin(speakerModuleCount, maxSpeakerModules - 1);
        speakerModules[idx] = m;
        speakerModuleCount = jmin(idx + 1, maxSpeakerModules);
        return idx;
    }

    int addMicModule(const EQModule& m)
    {
        const int idx = jmin(micModuleCount, maxMicModules - 1);
        micModules[idx] = m;
        micModuleCount = jmin(idx + 1, maxMicModules);
        return idx;
    }

private:
    // ----------------------------- Filter designs -----------------------------

    static inline void setNormCoeffs(BiquadState& st, float b0, float b1, float b2, float a0, float a1, float a2)
    {
        const float invA0 = 1.0f / a0;
        st.b0 = b0 * invA0;
        st.b1 = b1 * invA0;
        st.b2 = b2 * invA0;
        st.a1 = a1 * invA0;
        st.a2 = a2 * invA0;
    }

    void designPeak(BiquadState& st, float freq, float Q, float gainDB)
    {
        const float w = 2.0f * MathConstants<float>::pi * freq / sampleRate;
        const float c = std::cos(w), s = std::sin(w);
        const float A = std::pow(10.0f, gainDB / 40.0f);
        const float alpha = s / (2.0f * Q);

        const float b0 = 1.0f + alpha * A;
        const float b1 = -2.0f * c;
        const float b2 = 1.0f - alpha * A;
        const float a0 = 1.0f + alpha / A;
        const float a1 = -2.0f * c;
        const float a2 = 1.0f - alpha / A;

        setNormCoeffs(st, b0, b1, b2, a0, a1, a2);
    }

    void designHighPass(BiquadState& st, float freq, float Q)
    {
        const float w = 2.0f * MathConstants<float>::pi * freq / sampleRate;
        const float c = std::cos(w), s = std::sin(w);
        const float alpha = s / (2.0f * Q);

        const float b0 = (1.0f + c) * 0.5f;
        const float b1 = -(1.0f + c);
        const float b2 = (1.0f + c) * 0.5f;
        const float a0 = 1.0f + alpha;
        const float a1 = -2.0f * c;
        const float a2 = 1.0f - alpha;

        setNormCoeffs(st, b0, b1, b2, a0, a1, a2);
    }

    void designLowPass(BiquadState& st, float freq, float Q)
    {
        const float w = 2.0f * MathConstants<float>::pi * freq / sampleRate;
        const float c = std::cos(w), s = std::sin(w);
        const float alpha = s / (2.0f * Q);

        const float b0 = (1.0f - c) * 0.5f;
        const float b1 = 1.0f - c;
        const float b2 = (1.0f - c) * 0.5f;
        const float a0 = 1.0f + alpha;
        const float a1 = -2.0f * c;
        const float a2 = 1.0f - alpha;

        setNormCoeffs(st, b0, b1, b2, a0, a1, a2);
    }

    void designHighShelf(BiquadState& st, float freq, float Q, float gainDB)
    {
        const float w = 2.0f * MathConstants<float>::pi * freq / sampleRate;
        const float c = std::cos(w), s = std::sin(w);
        const float A = std::pow(10.0f, gainDB / 40.0f);
        const float beta = std::sqrt(A) / Q;

        const float b0 = A * ((A + 1.0f) + (A - 1.0f) * c + beta * s);
        const float b1 = -2.0f * A * ((A - 1.0f) + (A + 1.0f) * c);
        const float b2 = A * ((A + 1.0f) + (A - 1.0f) * c - beta * s);
        const float a0 = (A + 1.0f) - (A - 1.0f) * c + beta * s;
        const float a1 = 2.0f * ((A - 1.0f) - (A + 1.0f) * c);
        const float a2 = (A + 1.0f) - (A - 1.0f) * c - beta * s;

        setNormCoeffs(st, b0, b1, b2, a0, a1, a2);
    }

    void designLowShelf(BiquadState& st, float freq, float Q, float gainDB)
    {
        const float w = 2.0f * MathConstants<float>::pi * freq / sampleRate;
        const float c = std::cos(w), s = std::sin(w);
        const float A = std::pow(10.0f, gainDB / 40.0f);
        const float beta = std::sqrt(A) / Q;

        const float b0 = A * ((A + 1.0f) - (A - 1.0f) * c + beta * s);
        const float b1 = 2.0f * A * ((A - 1.0f) - (A + 1.0f) * c);
        const float b2 = A * ((A + 1.0f) - (A - 1.0f) * c - beta * s);
        const float a0 = (A + 1.0f) + (A - 1.0f) * c + beta * s;
        const float a1 = -2.0f * ((A - 1.0f) + (A + 1.0f) * c);
        const float a2 = (A + 1.0f) + (A - 1.0f) * c - beta * s;

        setNormCoeffs(st, b0, b1, b2, a0, a1, a2);
    }

    // ----------------------------- Module application -----------------------------

    void applyEQModuleToStates(const EQModule& mod, BiquadState dst[2][maxFiltersPerModule], int& outCount)
    {
        outCount = jlimit(0, maxFiltersPerModule, mod.count);

        for (int i = 0; i < outCount; ++i)
        {
            const auto& f = mod.filters[i];
            for (int ch = 0; ch < 2; ++ch)
            {
                switch (f.type)
                {
                    case HighPass:  designHighPass(dst[ch][i], f.frequency, f.Q); break;
                    case LowPass:   designLowPass(dst[ch][i],  f.frequency, f.Q); break;
                    case Peak:      designPeak(dst[ch][i],     f.frequency, f.Q, f.gainDB); break;
                    case HighShelf: designHighShelf(dst[ch][i],f.frequency, f.Q, f.gainDB); break;
                    case LowShelf:  designLowShelf(dst[ch][i], f.frequency, f.Q, f.gainDB); break;
                }
            }
        }
        for (int i = outCount; i < maxFiltersPerModule; ++i)
        {
            for (int ch = 0; ch < 2; ++ch)
            {
                dst[ch][i].b0 = 1.0f; dst[ch][i].b1 = dst[ch][i].b2 = 0.0f;
                dst[ch][i].a1 = dst[ch][i].a2 = 0.0f;
            }
        }
    }

    void applySpeakerModule()
    {
        const int idx = jlimit(0, jmax(0, speakerModuleCount - 1), speakerType);
        applyEQModuleToStates(speakerModules[idx], speakerStates, currentSpeakerCount);
    }

    void applyMicModule()
    {
        const int idx = jlimit(0, jmax(0, micModuleCount - 1), micType);
        applyEQModuleToStates(micModules[idx], micStates, currentMicCount);
    }

    // ----------------------------- CabAge -----------------------------

    void updateCabAge()
    {
        const float highCut = cabAge * cabMaxHighCut;   // negative
        const float lowBoost = cabAge * cabMaxLowBoost; // positive

        for (int ch = 0; ch < 2; ++ch)
        {
            designLowShelf(cabLowShelf[ch],  cabLowFreq,  cabShelfQ, lowBoost);
            designHighShelf(cabHighShelf[ch], cabHighFreq, cabShelfQ, highCut);
        }
    }

    // ----------------------------- Mojo -----------------------------

    void generateRandomGains()
    {
        Random& rng = Random::getSystemRandom();
        for (int i = 0; i < numEQBands; ++i)
        {
            const float r = rng.nextFloat() * 2.0f - 1.0f;
            currentGains[i] = r * maxGainRange * mojoStrength;
        }
    }

    void updateMojoFilters()
    {
        for (int band = 0; band < numEQBands; ++band)
        {
            const float freq = frequencies[band];
            const float gainDB = currentGains[band];

            const float w = 2.0f * MathConstants<float>::pi * freq / sampleRate;
            const float c = std::cos(w), s = std::sin(w);
            const float A = std::pow(10.0f, gainDB / 40.0f);
            const float alpha = s / (2.0f * eqQ);

            const float b0 = 1.0f + alpha * A;
            const float b1 = -2.0f * c;
            const float b2 = 1.0f - alpha * A;
            const float a0 = 1.0f + alpha / A;
            const float a1 = -2.0f * c;
            const float a2 = 1.0f - alpha / A;

            for (int ch = 0; ch < 2; ++ch)
            {
                setNormCoeffs(mojoStates[ch][band], b0, b1, b2, a0, a1, a2);
            }
        }
    }
};
}