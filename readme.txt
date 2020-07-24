This is version 0.3 (in dev) of scrolls alpha: the adventure game!

Getting started:

Precompiled distributions:
Right now, there is a windows exe release if you want to play the game without setting up the compilation environment.
Just download the zip file and skip to the section Playing the game.

Compiling:
To compile scrolls, you will need a c++14 compatable compiler, and the libraries: glfw, glew, glm, stb_image, boost
On windows, most of these libraries can be downloaded in precomplied states, and only glfw, glew and boost need libraries,
the rest are header only. I use Mingw for compiling, which works well.
On mac and linux, the libraries can be installed with atp-get or brew.
Before you compile the scripts, open the file scripts/cross-platform.h. If you are on windows, comment out the unix include statement
and leave the win.h include. If you are on mac or linux, do the opposite.
The compile command is rather simple, you just compile main.cc into main.exe.
My windows command is:
g++ main.cc -o main.exe -O3 -std=c++14 -lglew32s -lmingw32 -lglfw3 -lopengl32 -luser32 -lgdi32 -lshell32 -lWs2_32 -lboost_system-mt-x64 -DGLEW_STATIC
Thats it! now you should have an executable! However, it has to be kept with the resources folder wherever you run it.

Playing the game:
To play the game, double click on main.exe. A window should pop up with a new world open.
The controls are listed below, so walk around and explore! I am working on a tutorial, but for now, there is not
much guidance for what to do. A few pointers:
 -You can create a crafting table by collecting crafting table blocks (normally in the terrain structures)
  and place them in a square. A 2x2 crafting table is level 2, a 3x3 table is level 3, and so on. Each level unlocks more
  and more recipes.
 -Skeletons spawn in the snow, so watch out for them sneaking up on you, they blend right in
 -Pigs are a great source of food, they spawn in the grassy areas
 
Controls:
WASD - movement
Mouse - look around
Scroll - change holding item
Number Keys - select holding item
Left click - destroy item (takes time)
Right click - place item or interact with special blocks
E - inventory
C - hand crafting
M - main menu
Ctr-Q - quit

Debug:
O/P - show/hide debug menu
R - force cleaning of memory vectors
F/G - enter/exit spectator mode

Version 0.3 (in dev):
 Additions:
 -many new randomly generated terrain structures
 -block physics is now working! now blocks fall if they are not attached to another block they stick to
 -monsters have been added! When they die they become part of the landscape, instead of just dropping their loot
 -player movement has been improved, you can now go up one block steps automatically without losing your velocity
 -you can also construct ladders out of blocks that the player can climb, and the player can stand in gaps in the wall
 -finally fixed most of the bugs and lag from multithreading
 -working on multiplayer worlds and servers

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
