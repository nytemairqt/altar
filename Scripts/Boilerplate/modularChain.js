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

    const fxSlots = [
        Synth.getSlotFX("modularA"),
        Synth.getSlotFX("modularB"),
        Synth.getSlotFX("modularC"),
        Synth.getSlotFX("modularD"),
        Synth.getSlotFX("modularE"),
        Synth.getSlotFX("modularF"),
        Synth.getSlotFX("modularG")
    ];

    // These are the slot containers (not the inner effects)
    const fxModules = [
        Synth.getEffect("modularA"),
        Synth.getEffect("modularB"),
        Synth.getEffect("modularC"),
        Synth.getEffect("modularD"),
        Synth.getEffect("modularE"),
        Synth.getEffect("modularF"),
        Synth.getEffect("modularG")
    ];

    // Draggable slot panels (one per slot position)
    const pnlFxSlots = [
        Content.getComponent("pnlModularA"),
        Content.getComponent("pnlModularB"),
        Content.getComponent("pnlModularC"),
        Content.getComponent("pnlModularD"),
        Content.getComponent("pnlModularE"),
        Content.getComponent("pnlModularF"),
        Content.getComponent("pnlModularG")
    ];

    // Effect control panels (one per effect type)
    const pnlFx = [
        Content.getComponent("pnlOverdrive"),
        Content.getComponent("pnlAmp"),
        Content.getComponent("pnlCab"),
        Content.getComponent("pnlReverb"),
        Content.getComponent("pnlDelay"),
        Content.getComponent("pnlChorus"),
        Content.getComponent("pnlRingmod")
    ];

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
        g.setColour(Colours.white);
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
        local names = [
            "pnlModularA", "pnlModularB", "pnlModularC",
            "pnlModularD", "pnlModularE", "pnlModularF", "pnlModularG"
        ];

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

        // 5) Update effect-to-slot map and rebind UI controls
        rebuildEffectMap();
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
            // Reserved for context menu or quick actions
        }
    }

    for (p in pnlFxSlots)
        p.setMouseCallback(dragMouseCallback);

    // ---------------------------
    // Effect-to-slot map and UI bindings
    // ---------------------------

    // Effect type to current slot index map, rebuilt on demand
    reg effectToSlot = {};

    // Parameter index maps (update as required to match your DSP)
    const OVERDRIVE_PARAMS = { Gain: 0, Tone: 1, Mix: 2 };
    const AMP_PARAMS       = { Mode: 0, Input: 1, Low: 2, Mid: 3, High: 4, Presence: 5, Output: 6 };
    const CAB_PARAMS       = {
        Mix: 0,
        CabAEnable: 1, CabADelay: 2, CabAAxis: 3, CabADistance: 4, CabAPhase: 5, CabAPan: 6, CabAGain: 7,
        CabBEnable: 8, CabBDelay: 9, CabBAxis: 10, CabBDistance: 11, CabBPhase: 12, CabBPan: 13, CabBGain: 14
    };

    inline function safeGetComponent(id)
    {
        // Guard to avoid errors if a component is missing
        local c = Content.getComponent(id);
        return isDefined(c) ? c : undefined;
    }

    inline function bindPanelToEffect(effectId, componentPairs)
    {
        // Binds a list of components to the processor / parameter ids of the current slot for an effect
        if (!isDefined(effectToSlot[effectId])) return;

        local slotIndex = effectToSlot[effectId];
        local procId = fxSlots[slotIndex].getCurrentEffectId(); // the effect module id currently in that slot

        for (pair in componentPairs)
        {
            if (!isDefined(pair) || !isDefined(pair.comp)) continue;
            pair.comp.set("processorId", procId);
            pair.comp.set("parameterId", pair.param);
        }
    }

    inline function bindOverdrivePanel()
    {
        // Only bind if components exist
        bindPanelToEffect("overdrive", [
            { comp: safeGetComponent("knbODGain"), param: OVERDRIVE_PARAMS.Gain },
            { comp: safeGetComponent("knbODTone"), param: OVERDRIVE_PARAMS.Tone },
            { comp: safeGetComponent("knbODMix"),  param: OVERDRIVE_PARAMS.Mix }
        ]);
    }

    inline function bindAmpPanel()
    {
        bindPanelToEffect("amp", [
            { comp: safeGetComponent("cmbAmpMode"),  param: AMP_PARAMS.Mode },
            { comp: safeGetComponent("knbAmpInput"), param: AMP_PARAMS.Input },
            { comp: safeGetComponent("knbAmpLow"),   param: AMP_PARAMS.Low },
            { comp: safeGetComponent("knbAmpMid"),   param: AMP_PARAMS.Mid },
            { comp: safeGetComponent("knbAmpHigh"),  param: AMP_PARAMS.High },
            { comp: safeGetComponent("knbAmpPres"),  param: AMP_PARAMS.Presence },
            { comp: safeGetComponent("knbAmpOut"),   param: AMP_PARAMS.Output }
        ]);
    }

    inline function bindCabPanel()
    {
        bindPanelToEffect("cab", [
            { comp: safeGetComponent("knbCabMix"),        param: CAB_PARAMS.Mix },
            { comp: safeGetComponent("btnCabAEnable"),    param: CAB_PARAMS.CabAEnable },
            { comp: safeGetComponent("knbCabADelay"),     param: CAB_PARAMS.CabADelay },
            { comp: safeGetComponent("knbCabAAxis"),      param: CAB_PARAMS.CabAAxis },
            { comp: safeGetComponent("knbCabADistance"),  param: CAB_PARAMS.CabADistance },
            { comp: safeGetComponent("btnCabAPhase"),     param: CAB_PARAMS.CabAPhase },
            { comp: safeGetComponent("knbCabAPan"),       param: CAB_PARAMS.CabAPan },
            { comp: safeGetComponent("knbCabAGain"),      param: CAB_PARAMS.CabAGain },

            { comp: safeGetComponent("btnCabBEnable"),    param: CAB_PARAMS.CabBEnable },
            { comp: safeGetComponent("knbCabBDelay"),     param: CAB_PARAMS.CabBDelay },
            { comp: safeGetComponent("knbCabBAxis"),      param: CAB_PARAMS.CabBAxis },
            { comp: safeGetComponent("knbCabBDistance"),  param: CAB_PARAMS.CabBDistance },
            { comp: safeGetComponent("btnCabBPhase"),     param: CAB_PARAMS.CabBPhase },
            { comp: safeGetComponent("knbCabBPan"),       param: CAB_PARAMS.CabBPan },
            { comp: safeGetComponent("knbCabBGain"),      param: CAB_PARAMS.CabBGain }
        ]);
    }

    inline function rebindParameters()
    {
        // Reconnect UI elements to the effect instance in its current slot
        bindOverdrivePanel();
        bindAmpPanel();
        bindCabPanel();

        // Add additional binders (reverb, delay, chorus, ringmod) similarly if needed.
    }

    inline function rebuildEffectMap()
    {
        // Build a map of effectId -> slotIndex, then rebind UI
        effectToSlot = {};
        for (i = 0; i < fxSlots.length; i++)
            effectToSlot[fxSlots[i].getCurrentEffectId()] = i;

        rebindParameters();
    }

    // Initial binding pass (uncomment if you'd like to bind on load)
    // rebuildEffectMap();
}