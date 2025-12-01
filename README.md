## Altar

Altar is a free, open-source guitar amplifier. 

Built with HISE: https://hise.dev/  

## Modifications to HISE Source Code

Altar depends on several small fixes to the HISE source. 

Read [HISE](./HISE/HISE.md) for more information. 

## Building ThirdParty Nodes

Altar uses multiple HISE ThirdParty nodes with their own dependencies. They are included in the /DspNetworks/ThirdParty/src/dependencies folder and are (should be) compatible with GPL-3. 

Read [DEPENDENCIES](./DspNetworks/ThirdParty/src/dependencies/DEPENDENCIES.md) for more information.

You must also compile HISE with the following preprocessor:

```
HI_ENABLE_CUSTOM_NODES=1
```