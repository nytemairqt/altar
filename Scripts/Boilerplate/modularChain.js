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

namespace ModularChain
{
    /* GLOBAL CABLE */

    const grm = Engine.getGlobalRoutingManager();
    const namCable = grm.getCable("nam");
        
    /*
    namCable.registerCallback(function(data)
    {
        if (data < 0.0)
            return;
        
        var logMin = Math.log(40.0);
        var logMax = Math.log(400.0);
        var logFreq = logMin + data * (logMax - logMin);
        var pitch = Math.exp(logFreq);  
        
        //Console.print(Math.round(pitch) + "hz");
        var analyzed = analyzePitch(pitch);
    }, false);
    */
    
    const filePath = "C:/Users/iamla/Desktop/14a Marshall JCM800 2203X RATT Rhythm.nam";
    const namModel = FileSystem.fromAbsolutePath(filePath);
    const jsonData = namModel.loadAsObject();
    
    const btnTest = Content.getComponent("btnTest");
    
    
    inline function onbtnTestControl(component, value)
    {
	    if (value)
	    	namCable.sendData(jsonData);
    }
    
    btnTest.setControlCallback(onbtnTestControl);
    
    

	const fxSlots = [Synth.getSlotFX("modularA"),
                     Synth.getSlotFX("modularB"),
                     Synth.getSlotFX("modularC"),
                     Synth.getSlotFX("modularD"),
                     Synth.getSlotFX("modularE"),
                     Synth.getSlotFX("modularF"),
                     Synth.getSlotFX("modularG")];

    const fxModules =  [Synth.getEffect("modularA"),
                        Synth.getEffect("modularB"),
                        Synth.getEffect("modularC"),
                        Synth.getEffect("modularD"),
                        Synth.getEffect("modularE"),
                        Synth.getEffect("modularF"),
                        Synth.getEffect("modularG")];                           
                        
    // DSP Module Panels
    const pnlFxSlots = [Content.getComponent("pnlModularA"),
                        Content.getComponent("pnlModularB"),
                        Content.getComponent("pnlModularC"),
                        Content.getComponent("pnlModularD"),
                        Content.getComponent("pnlModularE"),
                        Content.getComponent("pnlModularF"),
                        Content.getComponent("pnlModularG")];       
                            
    const pnlFx = [Content.getComponent("pnlOverdrive"),
                   Content.getComponent("pnlAmp"),
                   Content.getComponent("pnlCab"),
                   Content.getComponent("pnlReverb"),
                   Content.getComponent("pnlDelay"),
                   Content.getComponent("pnlChorus"),
                   Content.getComponent("pnlRingmod")];

    reg effectToSlot = {};
    const OVERDRIVE_PARAMS = { Gain: 0, Tone: 1, Mix: 2 };
    const AMP_PARAMS = { Mode: 0, Input: 1, Low: 2, Mid: 3, High: 4, Presence: 5, Output: 6};
    
    for (f in fxModules)
    	Engine.addModuleStateToUserPreset(f.getId());

    inline function rebuildEffectMap()
    {
        effectToSlot = {};
        for (i = 0; i < fxSlots.length; i++) { effectToSlot[fxSlots[i].getCurrentEffectId()] = i; }
    }
 
    inline function paintRoutine(g)
    {
        for (i=0; i<pnlFxSlots.length; i++) { pnlFxSlots[i].set("text", fxSlots[i].getCurrentEffectId()); }        
        local area = [0, 0, this.getWidth(), this.getHeight()];        
        g.setColour(Colours.white);
        g.drawRoundedRectangle(area, 1.0, 1.0);               
        g.drawAlignedText(this.get("text"), area, "centred");
                        
        if (this.data.isTarget)
            g.drawLine(5, area[2]-5, area[3]-5, area[3]-5, 1.0);
    }       

    for (p in pnlFxSlots)
        p.setPaintRoutine(paintRoutine);

    inline function repaint()
    {
        for (p in pnlFxSlots)
        {
            p.data.isTarget = false;
            p.repaint();
        }
    }

    function dragPaintRoutine(g, obj)
    {
        var isValid = false;
        isValid = obj.valid;
        isValid |= obj.target == "";
        isValid |= obj.target.length == 0;
        isValid |= obj.target == obj.source;

        // probably ways to improve this
        if (isValid && obj.target != "" && obj.target != obj.source)
        {
            repaint();

            var component = Content.getComponent(obj.target);
            component.data.isTarget = true;
            component.repaint();
        }
            
        g.setColour(Colours.withAlpha(Colours.white, 0.2));
        g.fillRoundedRectangle(obj.area, 2.0);
        g.setColour(Colours.withAlpha(Colours.white, 0.6));
        g.fillRoundedRectangle(obj.area, 2.0, 2.0); 
    }    

    function drag(isValid, targetName)
    {
        if (targetName != "")
            var target = Content.getComponent(targetName);
        
        //repaintPnlFxSlots();
            
        if (!pnlFxSlots.contains(target))
            return;
        
        if (target == this)
            return;
        
        var currentIndex = pnlFxSlots.indexOf(this);
        var targetIndex = pnlFxSlots.indexOf(target);
        var currentSlot = fxSlots[currentIndex];
        var targetSlot = fxSlots[targetIndex]; 
        
        // swap FX & restore state
        var currentState = fxModules[currentIndex].exportState();
        var targetState = fxModules[targetIndex].exportState();         
        
        // CRASH RELATED TO SWAPPING CAB POSITION
        // MODULE POSITIONS AREN'T SAVING 
        //currentSlot.setEffect(targetName);
        //targetSlot.setEffect(currentName);    

        // This crashes due to a &nullptr 
        // christoph recommends using predefined states, not dynamic ones
        fxModules[currentIndex].restoreState(targetState);
        fxModules[targetIndex].restoreState(currentState);
        
        // get the new effects (after swap)
        var targetEffect = targetSlot.getCurrentEffect(); 
        var targetEffectName = targetSlot.getCurrentEffectId(); 
        var currentEffect = currentSlot.getCurrentEffect(); 
        var currentEffectName = currentSlot.getCurrentEffectId(); 
            
        // repaint
        //pnlFxSlotReloadText();      
        repaint();
    }    

    inline function checkValidPnlFxSlot(targetId)
    {
        local names = ["pnlModularA", "pnlModularB", "pnlModularC", "pnlModularD", "pnlModularE", "pnlModularF", "pnlModularG"];
        Content.refreshDragImage();         
        return names.contains(targetId);
    }    

    inline function hideFXPanels()
    {
        for (p in pnlFx)
            p.set("visible", false);
    }   

    inline function dragMouseCallback(event)
    {
        if (event.drag && !event.rightClick)
        {
            if (event.dragX > 10 || event.dragY > 10 || event.dragX < -10 || event.dragY < -10)
                this.startInternalDrag({
                    area: [0, 0, 25, 25],
                    paintRoutine: dragPaintRoutine,
                    dragCallback: drag,
                    isValid: checkValidPnlFxSlot
                });
        }
        else if (event.clicked && !event.rightClick)
        {
            // hide other panels
            hideFXPanels();
                
            switch (this.get("text"))
            {
                case "overdrive": pnlFx[0].set("visible", true); break;
                case "amp": pnlFx[1].set("visible", true); break;
                case "cab": pnlFx[2].set("visible", true); break;
                case "reverb": pnlFx[3].set("visible", true); break;
                case "delay": pnlFx[4].set("visible", true); break;
                case "chorus": pnlFx[5].set("visible", true); break;
                case "ringmod": pnlFx[6].set("visible", true); break;                           
            }
        }
        else if (event.clicked && event.rightClick)
        {
            // some sort of for loop        
        }
    }

    for (p in pnlFxSlots)
        p.setMouseCallback(dragMouseCallback);

    // NEW STUFF
    
   inline function bindPanelToEffect(effectId, knobParamPairs)
    {
        if (!effectToSlot.contains(effectId)) return;

        local slotIndex = effectToSlot[effectId];
        local procId = fxSlots[slotIndex].getCurrentEffectId(); // current processor inside the slot

        // knobParamPairs: [{ knob: Content.getComponent("knbODGain"), param: OVERDRIVE_PARAMS.Gain }, ...]
        for (p in knobParamPairs)
        {
            p.knob.set("processorId", procId);
            p.knob.set("parameterId", p.param);
            p.knob.setControlCallback(undefined);
        }
    }

    inline function bindOverdrivePanel()
    {
        bindPanelToEffect("overdrive", [
            { knob: Content.getComponent("knbODGain"),  param: OVERDRIVE_PARAMS.Gain },
            { knob: Content.getComponent("knbODTone"),  param: OVERDRIVE_PARAMS.Tone },
            { knob: Content.getComponent("knbODMix"),   param: OVERDRIVE_PARAMS.Mix }
        ]);
    }

    inline function rebindParameters()
    {
        bindPanelToEffect("amp", [
            { knob: Content.getComponent("knbAmpInput"), param: AMP_PARAMS.Input},
        ]);
    }
    
    rebuildEffectMap();
}