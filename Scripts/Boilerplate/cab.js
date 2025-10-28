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

namespace Cab
{   
	const controls = [Content.getComponent("btnCabAEnable"), Content.getComponent("btnCabAPhase"), Content.getComponent("knbCabAGain"), Content.getComponent("knbCabAPan"), Content.getComponent("knbCabAAxis"), Content.getComponent("knbCabADistance"), Content.getComponent("knbCabADelay"), Content.getComponent("btnCabBEnable"), Content.getComponent("btnCabBPhase"), Content.getComponent("knbCabBGain"), Content.getComponent("knbCabBPan"), Content.getComponent("knbCabBAxis"), Content.getComponent("knbCabBDistance"), Content.getComponent("knbCabBDelay"), Content.getComponent("knbCabMix")];	
	const fxSlots = [Synth.getSlotFX("modularA"), Synth.getSlotFX("modularB"), Synth.getSlotFX("modularC"), Synth.getSlotFX("modularD"), Synth.getSlotFX("modularE"), Synth.getSlotFX("modularF"), Synth.getSlotFX("modularG")];	
	const pnlCabALoader = Content.getComponent("pnlCabALoader");
	const pnlCabBLoader = Content.getComponent("pnlCabBLoader");
	const pnlCab = Content.getComponent("pnlCab");
	const pnlTooltip = Content.getComponent("pnlTooltip");	
	const btnCabAUnload = Content.getComponent("btnCabAUnload");
    const btnCabBUnload = Content.getComponent("btnCabBUnload");
	
	Engine.loadAudioFilesIntoPool();

    inline function onControl(component, value)
    {
        local attribute = component.get("text");
        
        for (slot in fxSlots)
            if (slot.getCurrentEffectId() == "cab")
            {
                local effect = slot.getCurrentEffect();
                local index = effect.getAttributeIndex(attribute);
                effect.setAttribute(index, value);                
            }
    }   
	
    for (control in controls) { control.setControlCallback(onControl); }
		
	// Drop
	inline function pnlCabALoaderDrop(f)
    {
	    if(f.drop)
	    {			
			for (slot in fxSlots)
			{
				local effectId = slot.getCurrentEffectId();
				if (effectId == "cab")
				{
					local id = slot.getCurrentEffect().getId();
					local ref = Synth.getAudioSampleProcessor(id);	
					local irSlot = this == pnlCabALoader ? ref.getAudioFile(0) : ref.getAudioFile(1);
					irSlot.loadFile(f.fileName);
				}
			}					    
		    local file = FileSystem.fromAbsolutePath(f.fileName);
		    this.set("text", file.toString(3));
		    this.repaint();
	    }
    }
    
    pnlCabALoader.setFileDropCallback("All Callbacks", "*.wav", pnlCabALoaderDrop);
    pnlCabBLoader.setFileDropCallback("All Callbacks", "*.wav", pnlCabALoaderDrop);   
    
    // Right Click (2 separate ones cause i can't figure out lambdas lol)    
    inline function pnlCabALoaderClick(event)
    {
	    if (event.clicked && event.rightClick)
		{
		    FileSystem.browse(FileSystem.AudioFiles, false, "*.wav", function(result)
			{
				if (!result)
					return;
				for (slot in fxSlots)
				{
					var effectId = slot.getCurrentEffectId();
					if (effectId == "cab")
					{
						var id = slot.getCurrentEffect().getId();
						var ref = Synth.getAudioSampleProcessor(id);	
						var irSlot = ref.getAudioFile(0);
						irSlot.loadFile(result.toString(0));
					}
				}					    
				pnlCabALoader.set("text", result.toString(3));
				pnlCabALoader.repaint();
			});
		}
		else if (event.hover) { pnlTooltip.set("text", this.get("tooltip")); }
    }       
    
    inline function pnlCabBLoaderClick(event)
    {
		if (event.clicked && event.rightClick)
		{
		    FileSystem.browse(FileSystem.AudioFiles, false, "*.wav", function(result)
			{
				if (!result)
					return;
				for (slot in fxSlots)
				{
					var effectId = slot.getCurrentEffectId();
					if (effectId == "cab")
					{
						var id = slot.getCurrentEffect().getId();
						var ref = Synth.getAudioSampleProcessor(id);	
						var irSlot = ref.getAudioFile(1);
						irSlot.loadFile(result.toString(0));
					}
				}		
				pnlCabBLoader.set("text", result.toString(3));
				pnlCabBLoader.repaint();			    				
			});			
		}
		else if (event.hover) { pnlTooltip.set("text", this.get("tooltip")); }
    }       
    
    pnlCabALoader.setMouseCallback(pnlCabALoaderClick);
    pnlCabBLoader.setMouseCallback(pnlCabBLoaderClick);

    inline function onbtnCabDesignerUnloadCabControl(component, value)
    {
	    if (!value) { return; }
	    for (slot in fxSlots)
		{
			local effectId = slot.getCurrentEffectId();
			if (effectId == "cab")
			{
				local id = slot.getCurrentEffect().getId();
				local ref = Synth.getAudioSampleProcessor(id);	
				if (component == btnCabAUnload) {local irSlot = ref.getAudioFile(0); irSlot.loadFile(""); pnlCabALoader.set("text", "Drag cab file, right click to browse."); pnlCabALoader.repaint();}
				if (component == btnCabBUnload) {local irSlot = ref.getAudioFile(1); irSlot.loadFile(""); pnlCabBLoader.set("text", "Drag cab file, right click to browse."); pnlCabBLoader.repaint();}							
			}
		}		    	    
    }
    
    btnCabAUnload.setControlCallback(onbtnCabDesignerUnloadCabControl);
    btnCabBUnload.setControlCallback(onbtnCabDesignerUnloadCabControl);

    // Look And Feel
    const pad = 8;
    const bounds = [pad, pad, 850 - pad * 2, 400 - pad * 2];
    
    inline function pnlCabLoaderPaintRoutine(g)
    {
	    g.setColour(ColourData.clrComponentBGGrey);
	    g.fillRoundedRectangle([0, 0, this.getWidth(), this.getHeight()], 4.0);  
	    g.setColour(ColourData.clrDarkgrey);
	    g.drawRoundedRectangle([0, 0, this.getWidth(), this.getHeight()], 4.0, 3.0);  
        g.setColour(ColourData.clrWhite);
 	   	g.drawAlignedText(this.get("text"), [0, 0, this.getWidth(), this.getHeight()], "centred") ;
    }
    
    pnlCabALoader.setPaintRoutine(pnlCabLoaderPaintRoutine);
   	pnlCabBLoader.setPaintRoutine(pnlCabLoaderPaintRoutine);

	pnlCab.loadImage("{PROJECT_FOLDER}bgCab.jpg", "bg");
    pnlCab.loadImage("{PROJECT_FOLDER}trim.png", "trim");
    pnlCab.setPaintRoutine(function(g)
    {               
        var stripHeight = 140;
        g.drawImage("bg", bounds, 0, 0);
        g.drawImage("trim", bounds, 0, 0);
        g.setColour(ColourData.clrComponentBGGrey);
        g.fillRoundedRectangle([pad, this.getHeight() / 2 - (stripHeight / 2), this.getWidth() - pad * 2, stripHeight], 2.0);
        g.setColour(ColourData.clrDarkgrey);
        g.drawRoundedRectangle(bounds, 0.0, 3.0);                
        g.drawRoundedRectangle([pad, this.getHeight() / 2 - (stripHeight / 2), this.getWidth() - pad * 2, stripHeight], 2.0, 2.0);
    });


}