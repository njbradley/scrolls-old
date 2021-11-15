This is version 0.4 (in dev) of scrolls alpha: the adventure game!

The physics engine qu3e (https://github.com/RandyGaul/qu3e/) is used for the physics in this game. Many thanks to the developers!

Getting started:

Precompiled distributions:
Right now, there is a windows exe release if you want to play the game without setting up the compilation environment.
Just download the zip file and skip to the section Playing the game.

Compiling:
To compile scrolls, you will need a c++14 compatable compiler, and the libraries: glfw, glew, glm, stb_image, boost
On windows, most of these libraries can be downloaded in precomplied states, and only glfw, glew and boost need libraries,
the rest are header only. I use Mingw for compiling, which works well.
On windows, msys2 can be used to install the libraries:
	mingw-w64-x86_64-gcc
	mingw-w64-x86_64-make
	mingw-w64-x86_64-freealut
	mingw-w64-x86_64-openal
	mingw-w64-x86_64-glm
	mingw-w64-x86_64-glfw
	mingw-w64-x86_64-glew
	mingw-w64-x86_64-boost
	mingw-w64-x86_64-dwarfstack
On linux, apt can be used:
	libglfw3-dev
	libglew-dev
	libalut-dev
	libglm-dev
	libboost-all-dev
On mac and linux, the libraries can be installed with atp-get or brew.
The compile command is rather simple with make, run the command "make" in the
root directory. Specify the platform by adding PLAT=MAC or PLAT=LINUX.
on windows with mingw the command is "mingw32-make" but otherwise
it is the same.
Thats it! now you should have an executable!

Playing the game:
To play the game, double click on scrolls.exe. A window should pop up with a new world open.
The controls are listed below, so walk around and explore! Right now there are not many game features, I have
mostly been working on efficient block physics.
 
Controls:
WASD - movement
Mouse - look around
Scroll - change holding item
Number Keys - select holding item
Left click - destroy block
Right click - place block, or grab block if selected item is nothing
Shift-Right click - place free block, that follows physics
Ctr-Q - quit

Debug:
F/G - enter/exit spectator mode
B - Pause physics simulation (will pause player movement too)
M - Resume physics simulation
N - Step forward one physics tick

Version 0.4:
 Additions:
 -Added "Free blocks" which are blocks that are not on the grid, either rotated or moved over.
 -Used the physics engine qu3e to add real world rigid body physics for the new freeblocks.
 -Reorganized the scripts into separate plugins, for different parts of the game, like graphics, audio, game, world.
 -

Version 0.3:
 Additions:
 -many new randomly generated terrain structures
 -block physics is now working! now blocks fall if they are not attached to another block they stick to
 -monsters have been added! When they die they become part of the landscape, and you can mine their bodies to get loot
 -player movement has been improved, you can now go up one block steps automatically without losing your velocity
 -you can also construct ladders out of blocks that the player can climb, and the player can stand in gaps in the wall
 -biomes have been added, right now there are only two but soon there will be much more
 -lighting is finally working! there is a day/night cycle, and at night you need lamps to be able to see
 -tools have been overhauled, with a new durability system. every tool has a weight level and a sharpness level
  when you mine, the sharpness decreases. you can craft a grindstone to sharpen the tool, but that wears away
  and decreases the weight
 -chests have been changed, now every block holds one item, but if you place chests next to each other they combine up to 10 spots

Version 0.2:
 Additions:
 -full multithreading of loading and saving chunks, so no lag while playing
 -overhaul of terrain generation, much more expandable for future updates
 -world is now infinite in all directions with no build limits
 -worlds are encapsulated in a zip folder for efficient storage
 -settings file for adjustment
 -more blocks and items
 -trees!
 -lots of bugs from the multithreading

Version 0.1:
 Right now the game is barely in a playable state, only the base features are in place.
