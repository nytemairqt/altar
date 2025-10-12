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
	const fxSlots = [Synth.getSlotFX("modularA"), Synth.getSlotFX("modularB"), Synth.getSlotFX("modularC"), Synth.getSlotFX("modularD"), Synth.getSlotFX("modularE"), Synth.getSlotFX("modularF"), Synth.getSlotFX("modularG")];
	const pnlCabALoader = Content.getComponent("pnlCabALoader");
	const pnlCabBLoader = Content.getComponent("pnlCabBLoader");
	Engine.loadAudioFilesIntoPool();
		
	// Drop
	inline function pnlCabALoaderDrop(f)
    {
	    if(f.drop)
	    {
			local file = FileSystem.fromAbsolutePath(f.fileName);
			switch (this)
			{
				case pnlCabALoader:
					for (slot in fxSlots)
					{
						local effectId = slot.getCurrentEffectId();
						
						if (effectId == "cab")
						{
							local id = slot.getCurrentEffect().getId();
							local ref = Synth.getAudioSampleProcessor(id);
							ref.setFile(f.fileName);							
						}
					}
						
					break;
				case pnlCabBLoader:
					break;
			}
		    //local json = file.loadAsObject();	
		    //namCable.sendData(json);	    
		    this.set("text", file.toString(3));
		    this.repaint();
	    }
    }
    
    pnlCabALoader.setFileDropCallback("All Callbacks", "*.wav", pnlCabALoaderDrop);
    pnlCabBLoader.setFileDropCallback("All Callbacks", "*.wav", pnlCabALoaderDrop);
    
    // Right Click
    inline function pnlCabLoaderClick(event)
    {
		if (event.clicked && event.rightClick)
		{
			// open file browser
			FileSystem.browse(FileSystem.AudioFiles, false, "*.wav", function(result)
			{
				// "result" is the file
				this.set("text", result.toString(3));
				this.repaint();
			});
		}        
    }
    
    pnlCabALoader.setMouseCallback(pnlCabLoaderClick);
    pnlCabBLoader.setMouseCallback(pnlCabLoaderClick);
    
    inline function pnlCabLoaderPaintRoutine(g)
    {
	    g.setColour(Colours.white);
        g.drawRoundedRectangle([0, 0, this.getWidth(), this.getHeight()], 0.0, 1.0);
 	   	g.drawAlignedText(this.get("text"), [0, 0, this.getWidth(), this.getHeight()], "centred") ;
    }
    
    pnlCabALoader.setPaintRoutine(pnlCabLoaderPaintRoutine);
   	pnlCabBLoader.setPaintRoutine(pnlCabLoaderPaintRoutine);
}