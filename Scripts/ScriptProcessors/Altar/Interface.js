Content.makeFrontInterface(1150, 900);

const grm = Engine.getGlobalRoutingManager();
const pitchCable = grm.getCable("pitch");

/*
pitchCable.registerCallback(function(data)
{
	Console.print(data);
}, false);
*/

include("Boilerplate/lookAndFeel.js");
include("Boilerplate/tooltip.js");
include("Boilerplate/click.js");
include("Boilerplate/tuner.js");
include("Boilerplate/cabDesigner.js");
include("Boilerplate/inputChain.js");
include("Boilerplate/modularChain.js");
include("Boilerplate/outputChain.js");
include("Boilerplate/oversampling.js");
	
function onNoteOn()
{
	
}
 function onNoteOff()
{
	
}
 function onController()
{
	
}
 function onTimer()
{
	
}
 function onControl(number, value)
{
	
}
 