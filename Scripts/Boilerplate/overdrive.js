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

namespace Overdrive
{        
    const clrDarkgrey = 0xFF252525;   
    const clrWhite = 0xFFFFFFFF;
    const clrExtradarkgrey = 0xFF171717;
    const clrGrey = 0xFF808080;   
    const pad = 8;
    const bounds = [pad, pad, 850 - pad * 2, 400 - pad * 2];

    const pnlOverdrive = Content.getComponent("pnlOverdrive");

    pnlOverdrive.setPaintRoutine(function(g)
    {
        g.setColour(clrExtradarkgrey);
        g.fillRoundedRectangle(bounds, 32.0);
        g.setColour(clrDarkgrey);
        g.drawRoundedRectangle(bounds, 32.0, 3.0);
    });
}