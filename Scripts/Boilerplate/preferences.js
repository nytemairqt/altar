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

namespace Preferences
{
	const btnShowPreferences = Content.getComponent("btnShowPreferences");
	const pnlPreferences = Content.getComponent("pnlPreferences");		
	
	inline function onbtnShowPreferencesControl(component, value)
	{
		pnlPreferences.set("visible", value);
	};
	
	btnShowPreferences.setControlCallback(onbtnShowPreferencesControl);
	
	pnlPreferences.setPaintRoutine(function(g)
    {
        var bounds = [310, 80, 530, 310];

        g.setColour(Colours.withAlpha(Colours.black, 1.0));
        g.fillRoundedRectangle(bounds, 2.0);
    });

    pnlPreferences.setMouseCallback(function(event)
    {
        var x = 310;
        var y = 80;
        var w = 530;
        var h = 310;
        
        if (event.mouseDownX < x || event.mouseDownX > (x + w) || event.mouseDownY < y || event.mouseDownY > (y + h)) 
        {
            btnShowPreferences.setValue(0);
            btnShowPreferences.changed();
        }   
    });
	
}