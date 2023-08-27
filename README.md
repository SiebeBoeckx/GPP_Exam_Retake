# ZombieGame and GOAP

This is both my zombie game and my research project

I used a behaviour tree for the AI of my zombie game, but I replaced all the conditions using GOAP.

If I were to do this again, I would use a finite state machine as my base. This is how GOAP is supposed to be used but I had already started with a behaviour tree.


-----Implementation-------

Every update cycle, before the tree gets updated, a WorldState class gets updated.

This class houses a struct with all the booleans the character needs.
In the Update cycle, all booleans get checked and updated using the data it can extract from the world.
Afterwards, the WorldState gets updated.
Finally in the behaviour tree, through the use of a different implementation of the BehaviourConditional and BehaviourAction, the WorldState gets used as conditions and can be passed through to the actions that can change the state.

If I had more time, I would implement the behaviour where after an action, there's another check if the new state is actually the state it was expecting after the action. If this was not the case, it would try another action to reach the desired state.
Right now it's just another way of defining the conditionals using the WorldState instead of the Blackboard.
This would also be easier to implement if I had used a finite state machine where the states could've have been goals to be reached (and checked).

## Acknowledgements

 https://www.youtube.com/watch?v=PaOLBOuyswI
 Three States and a Plan: The A.I. of F.E.A.R.
	Jeff Orkin
	Monolith Productions / M.I.T. Media Lab, Cognitive Machines Group
	http://www.jorkin.com

https://alumni.media.mit.edu/~jorkin/gdc2006_orkin_jeff_fear.pdf


 ## Demo

Insert gif or link to demo	
