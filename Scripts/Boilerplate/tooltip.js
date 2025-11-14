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

namespace Tooltip
{
    const pnlTooltip = Content.getComponent("pnlTooltip");

    pnlTooltip.setPaintRoutine(function(g)
    {
        var tooltip = Content.getCurrentTooltip();
        g.setColour(ColourData.clrWhite);
        g.setFont("Arial", 14);
        
        g.drawAlignedText(tooltip, [0, 0, this.getWidth(), this.getHeight()], "centred");
    });

    pnlTooltip.setTimerCallback(function()
    {
         this.repaint();
    });

    pnlTooltip.startTimer(17);
}

