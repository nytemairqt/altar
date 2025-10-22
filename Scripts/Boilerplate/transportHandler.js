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

namespace TransportHandler
{        
    const grm = Engine.getGlobalRoutingManager();  
    const tempoCable = grm.getCable("tempo");     
    tempoCable.setRange(20.0, 260.0);
    const transportHandler = Engine.createTransportHandler();      
    const lblDelayBPM = Content.getComponent("lblDelayBPM");          

    inline function tempoChange(newTempo)
    {
		lblDelayBPM.set("text", newTempo);

        tempoCable.sendData(newTempo);        
    }

    transportHandler.setOnTempoChange(SyncNotification, tempoChange); // defer to UI thread else bad juju       
}