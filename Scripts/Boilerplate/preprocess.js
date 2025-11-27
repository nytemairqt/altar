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

namespace Preprocess
{    
    const pnlPreprocess = Content.getComponent("pnlPreprocess");
    const btnShowPreprocess = Content.getComponent("btnShowPreprocess");          
    Engine.addModuleStateToUserPreset("preprocessEQ");
    const preprocessComp = Synth.getEffect("preprocessComp");
    const pnlPreprocessCompGR = Content.getComponent("pnlPreprocessCompGR");   
    const pnlPreprocessCompGRTimer = Engine.createTimerObject();     
    reg gr = 0.0;

    const padLeft = 154;
    const padTop = 50;
    const padRight = 308;
    const padBottom = 57;    
    const bounds = [padLeft, padTop, pnlPreprocess.getWidth() - padRight, pnlPreprocess.getHeight() - padBottom];
        
    inline function onbtnShowPreprocessControl(component, value) { pnlPreprocess.set("visible", value); }
    
    btnShowPreprocess.setControlCallback(onbtnShowPreprocessControl);
        
    pnlPreprocess.setMouseCallback(function(event)
    {		
        var x = bounds[0];
        var y = bounds[1];
        var w = bounds[2];
        var h = bounds[3];
        
        if (event.mouseDownX < x || event.mouseDownX > (x + w) || event.mouseDownY < y || event.mouseDownY > (y + h)) 
        {
            btnShowPreprocess.setValue(0);
            btnShowPreprocess.changed();
        }   
    });
    
    inline function pnlPreprocessCompGRTimerCallback()
    {
    	if (!preprocessComp.isBypassed()) { gr = 1-preprocessComp.getAttribute(preprocessComp.CompressorReduction); }
    	else { gr = 0.0; }   
	    pnlPreprocessCompGR.repaint();	    	    
    }
    
    pnlPreprocessCompGRTimer.setTimerCallback(pnlPreprocessCompGRTimerCallback);    
    pnlPreprocessCompGRTimer.startTimer(100);

    pnlPreprocess.setPaintRoutine(function(g)
    {
        g.setColour(Colours.withAlpha(ColourData.clrComponentBGGrey, 1.0));
        g.fillRoundedRectangle(bounds, 2.0);
        g.setColour(ColourData.clrMidgrey);
        g.drawRoundedRectangle(bounds, 2.0, 3.0);        
    });
    
    pnlPreprocessCompGR.setPaintRoutine(function(g)
    {
	    g.setColour(Colours.withAlpha(ColourData.clrComponentBGGrey, 1.0));
	    g.fillRoundedRectangle(bounds, 2.0);	    	    
	    g.setColour(ColourData.clrGrey);	    
	    g.fillRect([0, 0, this.getWidth(), 1]);
	    g.fillRect([this.getWidth() - 1, 0, 1, this.getHeight()]);
	    g.fillRect([0, 0, this.getWidth(), this.getHeight() * gr]);
    });
    
}