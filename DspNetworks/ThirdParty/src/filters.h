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
        static constexpr int maxFilters = 20;

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

    // DC Blocker

    ToneStack dcBlocker{
        {
            HP(140.0f),
            HP(140.0f),
            HP(140.0f),
            HP(140.0f),            
        }, {}
    };

    // CLEAN

    ToneStack glass{
        // pre
        {
            HP(55.0f),
            PEAK(870.0f, 0.4f, 4.0f),
            PEAK(3700.0f, 0.32f, 7.0f),
            HS(6000.0f, 0.7f, 16.0f)
        },
        // post
        {
            HP(60.0f),
            PEAK(140.0f, 2.4f, -3.5f),
            PEAK(2200.0f, 0.3f, 4.0f),
            PEAK(2600.0f, 5.8f, -5.1f),
            PEAK(6400.0f, 1.0f, 4.8f),
            LP(10000.0f)
        }
    };

    ToneStack pearl{
        {
            HP(55.0f),
            PEAK(870.0f, 0.4f, 4.0f),
            PEAK(3700.0f, 0.32f, 7.0f),
            HS(6000.0f, 0.7f, 16.0f)
        },
        {
            HP(60.0f),
            PEAK(140.0f, 2.4f, -3.5f),
            PEAK(2200.0f, 0.3f, 4.0f),
            PEAK(2600.0f, 5.8f, -5.1f),
            PEAK(6400.0f, 1.0f, 4.8f),
            LP(10000.0f)
        }
    };

    ToneStack diamond{
        {
            HP(55.0f),
            PEAK(870.0f, 0.4f, 4.0f),
            PEAK(3700.0f, 0.32f, 7.0f),
            HS(6000.0f, 0.7f, 16.0f)
        },
        {
            HP(60.0f),
            PEAK(140.0f, 2.4f, -3.5f),
            PEAK(2200.0f, 0.3f, 4.0f),
            PEAK(2600.0f, 5.8f, -5.1f),
            PEAK(6400.0f, 1.0f, 4.8f),
            LP(10000.0f)
        }
    };

    ToneStack zircon{
        {
            HP(55.0f),
            PEAK(870.0f, 0.4f, 4.0f),
            PEAK(3700.0f, 0.32f, 7.0f),
            HS(6000.0f, 0.7f, 16.0f)
        },
        {
            HP(60.0f),
            PEAK(140.0f, 2.4f, -3.5f),
            PEAK(2200.0f, 0.3f, 4.0f),
            PEAK(2600.0f, 5.8f, -5.1f),
            PEAK(6400.0f, 1.0f, 4.8f),
            LP(10000.0f)
        }
    };

    // DIRTY

    ToneStack slate{
       {
            HP(82.0f),
            PEAK(650.0f, 1.2f, 2.5f),
            PEAK(2200.0f, 0.8f, 3.2f),
            HS(4500.0f, 0.7f, 6.0f)
       },
       {
            HP(40.0f),
            LS(45.0f, 0.5f, 6.0f),
            PEAK(75.1f, 0.46f, 4.38f),
            PEAK(120.0f, 0.5f, 8.0f),
            LS(150.0f, 0.7f, -2.0f),
            PEAK(160.0f, 0.6f, 2.0f),
            PEAK(750.0f, 1.4f, 4.0f),
            PEAK(1800.0f, 1.0f, 2.8f),
            PEAK(2500.0f, 2.11f, 1.86f),
            PEAK(3200.0f, 1.2f, -1.5f),
            PEAK(3638.0f, 1.156f, 4.616f),
            PEAK(3670, 1.14f, 3.48f),
            PEAK(5000.0f, 0.8f, 2.0f),
            HS(6500.0f, 0.7f, -1.8f),
            PEAK(7842.0f, 0.938f, 4.032f),
            HS(9000.0f, 0.6f, 3.5f),
            LP(12000.0f),
       }
    };

    ToneStack obsidian{
        {
            HP(60.0f),
            PEAK(520.0f, 0.9f, 2.8f),
            PEAK(1400.0f, 0.7f, 3.2f),
            PEAK(3800.0f, 1.1f, 4.5f),
        },
        {
            HP(32.0f),
            HP(32.0f),
            RBJ_LS(120.0f, 0.5f, 11.712f),
            PEAK(129.0f, 1.74f, 5.86f),
            PEAK(180.0f, 0.813f, 3.0f),
            PEAK(528.0f, 0.71f, -9.0f),
            PEAK(600.0f, 0.8f, 3.84f),
            PEAK(1100.0f, 1.374f, -5.872f),
            PEAK(1787.0f, 0.813, -2.496f),
            PEAK(2185.0f, 0.437f, 7.686f),
            PEAK(2900.0f, 2.03f, 3.60f),
            HS(6000.0f, 0.8f, 5.0f),
            HS(7760.0f, 0.813f, 2.5f),
            PEAK(8678.0f, 0.688f, 3.532f),
            LP(17122.0f)
        }
    };

    ToneStack amethyst{
        {
            HP(60.0f),
            PEAK(500.0f, 1.1f, 2.8f),
            PEAK(1600.0f, 0.9f, 3.5f),
            PEAK(4000.0f, 1.3f, 4.0f),
        },
        {
            HP(45.0f),
            LS(45.0f, 0.5f, 6.0f),
            PEAK(120.0f, 0.5f, 8.0f),
            PEAK(180.0f, 1.2f, -1.8f),
            PEAK(600.0f, 1.0f, 3.2f),
            PEAK(1200.0f, 0.8f, 2.5f),
            PEAK(2800.0f, 1.4f, -2.0f),
            PEAK(4500.0f, 1.1f, 4.5f),
            PEAK(6500.0f, 0.9f, 2.8f),
            HS(8500.0f, 0.7f, 3.5f),
            LP(13000.0f)
        }
    };

    ToneStack opal{
        {
            HP(90.0f),
            PEAK(720.0f, 1.5f, 3.8f),
            PEAK(2000.0f, 1.0f, 2.5f),
            PEAK(5500.0f, 1.8f, 5.2f)
        },
        {
            HP(40.0f),
            HP(45.0f),
            LS(45.0f, 0.5f, 6.0f),
            PEAK(120.0f, 0.5f, 8.0f),
            PEAK(200.0f, 1.5f, -3.2f),
            PEAK(800.0f, 1.2f, 4.5f),
            PEAK(1600.0f, 0.9f, 2.8f),
            PEAK(3200.0f, 2.0f, -2.8f),
            PEAK(5800.0f, 1.3f, 4.8f),
            HS(7500.0f, 0.8f, -2.5f),
            LP(11500.0f)
        }
    };

    ToneStack sapphire{
        {
            HP(85.0f),
            PEAK(680.0f, 1.4f, 4.2f),
            PEAK(1800.0f, 0.9f, 3.5f),
            PEAK(4800.0f, 1.6f, 5.8f),
        },
        {
            HP(45.0f),
            LS(45.0f, 0.5f, 6.0f),
            PEAK(120.0f, 0.5f, 8.0f),
            PEAK(150.0f, 1.2f, -2.5f),
            PEAK(750.0f, 1.1f, 4.8f),
            PEAK(1500.0f, 0.8f, 2.8f),
            PEAK(2800.0f, 1.8f, -2.2f),
            PEAK(5200.0f, 1.2f, 5.5f),
            PEAK(7200.0f, 0.9f, 3.2f),
            HS(9000.0f, 0.7f, -1.8f),
            LP(12800.0f)
        }   
    };

    ToneStack garnet{
        {
            HP(50.0f),
            PEAK(320.0f, 0.8f, 2.2f),
            PEAK(1000.0f, 0.7f, 2.8f),
            PEAK(2800.0f, 0.6f, 3.8f),
        },
        {
            HP(45.0f),
            LS(45.0f, 0.5f, 6.0f),
            LS(100.0f, 0.7f, 2.0f),
            PEAK(120.0f, 0.5f, 8.0f),
            PEAK(360.0f, 0.9f, 2.5f),
            PEAK(850.0f, 0.8f, 2.8f),
            PEAK(1600.0f, 0.9f, 2.2f),
            PEAK(3200.0f, 1.1f, 3.5f),
            PEAK(5500.0f, 0.8f, 3.8f),
            HS(7500.0f, 0.7f, 4.5f),
            LP(16000.0f)
        }
    };

}