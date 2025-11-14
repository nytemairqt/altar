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

include("Boilerplate/overdrive.js");
include("Boilerplate/amp.js");
include("Boilerplate/cab.js");
include("Boilerplate/reverb.js");
include("Boilerplate/delay.js");
include("Boilerplate/chorus.js");
include("Boilerplate/ringmod.js");

namespace ModularChain
{   

    // slot containers
    const fxSlots = [Synth.getSlotFX("modularA"), Synth.getSlotFX("modularB"), Synth.getSlotFX("modularC"), Synth.getSlotFX("modularD"), Synth.getSlotFX("modularE"), Synth.getSlotFX("modularF"), Synth.getSlotFX("modularG")];
    
    // slot containers as effects
    const fxModules = [Synth.getEffect("modularA"), Synth.getEffect("modularB"), Synth.getEffect("modularC"), Synth.getEffect("modularD"), Synth.getEffect("modularE"), Synth.getEffect("modularF"), Synth.getEffect("modularG")];

    // draggable panels
    const pnlFxSlots = [ Content.getComponent("pnlModularA"), Content.getComponent("pnlModularB"), Content.getComponent("pnlModularC"), Content.getComponent("pnlModularD"), Content.getComponent("pnlModularE"), Content.getComponent("pnlModularF"), Content.getComponent("pnlModularG")];

    // module-specific panels 
    const pnlFx = [Content.getComponent("pnlOverdrive"), Content.getComponent("pnlAmp"), Content.getComponent("pnlCab"), Content.getComponent("pnlReverb"), Content.getComponent("pnlDelay"), Content.getComponent("pnlChorus"), Content.getComponent("pnlRingmod")];    
    
    // invisible bypass buttons
    const bypassButtons = [Content.getComponent("btnModularABypass"), Content.getComponent("btnModularBBypass"), Content.getComponent("btnModularCBypass"), Content.getComponent("btnModularDBypass"), Content.getComponent("btnModularEBypass"), Content.getComponent("btnModularFBypass"), Content.getComponent("btnModularGBypass")];            

    for (m in fxModules) { Engine.addModuleStateToUserPreset(m.getId()); }
            
    const btnShowCabDesigner = Content.getComponent("btnShowCabDesigner");   
    inline function hideCabDesigner() { btnShowCabDesigner.setValue(0); btnShowCabDesigner.changed();}

    // panel dragging logic        
    reg dragging = false;
    reg target; 

    inline function paintRoutine(g)
    {               
        local area = [0, 0, this.getWidth(), this.getHeight()];

        local slotIndex = pnlFxSlots.indexOf(this);
        local btn = slotIndex != -1 ? bypassButtons[slotIndex] : 0;

        g.setColour(btn && btn.getValue() ? Colours.white : Colours.grey);                
        switch (this.get("text"))
        {
            case "overdrive": 
                g.drawAlignedText("Overdrive", area, "centred");
                if (pnlFx[0].get("visible")) { g.setColour(ColourData.clrWhite); g.drawLine(20, area[2] - 20, area[3] - 5, area[3] - 5, 1.0); }
                break;
            case "amp": 
                g.drawAlignedText("Amp", area, "centred"); 
                if (pnlFx[1].get("visible")) { g.setColour(ColourData.clrWhite); g.drawLine(20, area[2] - 20, area[3] - 5, area[3] - 5, 1.0); }
                break;
            case "cab": 
                g.drawAlignedText("Cab", area, "centred"); 
                if (pnlFx[2].get("visible")) { g.setColour(ColourData.clrWhite); g.drawLine(20, area[2] - 20, area[3] - 5, area[3] - 5, 1.0); }
                break;
            case "reverb": 
                g.drawAlignedText("Reverb", area, "centred"); 
                if (pnlFx[3].get("visible")) { g.setColour(ColourData.clrWhite); g.drawLine(20, area[2] - 20, area[3] - 5, area[3] - 5, 1.0); }
                break;
            case "delay": 
                g.drawAlignedText("Delay", area, "centred"); 
                if (pnlFx[4].get("visible")) { g.setColour(ColourData.clrWhite); g.drawLine(20, area[2] - 20, area[3] - 5, area[3] - 5, 1.0); }
                break;
            case "chorus": 
                g.drawAlignedText("Chorus", area, "centred"); 
                if (pnlFx[5].get("visible")) { g.setColour(ColourData.clrWhite); g.drawLine(20, area[2] - 20, area[3] - 5, area[3] - 5, 1.0); }
                break;
            case "ringmod": 
                g.drawAlignedText("Ringmod", area, "centred"); 
                if (pnlFx[6].get("visible")) { g.setColour(ColourData.clrWhite); g.drawLine(20, area[2] - 20, area[3] - 5, area[3] - 5, 1.0); }
                break;          
        }                
            
        if (dragging) // green highlight
        {
            if (this == target) { g.setColour(Colours.withAlpha(Colours.green, 0.3)); }             
            else { g.setColour(Colours.withAlpha(Colours.green, 0.1)); }                
            g.fillRoundedRectangle(area, 4.0);
        }  
        else if (this.data.hover && !dragging)
        {
            g.setColour(Colours.withAlpha(Colours.white, 0.05));
            g.fillRoundedRectangle(area, 4.0);
        }
    }

    for (p in pnlFxSlots)
        p.setPaintRoutine(paintRoutine);

    inline function repaintAllSlots()
    {
        for (i = 0; i < pnlFxSlots.length; i++) { pnlFxSlots[i].set("text", fxSlots[i].getCurrentEffectId()); }                             
        for (p in pnlFxSlots) { p.repaint(); }
    }

    function dragPaintRoutine(g, obj)
    {               
        g.setColour(Colours.withAlpha(Colours.white, 0.60));
        g.fillRoundedRectangle(obj.area, 2.0, 2.0);
    }

    inline function checkValid(targetId)
    {
        // only allow drop on other modular UI elements
        local names = ["pnlModularA", "pnlModularB", "pnlModularC", "pnlModularD", "pnlModularE", "pnlModularF", "pnlModularG"];
        Content.refreshDragImage();     
        if (names.contains(targetId))
        {
            target = Content.getComponent(targetId);
            repaintAllSlots();
            return true;  
        }        
        else { return false; }
    }

    inline function hideFXPanels()
    {
        for (p in pnlFx)
            p.set("visible", false);
    }

    inline function snapshotStates()
    {
        local states = [];
        for (i = 0; i < fxModules.length; i++)
            states.push(fxModules[i].exportState());
        return states;
    }

    inline function applyStatesByMapping(states, indexMapping)
    {
        for (newIndex = 0; newIndex < fxModules.length; newIndex++)
        {
            local oldIndex = indexMapping[newIndex];
            fxModules[newIndex].restoreState(states[oldIndex]);
        }
    }

    inline function snapshotBypassValues()
    {
        local vals = [];
        for (i = 0; i < bypassButtons.length; i++)
            vals.push(bypassButtons[i].getValue()); 
        return vals;
    }

    inline function applyBypassValuesByMapping(vals, indexMapping)
    {
        for (newIndex = 0; newIndex < bypassButtons.length; newIndex++)
        {
            local oldIndex = indexMapping[newIndex];
            local v = vals[oldIndex];
            bypassButtons[newIndex].setValue(v);
            bypassButtons[newIndex].changed();
        }
    }

    inline function computeInsertMapping(fromIndex, toIndex, length)
    {
        local indices = [];
        for (i = 0; i < length; i++) { indices.push(i); }

        indices.remove(fromIndex);

        local insertIndex = toIndex;

        if (insertIndex < 0) insertIndex = 0;
        if (insertIndex > indices.length) insertIndex = indices.length;

        indices.insert(insertIndex, fromIndex);

        return indices;
    }

    function insertDragCallback(isValid, targetName)
    {
        dragging = false;
        target = 0;

        if (!isValid || targetName == "")
        {
            repaintAllSlots();
            return;
        }

        var dropTarget = Content.getComponent(targetName);
        if (!pnlFxSlots.contains(dropTarget))
        {
            repaintAllSlots();
            return;
        }
        if (dropTarget == this)
        {
            repaintAllSlots();
            return;
        }

        var fromIndex = pnlFxSlots.indexOf(this);
        var toIndex = pnlFxSlots.indexOf(dropTarget);
        if (fromIndex == -1 || toIndex == -1 || fromIndex == toIndex)
        {
            repaintAllSlots();
            return;
        }

        var states = snapshotStates();
        var bypassVals = snapshotBypassValues();

        var mapping = computeInsertMapping(fromIndex, toIndex, fxModules.length);

        applyStatesByMapping(states, mapping);

        applyBypassValuesByMapping(bypassVals, mapping);
        
        repaintAllSlots();

        target = 0;

        Amp.sendNAMCableData();                
    }

    inline function mouseCallback(event)
    {
        dragging = false;       
        this.setMouseCursor("NormalCursor", Colours.white, [0, 0]);

        if (event.drag && !event.rightClick)
        {
            dragging = true;
            this.setMouseCursor("DraggingHandCursor", Colours.white, [0, 0]);  
            hideCabDesigner();          

            if (event.dragX > 10 || event.dragY > 10 || event.dragX < -10 || event.dragY < -10)
            {               
                this.startInternalDrag({
                    area: [0, 0, 25, 25],
                    paintRoutine: dragPaintRoutine,
                    dragCallback: insertDragCallback,
                    isValid: checkValid,
                });
            }
        }
        else if (event.clicked && !event.rightClick)
        {
            hideFXPanels();
            hideCabDesigner();
            switch (this.get("text"))
            {
                case "overdrive": pnlFx[0].set("visible", true); break;
                case "amp":       pnlFx[1].set("visible", true); break;
                case "cab":       pnlFx[2].set("visible", true); break;
                case "reverb":    pnlFx[3].set("visible", true); break;
                case "delay":     pnlFx[4].set("visible", true); break;
                case "chorus":    pnlFx[5].set("visible", true); break;
                case "ringmod":   pnlFx[6].set("visible", true); break;
            }            
            repaintAllSlots();
        }
        else if (event.clicked && event.rightClick)
        {
            local slotIndex = pnlFxSlots.indexOf(this);
            if (slotIndex != -1)
            {
                local btn = bypassButtons[slotIndex];
                btn.setValue(1 - btn.getValue());
                btn.changed();
            }            
        }

        this.data.hover = event.hover;    
        this.repaint();
    }

    for (p in pnlFxSlots)
        p.setMouseCallback(mouseCallback);
    
    inline function onbtnModularBypassControl(component, value)
    {
        local idx = bypassButtons.indexOf(component);
        if (idx != -1)
        {
            local effect = fxSlots[idx].getCurrentEffect();
            effect.setBypassed(1 - value);
        }
    }
            
    for (b in bypassButtons)
        b.setControlCallback(onbtnModularBypassControl);

}