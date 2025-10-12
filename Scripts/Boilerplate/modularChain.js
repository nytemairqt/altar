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
    
    // UI Controls
    const bypassButtons = [Content.getComponent("btnModularABypass"), Content.getComponent("btnModularBBypass"), Content.getComponent("btnModularCBypass"), Content.getComponent("btnModularDBypass"), Content.getComponent("btnModularEBypass"), Content.getComponent("btnModularFBypass"), Content.getComponent("btnModularGBypass")];            
    const knbAmpControl = [Content.getComponent("knbAmpMode"), Content.getComponent("knbAmpInput"), Content.getComponent("knbAmpLow"), Content.getComponent("knbAmpMid"), Content.getComponent("knbAmpHigh"), Content.getComponent("knbAmpPresence"), Content.getComponent("knbAmpOutput")];
    const knbCabControl = [Content.getComponent("knbCabMix"), Content.getComponent("btnCabAEnable"), Content.getComponent("knbCabAAxis"), Content.getComponent("knbCabADistance"), Content.getComponent("knbCabADelay"), Content.getComponent("knbCabAPan"), Content.getComponent("knbCabAGain"), Content.getComponent("btnCabAPhase"), Content.getComponent("btnCabBPhase"), Content.getComponent("btnCabBEnable"), Content.getComponent("knbCabBAxis"), Content.getComponent("knbCabBDistance"), Content.getComponent("knbCabBDelay"), Content.getComponent("knbCabBPan"), Content.getComponent("knbCabBGain")];
    const pnlAmpNAMLoader = Content.getComponent("pnlAmpNAMLoader");  

    // Ensure slot states are stored with user presets
    for (m in fxModules)
        Engine.addModuleStateToUserPreset(m.getId());

    // ---------------------------
    // Painting & Repaint Utilities
    // ---------------------------

    inline function paintRoutine(g)
    {
        // Keep panel labels in sync with current effect id
        for (i = 0; i < pnlFxSlots.length; i++)
            pnlFxSlots[i].set("text", fxSlots[i].getCurrentEffectId());

        local area = [0, 0, this.getWidth(), this.getHeight()];

        // FIX: determine bypass state by slot index, not effect name
        local slotIndex = pnlFxSlots.indexOf(this);
        local btn = slotIndex != -1 ? bypassButtons[slotIndex] : 0;

        g.setColour(btn && btn.getValue() ? Colours.white : Colours.grey);
        g.drawRoundedRectangle(area, 1.0, 1.0);
        g.drawAlignedText(this.get("text"), area, "centred");

        if (this.data.isTarget)
            g.drawLine(5, area[2] - 5, area[3] - 5, area[3] - 5, 1.0);
    }

    for (p in pnlFxSlots)
        p.setPaintRoutine(paintRoutine);

    inline function repaintAllSlots()
    {
        for (p in pnlFxSlots)
        {
            p.data.isTarget = false;
            p.repaint();
        }
    }

    // ---------------------------
    // Drag visuals
    // ---------------------------

    function dragPaintRoutine(g, obj)
    {
        // Visual affordance while dragging
        var isValid = false;
        isValid = obj.valid;
        isValid |= obj.target == "";
        isValid |= obj.target.length == 0;
        isValid |= obj.target == obj.source;

        if (isValid && obj.target != "" && obj.target != obj.source)
        {
            repaintAllSlots();
            var component = Content.getComponent(obj.target);
            component.data.isTarget = true;
            component.repaint();
        }

        g.setColour(Colours.withAlpha(Colours.white, 0.20));
        g.fillRoundedRectangle(obj.area, 2.0);
        g.setColour(Colours.withAlpha(Colours.white, 0.60));
        g.fillRoundedRectangle(obj.area, 2.0, 2.0);
    }

    inline function checkValidPnlFxSlot(targetId)
    {
        // Only allow drops onto these panels
        local names = ["pnlModularA", "pnlModularB", "pnlModularC", "pnlModularD", "pnlModularE", "pnlModularF", "pnlModularG"];
        Content.refreshDragImage();
        return names.contains(targetId);
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
        // Example: moving forward from 1 -> 6
        //   after removal: [0,2,3,4,5,6] (length 6)
        //   insert at index 6 => append => dragged item becomes last.
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
        // Called when we drag a module panel onto another module panel
        if (!isValid || targetName == "") return;

        var target = Content.getComponent(targetName);
        if (!pnlFxSlots.contains(target)) return;
        if (target == this) return;

        // Compute indices
        var fromIndex = pnlFxSlots.indexOf(this);
        var toIndex = pnlFxSlots.indexOf(target);
        if (fromIndex == -1 || toIndex == -1 || fromIndex == toIndex) return;

        // 1) Snapshot all slot container states before changes
        var states = snapshotStates();

        // 2) Build the new mapping using "insert" semantics
        var mapping = computeInsertMapping(fromIndex, toIndex, fxModules.length);

        // 3) Apply states by mapping in one restore pass
        applyStatesByMapping(states, mapping);

        // 4) Refresh UI (texts, target highlights)
        repaintAllSlots();        
    }

    // ---------------------------
    // Mouse interaction
    // ---------------------------

    inline function dragMouseCallback(event)
    {
        if (event.drag && !event.rightClick)
        {
            if (event.dragX > 10 || event.dragY > 10 || event.dragX < -10 || event.dragY < -10)
            {
                this.startInternalDrag({
                    area: [0, 0, 25, 25],
                    paintRoutine: dragPaintRoutine,
                    dragCallback: insertDragCallback,
                    isValid: checkValidPnlFxSlot
                });
            }
        }
        else if (event.clicked && !event.rightClick)
        {
            // Show the UI panel for the effect under this slot
            hideFXPanels();

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
            this.repaint();
        }
    }

    for (p in pnlFxSlots)
        p.setMouseCallback(dragMouseCallback);

    // ---------------------------
    // Bind Parameters
    // ---------------------------


    
    inline function onknbModularControl(component, value)
    {
        local text = component.get("text");
        local idx = text.indexOf("_");
        local mod = text.substring(0, idx);
        local param = text.substring(idx + 1, text.length);        
        
        for (slot in fxSlots)
            if (slot.getCurrentEffectId() == mod)
            {
                local effect = slot.getCurrentEffect();
                local index = effect.getAttributeIndex(param);
                effect.setAttribute(index, value);                
            }

        // Additional per-component logic goes here
        if (component == knbAmpControl[0]) // amp mode
        {
            if (value < 2) { pnlAmpNAMLoader.set("visible", false); }
            else { pnlAmpNAMLoader.set("visible", true); }
        }
    }    

    for (k in knbAmpControl) { k.setControlCallback(onknbModularControl); }
    for (k in knbCabControl) { k.setControlCallback(onknbModularControl); }
    
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