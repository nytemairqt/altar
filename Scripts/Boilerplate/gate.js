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

namespace Gate
{
	const p = Content.createPath();    

    const gate = Synth.getEffect("gate");
    const btnGate = Content.getComponent("btnGate");
    const knbGateThreshold = Content.getComponent("knbGateThreshold");
    const btnGateSettings = Content.getComponent("btnGateSettings");       	

    const knbGateAttack = Content.getComponent("knbGateAttack");
    const knbGateRelease = Content.getComponent("knbGateRelease");
    const knbTranspose = Content.getComponent("knbTranspose");
    const knbOctave = Content.getComponent("knbOctave");

    inline function onbtnGateSettingsControl(component, value)
    {
        knbGateAttack.set("visible", value);
        knbGateRelease.set("visible", value);
        knbTranspose.set("visible", 1-value);
        knbOctave.set("visible", 1-value);
    }

    btnGateSettings.setControlCallback(onbtnGateSettingsControl);     
    
}