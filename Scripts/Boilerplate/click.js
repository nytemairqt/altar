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

namespace Click
{		    
    const click = Synth.getEffect("click");            
    const btnShowClick = Content.getComponent("btnShowClick");
    const btnClick = Content.getComponent("btnClick");
    const knbClickGain = Content.getComponent("knbClickGain");
    const knbClickTempo = Content.getComponent("knbClickTempo");
    const pnlClick = Content.getComponent("pnlClick");
    const isPlugin = Engine.isPlugin();
    const isHISE = Engine.isHISE();     
    const bounds = [160, 50, 240, 120];   

    inline function onbtnShowClickControl(component, value)
    {
        pnlClick.set("visible", value);        
    }
    
    btnShowClick.setControlCallback(onbtnShowClickControl);
	
    pnlClick.setPaintRoutine(function(g)
    {		
        g.setColour(ColourData.clrExtradarkgrey);
        g.fillRoundedRectangle(bounds, 2.0);
        g.setColour(ColourData.clrGrey);
        g.drawRoundedRectangle(bounds, 2.0, 2.0);        
    });

    pnlClick.setMouseCallback(function(event)
    {
        var x = bounds[0];
        var y = bounds[1];
        var w = bounds[2];
        var h = bounds[3];
        
        if (event.mouseDownX < x || event.mouseDownX > (x + w) || event.mouseDownY < y || event.mouseDownY > (y + h)) 
        {
            btnShowClick.setValue(0);
            btnShowClick.changed();
        }   
    });     
    
    inline function onknbClickTempoControl(component, value)
    {		
    	if (!isPlugin)
	    	Engine.setHostBpm(value);
    }   
    
    knbClickTempo.setControlCallback(onknbClickTempoControl);   
}