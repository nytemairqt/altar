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
	const cmbCabALoader = Content.getComponent("cmbCabALoader");
	const cmbCabBLoader = Content.getComponent("cmbCabBLoader");
	const btnCabAUnload = Content.getComponent("btnCabAUnload");
    const btnCabBUnload = Content.getComponent("btnCabBUnload");
    const btnCabALoadPrev = Content.getComponent("btnCabALoadPrev");
    const btnCabALoadNext = Content.getComponent("btnCabALoadNext");
    const btnCabBLoadPrev = Content.getComponent("btnCabBLoadPrev");
    const btnCabBLoadNext = Content.getComponent("btnCabBLoadNext");	
	const audioFiles = Engine.loadAudioFilesIntoPool();
	
	inline function initializeCabs()
	{		
		cmbCabALoader.set("items", "");
		cmbCabBLoader.set("items", "");
		for (file in audioFiles)
		{				
			if (!file.contains("_click") && !file.contains("_dirac") && !file.contains("_testAudio")) { cmbCabALoader.addItem(file); cmbCabBLoader.addItem(file); }
		}
	}
	
	initializeCabs();			

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
    
    inline function oncmbAmpLoaderControl(component, value)
    {		
    	local path = component.getItemText();    
    	local index = path.indexOf("}");    	
    	local prettyName = path.substring(index + 1, path.length);    	
	    for (slot in fxSlots)
		{
			local effectId = slot.getCurrentEffectId();
			if (effectId == "cab")
			{
				local id = slot.getCurrentEffect().getId();
				local ref = Synth.getAudioSampleProcessor(id);	
				local irSlot = component == cmbCabALoader ? ref.getAudioFile(0) : ref.getAudioFile(1);
				irSlot.loadFile(path);
			}
		}	
		if (component == cmbCabALoader) { pnlCabALoader.set("text", prettyName); pnlCabALoader.repaint(); }
		if (component == cmbCabBLoader) { pnlCabBLoader.set("text", prettyName); pnlCabBLoader.repaint(); }
    }
    
    cmbCabALoader.setControlCallback(oncmbAmpLoaderControl);
    cmbCabBLoader.setControlCallback(oncmbAmpLoaderControl);

    // file drop
		
	inline function pnlCabLoaderDrop(f)
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
    
    pnlCabALoader.setFileDropCallback("All Callbacks", "*.wav", pnlCabLoaderDrop);
    pnlCabBLoader.setFileDropCallback("All Callbacks", "*.wav", pnlCabLoaderDrop);   

    // right click load

    reg panelIndex;
    inline function pnlCabLoaderClick(event)
    {
    	panelIndex = this;
    	if (event.clicked && event.rightClick)
		{
		    FileSystem.browse(FileSystem.AudioFiles, false, "*.wav", function(result)
			{
				if (!result || result.toString(0) == "") { return; }
				for (slot in fxSlots)
				{
					var effectId = slot.getCurrentEffectId();
					if (effectId == "cab")
					{
						var id = slot.getCurrentEffect().getId();
						var ref = Synth.getAudioSampleProcessor(id);	
						var irSlot = panelIndex == pnlCabALoader ? ref.getAudioFile(0) : ref.getAudioFile(1);			
						irSlot.loadFile(result.toString(0));
					}
				}					    
				panelIndex.set("text", result.toString(3));
				panelIndex.repaint();
			});
		}
		else if (event.hover) { pnlTooltip.set("text", this.get("tooltip")); }
    }
    
    pnlCabALoader.setMouseCallback(pnlCabLoaderClick);
	pnlCabBLoader.setMouseCallback(pnlCabLoaderClick);

	// unload

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
    
    // left / right cycle

    reg fileList;  
    inline function onbtnCabCycleControl(component, value)
    {	
    	local usingProjectFolder = false;
    	if (!value) { return; }    	
    	for (slot in fxSlots)
		{
			local effectId = slot.getCurrentEffectId();
			if (effectId == "cab")
			{
				local id = slot.getCurrentEffect().getId();
				local ref = Synth.getAudioSampleProcessor(id);	

				if (component == btnCabALoadPrev || component == btnCabALoadNext) { local irSlot = ref.getAudioFile(0); local panel = pnlCabALoader;}
				else { local irSlot = ref.getAudioFile(1); local panel = pnlCabBLoader;}

				local fileName = irSlot.getCurrentlyLoadedFile(); 
				if (fileName == "") { return; }				
				local fileToString = FileSystem.fromReferenceString(fileName, FileSystem.AudioFiles).toString(0); 
				
				if (fileName.contains("{PROJECT_FOLDER}"))
				{													
					switch (component)
					{
						case btnCabALoadPrev: 
							local index = cmbCabALoader.getValue();
							local newValue = index > cmbCabALoader.get("min") ? index - 1 : cmbCabALoader.get("max");
							cmbCabALoader.setValue(newValue);
							cmbCabALoader.changed();
							break;
						case btnCabALoadNext: 
							local index = cmbCabALoader.getValue();
							local newValue = index < cmbCabALoader.get("max") ? index + 1 : cmbCabALoader.get("min");
							cmbCabALoader.setValue(newValue);
							cmbCabALoader.changed();
							break;
						case btnCabBLoadPrev: 
							local index = cmbCabBLoader.getValue();
							local newValue = index > cmbCabBLoader.get("min") ? index - 1 : cmbCabBLoader.get("max");
							cmbCabBLoader.setValue(newValue);
							cmbCabBLoader.changed();
							break;
						case btnCabBLoadNext: 
							local index = cmbCabBLoader.getValue();
							local newValue = index < cmbCabBLoader.get("max") ? index + 1 : cmbCabBLoader.get("min");
							cmbCabBLoader.setValue(newValue);
							cmbCabBLoader.changed();
							break;
						default: return;
					}
					return;
				}
									
				local parent = FileSystem.fromReferenceString(fileName, FileSystem.AudioFiles).getParentDirectory();
				fileList = FileSystem.findFiles(parent, "*.wav", false);									
				for (i=0; i<fileList.length; i++) { if (fileList[i].toString(0) == fileToString) { local index = i; } }					
				
				if (component == btnCabALoadPrev || component == btnCabBLoadPrev)		
				{
					if (index == 0) { index = fileList.length - 1; }
					else (index -= 1);
				}	
				else
				{
					if (index == fileList.length - 1) { index = 0; }
					else (index += 1);
				}
								
				irSlot.loadFile(fileList[index].toString(0));
				panel.set("text", fileList[index].toString(3));
				panel.repaint();
			}
		}	
    }

    btnCabALoadPrev.setControlCallback(onbtnCabCycleControl);
	btnCabALoadNext.setControlCallback(onbtnCabCycleControl);
	btnCabBLoadPrev.setControlCallback(onbtnCabCycleControl);
	btnCabBLoadNext.setControlCallback(onbtnCabCycleControl);    
	
	const LAFComboBoxCabLoader = Content.createLocalLookAndFeel();

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
    
    inline function drawComboBox(g, obj)
    {		
    	local area = [0, 0, obj.area[2], obj.area[3]];
    	local pad = 6;
    	local width = 4;
    	g.setColour(obj.hover ? ColourData.clrWhite : ColourData.clrLightgrey);    	    	
    	g.fillEllipse([pad, 22, 4, 4]);
    	g.fillEllipse([area[2] / 2 - (width / 2), 22, 4, 4]);
    	g.fillEllipse([area[2] - (pad + width), 22, 4, 4]);
    }

    inline function drawComboBoxItem(g, obj)
    {
    	local area = [0, 0, obj.area[2], obj.area[3]];
    	local index = obj.text.lastIndexOf("}");
    	local text = obj.text.substring(index + 1, obj.text.length);
    	
    	g.fillAll(obj.isHighlighted ? ColourData.clrDarkgrey : ColourData.clrComponentBGGrey);    	    					
    	g.setColour(ColourData.clrLightgrey);
    	g.drawAlignedText(text, area, "centred");
    }
    
    inline function setComboBoxItemSize(g, obj) { return [150, 27]; }

	LAFComboBoxCabLoader.registerFunction("drawComboBox", drawComboBox);
	LAFComboBoxCabLoader.registerFunction("drawPopupMenuItem", drawComboBoxItem);
	LAFComboBoxCabLoader.registerFunction("getIdealPopupMenuItemSize", setComboBoxItemSize);
	
	cmbCabALoader.setLocalLookAndFeel(LAFComboBoxCabLoader);
	cmbCabBLoader.setLocalLookAndFeel(LAFComboBoxCabLoader);

}