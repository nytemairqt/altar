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

    // ---------------------------
    // Slots / Modules / UI Panels
    // ---------------------------

    const fxSlots = [Synth.getSlotFX("modularA"), Synth.getSlotFX("modularB"), Synth.getSlotFX("modularC"), Synth.getSlotFX("modularD"), Synth.getSlotFX("modularE"), Synth.getSlotFX("modularF"), Synth.getSlotFX("modularG")];

    // These are the slot containers (not the inner effects)
    const fxModules = [Synth.getEffect("modularA"), Synth.getEffect("modularB"), Synth.getEffect("modularC"), Synth.getEffect("modularD"), Synth.getEffect("modularE"), Synth.getEffect("modularF"), Synth.getEffect("modularG")];

    // Draggable slot panels (one per slot position)
    const pnlFxSlots = [ Content.getComponent("pnlModularA"), Content.getComponent("pnlModularB"), Content.getComponent("pnlModularC"), Content.getComponent("pnlModularD"), Content.getComponent("pnlModularE"), Content.getComponent("pnlModularF"), Content.getComponent("pnlModularG")];

    // Effect control panels (one per effect type)
    const pnlFx = [Content.getComponent("pnlOverdrive"), Content.getComponent("pnlAmp"), Content.getComponent("pnlCab"), Content.getComponent("pnlReverb"), Content.getComponent("pnlDelay"), Content.getComponent("pnlChorus"), Content.getComponent("pnlRingmod")];    
    
    // UI Controls (skip any that have individual logic in their respective namespaces)
    const bypassButtons = [Content.getComponent("btnModularABypass"), Content.getComponent("btnModularBBypass"), Content.getComponent("btnModularCBypass"), Content.getComponent("btnModularDBypass"), Content.getComponent("btnModularEBypass"), Content.getComponent("btnModularFBypass"), Content.getComponent("btnModularGBypass")];            

    // Ensure slot states are stored with user presets
    for (m in fxModules)
        Engine.addModuleStateToUserPreset(m.getId());
    
    // Make sure cab designer is hidden
    const btnShowCabDesigner = Content.getComponent("btnShowCabDesigner");   
    inline function hideCabDesigner() { btnShowCabDesigner.setValue(0); btnShowCabDesigner.changed();}
        
    // Panel shenanigans
    reg dragging = false;
    reg target; 

    // ---------------------------
    // Painting & Repaint Utilities
    // ---------------------------

    inline function paintRoutine(g)
    {               
        local area = [0, 0, this.getWidth(), this.getHeight()];

        // FIX: determine bypass state by slot index, not effect name
        local slotIndex = pnlFxSlots.indexOf(this);
        local btn = slotIndex != -1 ? bypassButtons[slotIndex] : 0;

        // Text 
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
            
        // Hover Highlight
        if (dragging)
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

    // ---------------------------
    // Drag visuals
    // ---------------------------

    function dragPaintRoutine(g, obj)
    {               
        // Little square that pops up when dragging
        g.setColour(Colours.withAlpha(Colours.white, 0.60));
        g.fillRoundedRectangle(obj.area, 2.0, 2.0);
    }

    inline function checkValid(targetId)
    {
        // Only allow drops onto these panels
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

    // ---------------------------
    // Chain Reordering (Insert)
    // ---------------------------

    inline function snapshotStates()
    {
        // Export all slot container states up-front (stable approach)
        local states = [];
        for (i = 0; i < fxModules.length; i++)
            states.push(fxModules[i].exportState());
        return states;
    }

    inline function applyStatesByMapping(states, indexMapping)
    {
        // indexMapping[newIndex] = oldIndex
        // Restore in one pass for stability
        for (newIndex = 0; newIndex < fxModules.length; newIndex++)
        {
            local oldIndex = indexMapping[newIndex];
            fxModules[newIndex].restoreState(states[oldIndex]);
        }
    }

    // NEW: snapshot/apply bypass states so they move with modules
    inline function snapshotBypassValues()
    {
        local vals = [];
        for (i = 0; i < bypassButtons.length; i++)
            vals.push(bypassButtons[i].getValue()); // 1 = active, 0 = bypassed
        return vals;
    }

    inline function applyBypassValuesByMapping(vals, indexMapping)
    {
        for (newIndex = 0; newIndex < bypassButtons.length; newIndex++)
        {
            local oldIndex = indexMapping[newIndex];
            local v = vals[oldIndex];

            // Update the button value for the new slot position
            bypassButtons[newIndex].setValue(v);

            // Propagate to the actual effect in this slot
            // onbtnModularBypassControl uses 1 - value to call setBypassed()
            bypassButtons[newIndex].changed();
        }
    }

    inline function computeInsertMapping(fromIndex, toIndex, length)
    {
        // Returns an array map: map[newIndex] = oldIndex
        // Start with identity mapping
        local indices = [];
        for (i = 0; i < length; i++) indices.push(i);

        // Remove the dragged index
        indices.remove(fromIndex);

        // Insert semantics: drop takes the target's position, pushing target (and following) right.
        // Therefore we always insert at 'toIndex' in the reduced array.
        local insertIndex = toIndex;

        // Clamp for safety
        if (insertIndex < 0) insertIndex = 0;
        if (insertIndex > indices.length) insertIndex = indices.length;

        indices.insert(insertIndex, fromIndex);

        // Now 'indices' describes which oldIndex goes to each newIndex
        // i.e. indices[newIndex] = oldIndex
        return indices;
    }

    function insertDragCallback(isValid, targetName)
    {
        dragging = false;
        // Defer repaint until after states are restored so labels reflect the new order
        // repaintAllSlots();
        
        // Called when we drag a module panel onto another module panel
        if (!isValid || targetName == "") return;

        var target = Content.getComponent(targetName);
        if (!pnlFxSlots.contains(target)) return;
        if (target == this) return;  

        // Compute indices
        var fromIndex = pnlFxSlots.indexOf(this);
        var toIndex = pnlFxSlots.indexOf(target);
        if (fromIndex == -1 || toIndex == -1 || fromIndex == toIndex) return;

        // 1) Snapshot all slot container states and current bypass UI values
        var states = snapshotStates();
        var bypassVals = snapshotBypassValues();

        // 2) Build the new mapping using "insert" semantics
        var mapping = computeInsertMapping(fromIndex, toIndex, fxModules.length);

        // 3) Apply states by mapping in one restore pass
        applyStatesByMapping(states, mapping);

        // 4) Apply bypass values with the same mapping and propagate to effects
        applyBypassValuesByMapping(bypassVals, mapping);
        
        // 5) Now that the mapping is applied, update labels and force a repaint
        repaintAllSlots();

        // Clear hover highlight target
        target = 0;

        // restore NAM model
        Amp.sendNAMCableData();                
    }

    // ---------------------------
    // Mouse interaction
    // ---------------------------

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
            // Show the UI panel for the effect under this slot
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
            // FIX: toggle bypass for the button that belongs to THIS SLOT, not by effect name
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
    
    
    // ---------------------------
    // Bind Bypass to Hidden Controls
    // ---------------------------
    inline function onbtnModularBypassControl(component, value)
    {
        // FIX: map button -> slot index -> effect in that slot
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