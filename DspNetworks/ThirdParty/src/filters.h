// FIX ME: add GPL license to all .h files

#pragma once
#include <JuceHeader.h>

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

}