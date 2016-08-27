A Qt/OpenGL-based implementation of a heap history visualisation UI in along the
lines of Gerardo Richarte's HeapDraw (for more details, check
http://actes.sstic.org/SSTIC07/Rump_sessions/SSTIC07-rump-Richarte-Heap_Massaging.pdf)

The tool had been reimplemented by various people in various places a few times,
usually in a hackish / nonscalable manner. zynamics used to hand source code for
a JOGL-based UI out with some of our trainings, but that tool is not easily 
portable to modern OpenGL *or* 64-bit address spaces.

Other common mistakes include:

 - Not using OpenGL to render lots of rectangles.
 - Using floats or other low-precision coordinates to represent rectangle
   corners and hence suffering from rounding errors.
 - Not using a language that can deal with a couple million rectangles.

The codebase in this repository has the following goals:

 - Fast and scalable to 20m+ allocations. Haven't tried, but should work.
 - As precise as possible. Use precise integer arithmetic as much as possible
   to avoid rounding errors shifting rectangles around.
 - Useable. This will take some time.

Instructions for the moment:
 - Build using stock QtCreator
 - The current trunk will simply try to load /tmp/heap.json - use the enclosed
   json file as an example.

A million tasks are still left to do. Useful things that should be added:

 - Code to display tooltips when the mouse hovers over a block.
 - Code to highlight a block when it is clicked / selected.
 - Code to add horizontal red lines, too.
 - Code to select all blocks that are modified between two events.
 - ...

Contributions very welcome.

Cheers,
Halvar Flake / Thomas Dullien
