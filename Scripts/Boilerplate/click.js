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
    const click = Synth.getChildSynth("click");
    const clickMIDI = Synth.getMidiPlayer("clickMIDI");
    const btnShowClick = Content.getComponent("btnShowClick");
    const btnClick = Content.getComponent("btnClick");
    const knbClickGain = Content.getComponent("knbClickGain");
    const lblClickDisasbled = Content.getComponent("lblClickDisasbled");
    const pnlClick = Content.getComponent("pnlClick");
    const isPlugin = Engine.isPlugin();    

    inline function addNote(eventListToUse, type, channel, noteNumber, vel, timestamp)
    {
        /* Adds a single MIDI note to the midiList */
        local message = Engine.createMessageHolder();
        message.setType(type);
        message.setChannel(channel);
        message.setNoteNumber(noteNumber);
        message.setVelocity(vel);
        message.setTimestamp(timestamp);
        eventListToUse.push(message);
    }

    inline function setupClick()
    {
        /* Flushes the click list with a new 4/4 counter */
        if (!clickMIDI.isEmpty())
            return;

        clickMIDI.create(4, 4, 1);
        
        local list = [];
        
        addNote(list, 1, 1, 84, 127, 0);
        addNote(list, 2, 1, 84, 127, 2756);
        addNote(list, 1, 1, 84, 127, 22050);
        addNote(list, 2, 1, 84, 127, 24806);
        addNote(list, 1, 1, 84, 127, 44100);
        addNote(list, 2, 1, 84, 127, 46856);
        addNote(list, 1, 1, 84, 127, 66150);
        addNote(list, 2, 1, 84, 127, 68906); // these might vary based on sampleRate...
        
        clickMIDI.flushMessageList(list);
    }

    inline function onbtnShowClickControl(component, value)
    {
        /* Toggles the visibility of the click panel */
        pnlClick.set("visible", value);
        if (!value)
        {
            btnClick.setValue(0);   
            btnClick.changed();
        }
    }

    pnlClick.setPaintRoutine(function(g)
    {
        var bounds = [310, 80, 530, 310];

        g.setColour(Colours.withAlpha(Colours.black, 1.0));
        g.fillRoundedRectangle(bounds, 2.0);
    });

    pnlClick.setMouseCallback(function(event)
    {
        var x = 310;
        var y = 80;
        var w = 530;
        var h = 310;
        
        if (event.mouseDownX < x || event.mouseDownX > (x + w) || event.mouseDownY < y || event.mouseDownY > (y + h)) 
        {
            btnShowClick.setValue(0);
            btnShowClick.changed();
        }   
    });

    if (isPlugin)
    {
        /* disables the click in the plugin version */
        btnClick.set("enabled", false);
        knbClickGain.set("enabled", false);
        lblClickDisasbled.set("visible", true); 
    }

    //cabDesignerMIDIPlayer.create(4, 4, 1);

    // keep me here
    reg clickMIDIList = [      
      "MessageHolder: Type: NoteOn, Channel: 1, Number: 84, Value: 100, EventId: 0, Timestamp: 0, ",
      "MessageHolder: Type: NoteOff, Channel: 1, Number: 84, Value: 64, EventId: 0, Timestamp: 2756, ",
      "MessageHolder: Type: NoteOn, Channel: 1, Number: 84, Value: 100, EventId: 1, Timestamp: 22050, ",
      "MessageHolder: Type: NoteOff, Channel: 1, Number: 84, Value: 64, EventId: 1, Timestamp: 24806, ",
      "MessageHolder: Type: NoteOn, Channel: 1, Number: 84, Value: 100, EventId: 2, Timestamp: 44100, ",
      "MessageHolder: Type: NoteOff, Channel: 1, Number: 84, Value: 64, EventId: 2, Timestamp: 46856, ",
      "MessageHolder: Type: NoteOn, Channel: 1, Number: 84, Value: 100, EventId: 3, Timestamp: 66150, ",
      "MessageHolder: Type: NoteOff, Channel: 1, Number: 84, Value: 64, EventId: 3, Timestamp: 68906, "
    ];

    inline function onbtnClickControl(component, value)
    {
        /* enables / flushes the click */
        click.setBypassed(1-value);
        if (value)
        {
            setupClick();
            clickMIDI.play(0);
        }
        else
            clickMIDI.stop(0);
    }

    btnClick.setControlCallback(onbtnClickControl);
    btnShowClick.setControlCallback(onbtnShowClickControl);
   
}