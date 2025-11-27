// FIX ME: add GPL license to all .h files

#pragma once
#include <JuceHeader.h>
#include <algorithm>
#include <initializer_list>
#include <array>

namespace altarFilters
{    

    struct BiquadState
    {
        // vibe coded lol
        float x1 = 0.0f, x2 = 0.0f;
        float y1 = 0.0f, y2 = 0.0f;
        float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f;
        float a1 = 0.0f, a2 = 0.0f;
        
        void reset() { x1 = x2 = y1 = y2 = 0.0f; }
        
        float process(float input)
        {
            float output = b0 * input + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
            
            x2 = x1;
            x1 = input;
            y2 = y1;
            y1 = output;
            
            return output;
        }
    };    

    void setBiquadCoeffs(BiquadState& state, float b0, float b1, float b2, float a0, float a1, float a2)
    {
        state.b0 = b0 / a0;
        state.b1 = b1 / a0;
        state.b2 = b2 / a0;
        state.a1 = a1 / a0;
        state.a2 = a2 / a0;
    }
    
    void setBiquadCoeffsStereo(BiquadState& L, BiquadState& R, float b0, float b1, float b2, float a0, float a1, float a2)
    {
        // makes filter stereo
        setBiquadCoeffs(L, b0, b1, b2, a0, a1, a2);
        setBiquadCoeffs(R, b0, b1, b2, a0, a1, a2);
    }

    void makePeakFilterStereo(BiquadState& L, BiquadState& R, float freq, float Q, float gainDB, float sr)
    {
        float omega = 2.0f * juce::MathConstants<float>::pi * freq / sr;
        float cosOmega = std::cos(omega);
        float sinOmega = std::sin(omega);
        float A = std::pow(10.0f, gainDB / 40.0f);
        float alpha = sinOmega / (2.0f * Q);
        
        float b0 = 1.0f + alpha * A;
        float b1 = -2.0f * cosOmega;
        float b2 = 1.0f - alpha * A;
        float a0 = 1.0f + alpha / A;
        float a1 = -2.0f * cosOmega;
        float a2 = 1.0f - alpha / A;
        
        setBiquadCoeffsStereo(L, R, b0, b1, b2, a0, a1, a2);
    }

    void makeLowShelfStereo(BiquadState& L, BiquadState& R, float freq, float Q, float gainDB, float sr)
    {
        float omega = 2.0f * juce::MathConstants<float>::pi * freq / sr;
        float cosOmega = std::cos(omega);
        float sinOmega = std::sin(omega);
        float A = std::pow(10.0f, gainDB / 40.0f);
        float beta = std::sqrt(A) / Q;
        
        float b0 = A * ((A + 1.0f) - (A - 1.0f) * cosOmega + beta * sinOmega);
        float b1 = 2.0f * A * ((A - 1.0f) - (A + 1.0f) * cosOmega);
        float b2 = A * ((A + 1.0f) - (A - 1.0f) * cosOmega - beta * sinOmega);
        float a0 = (A + 1.0f) + (A - 1.0f) * cosOmega + beta * sinOmega;
        float a1 = -2.0f * ((A - 1.0f) + (A + 1.0f) * cosOmega);
        float a2 = (A + 1.0f) + (A - 1.0f) * cosOmega - beta * sinOmega;
        
        setBiquadCoeffsStereo(L, R, b0, b1, b2, a0, a1, a2);
    }

    void makeHighShelfStereo(BiquadState& L, BiquadState& R, float freq, float Q, float gainDB, float sr)
    {
        float omega = 2.0f * juce::MathConstants<float>::pi * freq / sr;
        float cosOmega = std::cos(omega);
        float sinOmega = std::sin(omega);
        float A = std::pow(10.0f, gainDB / 40.0f);
        float beta = std::sqrt(A) / Q;
        
        float b0 = A * ((A + 1.0f) + (A - 1.0f) * cosOmega + beta * sinOmega);
        float b1 = -2.0f * A * ((A - 1.0f) + (A + 1.0f) * cosOmega);
        float b2 = A * ((A + 1.0f) + (A - 1.0f) * cosOmega - beta * sinOmega);
        float a0 = (A + 1.0f) - (A - 1.0f) * cosOmega + beta * sinOmega;
        float a1 = 2.0f * ((A - 1.0f) - (A + 1.0f) * cosOmega);
        float a2 = (A + 1.0f) - (A - 1.0f) * cosOmega - beta * sinOmega;
        
        setBiquadCoeffsStereo(L, R, b0, b1, b2, a0, a1, a2);
    }

    void makeRBJLowShelfStereo(BiquadState& L, BiquadState& R,
                        float freq, float shelfSlope, float gainDB, float sr)
    {
        float A      = std::pow(10.0f, gainDB / 40.0f);
        float omega  = 2.0f * juce::MathConstants<float>::pi * freq / sr;
        float sinO   = std::sin(omega);
        float cosO   = std::cos(omega);

        // shelfSlope = S (typically 1.0 is gentle; >1 steeper transition)
        float alpha  = sinO / 2.0f * std::sqrt( (A + 1.0f/A) * (1.0f / shelfSlope - 1.0f) + 2.0f );

        float beta   = 2.0f * std::sqrt(A) * alpha;

        float b0 =    A*( (A+1) - (A-1)*cosO + beta );
        float b1 =  2*A*( (A-1) - (A+1)*cosO );
        float b2 =    A*( (A+1) - (A-1)*cosO - beta );
        float a0 =       (A+1) + (A-1)*cosO + beta;
        float a1 =  -2*( (A-1) + (A+1)*cosO );
        float a2 =       (A+1) + (A-1)*cosO - beta;

        setBiquadCoeffsStereo(L, R, b0, b1, b2, a0, a1, a2);
    }

    void makeRBJHighShelfStereo(BiquadState& L, BiquadState& R,
                             float freq, float shelfSlope, float gainDB, float sr)
    {
        float A      = std::pow(10.0f, gainDB / 40.0f);
        float omega  = 2.0f * juce::MathConstants<float>::pi * freq / sr;
        float sinO   = std::sin(omega);
        float cosO   = std::cos(omega);

        float alpha = sinO / 2.0f * std::sqrt( (A + 1.0f/A) * (1.0f / shelfSlope - 1.0f) + 2.0f );
        float beta  = 2.0f * std::sqrt(A) * alpha;

        float b0 =    A*( (A+1) + (A-1)*cosO + beta );
        float b1 = -2*A*( (A-1) + (A+1)*cosO );
        float b2 =    A*( (A+1) + (A-1)*cosO - beta );
        float a0 =       (A+1) - (A-1)*cosO + beta;
        float a1 =  2*( (A-1) - (A+1)*cosO );
        float a2 =       (A+1) - (A-1)*cosO - beta;

        setBiquadCoeffsStereo(L, R, b0, b1, b2, a0, a1, a2);
    }

    void makeHighPassStereo(BiquadState& L, BiquadState& R, float freq, float sr)
    {
        float omega = 2.0f * juce::MathConstants<float>::pi * freq / sr;
        float cosOmega = std::cos(omega);
        float sinOmega = std::sin(omega);
        float alpha = sinOmega / (2.0f * 0.707f);
        
        float b0 = (1.0f + cosOmega) / 2.0f;
        float b1 = -(1.0f + cosOmega);
        float b2 = (1.0f + cosOmega) / 2.0f;
        float a0 = 1.0f + alpha;
        float a1 = -2.0f * cosOmega;
        float a2 = 1.0f - alpha;
        
        setBiquadCoeffsStereo(L, R, b0, b1, b2, a0, a1, a2);
    }

    void makeLowPassStereo(BiquadState& L, BiquadState& R, float freq, float sr)
    {
        float omega = 2.0f * juce::MathConstants<float>::pi * freq / sr;
        float cosOmega = std::cos(omega);
        float sinOmega = std::sin(omega);
        float alpha = sinOmega / (2.0f * 0.707f);
        
        float b0 = (1.0f - cosOmega) / 2.0f;
        float b1 = 1.0f - cosOmega;
        float b2 = (1.0f - cosOmega) / 2.0f;
        float a0 = 1.0f + alpha;
        float a1 = -2.0f * cosOmega;
        float a2 = 1.0f - alpha;
        
        setBiquadCoeffsStereo(L, R, b0, b1, b2, a0, a1, a2);
    }

    struct ToneStack
    {
        static constexpr int maxFilters = 30;

        enum class FilterType
        {
            HighPass,
            LowPass,
            Peak,
            LowShelf,
            HighShelf,
            RBJLowShelf,
            RBJHighShelf
        };

        struct FilterSpec
        {
            FilterType type;
            float freq  = 0.0f;
            float q = 0.707f;
            float gain  = 0.0f;
        };

        BiquadState pre[2][maxFilters];
        BiquadState post[2][maxFilters];

        std::array<FilterSpec, maxFilters> preSpecs;
        std::array<FilterSpec, maxFilters> postSpecs;        

        int preSize = 0;
        int postSize = 0;
        
        ToneStack() = default;
        
        ToneStack(std::initializer_list<FilterSpec> preFilters, std::initializer_list<FilterSpec> postFilters = {})
        {
            preSize = std::min(static_cast<int>(preFilters.size()), maxFilters);
            postSize = std::min(static_cast<int>(postFilters.size()), maxFilters);
            
            std::copy(preFilters.begin(), preFilters.begin() + preSize, preSpecs.begin());
            std::copy(postFilters.begin(), postFilters.begin() + postSize, postSpecs.begin());
        }

        void reset()
        {
            for (int ch = 0; ch < 2; ++ch)
            {
                for (int i = 0; i < preSize; ++i) pre[ch][i].reset();
                for (int i = 0; i < postSize; ++i) post[ch][i].reset();
            }
        }
        
        void setPreGain(int index, float gainDB)
        {
            if (index >= 0 && index < preSize)
                preSpecs[static_cast<size_t>(index)].gain = gainDB;
        }

        void setPreFreq(int index, float freq)
        {
            if (index >= 0 && index < preSize)
                preSpecs[static_cast<size_t>(index)].freq = freq;
        }

        void setPreQ(int index, float q)
        {
            if (index >= 0 && index < preSize)
                preSpecs[static_cast<size_t>(index)].q = q;
        }

        void setPostGain(int index, float gainDB)
        {
            if (index >= 0 && index < postSize)
                postSpecs[static_cast<size_t>(index)].gain = gainDB;
        }

        void setPostFreq(int index, float freq)
        {
            if (index >= 0 && index < postSize)
                postSpecs[static_cast<size_t>(index)].freq = freq;
        }

        void setPostQ(int index, float q)
        {
            if (index >= 0 && index < postSize)
                postSpecs[static_cast<size_t>(index)].q = q;
        }

        void update(float sr)
        {                        
            auto apply = [sr](const FilterSpec& fs, BiquadState& left, BiquadState& right)
            {
                switch (fs.type)
                {
                    case FilterType::HighPass:    makeHighPassStereo(left, right, fs.freq, sr); break;
                    case FilterType::LowPass:     makeLowPassStereo(left, right, fs.freq, sr); break;
                    case FilterType::Peak:        makePeakFilterStereo(left, right, fs.freq, fs.q, fs.gain, sr); break;
                    case FilterType::LowShelf:    makeLowShelfStereo(left, right, fs.freq, fs.q, fs.gain, sr); break;
                    case FilterType::HighShelf:   makeHighShelfStereo(left, right, fs.freq, fs.q, fs.gain, sr); break;
                    case FilterType::RBJLowShelf: makeRBJLowShelfStereo(left, right, fs.freq, fs.q, fs.gain, sr); break;
                    case FilterType::RBJHighShelf: makeRBJHighShelfStereo(left, right, fs.freq, fs.q, fs.gain, sr); break;
                }
            };

            for (int i = 0; i < preSize;  ++i) apply(preSpecs[i],  pre[0][i],  pre[1][i]);
            for (int i = 0; i < postSize; ++i) apply(postSpecs[i], post[0][i], post[1][i]);
        }

        inline float processPre (int ch, float x) 
        {
            for (int i = 0; i < preSize; ++i) x = pre[ch][i].process(x);
            return x;
        }

        inline float processPost(int ch, float x) 
        {
            for (int i = 0; i < postSize; ++i) x = post[ch][i].process(x);
            return x;
        }

        // accomodate every possible filter arg
        static FilterSpec makeFilter(FilterType type, float freq) { return {type, freq, 0.707f, 0.0f}; }
        static FilterSpec makeFilter(FilterType type, float freq, float q) { return {type, freq, q, 0.0f}; }
        static FilterSpec makeFilter(FilterType type, float freq, float q, float gain) { return {type, freq, q, gain}; }
    };  

    #define HP(freq) ToneStack::makeFilter(ToneStack::FilterType::HighPass, freq)
    #define LP(freq) ToneStack::makeFilter(ToneStack::FilterType::LowPass, freq)
    #define PEAK(freq, q, gain) ToneStack::makeFilter(ToneStack::FilterType::Peak, freq, q, gain)
    #define LS(freq, q, gain) ToneStack::makeFilter(ToneStack::FilterType::LowShelf, freq, q, gain)
    #define HS(freq, q, gain) ToneStack::makeFilter(ToneStack::FilterType::HighShelf, freq, q, gain)
    #define RBJ_LS(freq, q, gain) ToneStack::makeFilter(ToneStack::FilterType::RBJLowShelf, freq, q, gain)
    #define RBJ_HS(freq, q, gain) ToneStack::makeFilter(ToneStack::FilterType::RBJHighShelf, freq, q, gain) 

    

}