## Included Dependencies

All vendored dependencies are either GPL-3, GPL-2 + Later, BSD-3 or MIT and as such should be fine to include here. If they aren't, please contact me via:

https://iamlamprey.com/contact

## Modifications

Various #include statements have been modified to use absolute relative paths, because HISE auto-generates the .projucer files for ThirdParty nodes at compile time. If you somehow misplace them, they are:

JSON: https://github.com/nlohmann/json/  
math_approx: https://github.com/Chowdhury-DSP/math_approx  
RTNeural: https://github.com/jatinchowdhury18/RTNeural  
RTNeural-NAM: https://github.com/jatinchowdhury18/RTNeural-NAM  
Rubberband (0.3b & below): https://github.com/breakfastquay/rubberband
signalsmith-stretch (0.4b & above): https://github.com/Signalsmith-Audio/signalsmith-stretch
xsimd: https://github.com/xtensor-stack/xsimd 