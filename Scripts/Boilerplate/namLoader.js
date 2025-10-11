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

namespace NAMLoader
{
    /* GLOBAL CABLE */

    const grm = Engine.getGlobalRoutingManager();
    const namCable = grm.getCable("nam");
    const pnlAmpNAMLoader = Content.getComponent("pnlAmpNAMLoader");            
   
    inline function pnlAmpNAMLoaderDrop(f)
    {
	    if(f.drop)
	    {
			local file = FileSystem.fromAbsolutePath(f.fileName);
		    local json = file.loadAsObject();	
		    namCable.sendData(json);	    
		    pnlAmpNAMLoader.set("text", file.toString(3));
		    pnlAmpNAMLoader.repaint();
	    }
    }
    
    pnlAmpNAMLoader.setFileDropCallback("All Callbacks", "*.nam, *.json", pnlAmpNAMLoaderDrop);
    
    inline function pnlAmpNAMLoaderClick(event)
    {
		if (event.clicked && event.rightClick)
		{
			// open file browser
			FileSystem.browse(FileSystem.Documents, false, "*.nam, *.json", function(result)
			{
				var json = result.loadAsObject();
				namCable.sendData(json);
				pnlAmpNAMLoader.set("text", result.toString(3));
				pnlAmpNAMLoader.repaint();
			});
		}        
    }
    
    pnlAmpNAMLoader.setMouseCallback(pnlAmpNAMLoaderClick);
    
    pnlAmpNAMLoader.setPaintRoutine(function(g)
    {
	   g.setColour(Colours.white);
       g.drawRoundedRectangle([0, 0, this.getWidth(), this.getHeight()], 0.0, 1.0);
	   g.drawAlignedText(this.get("text"), [0, 0, this.getWidth(), this.getHeight()], "centred") ;
    });
        
}