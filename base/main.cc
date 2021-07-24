#include "game.h"
#include "cross-platform.h"
#include "debug.h"


int main( void )
{
	logger->log(5) << "Starting game! " << endl;
	
	for (int i = 0; i < 100000000; i ++) {
		
	}
	
	debugger->setup_backtrace();
	return 0;
	game->setup_gameloop();
	while (game->playing) {
		game->gametick();
	}
	
	logger->log(3) << " ending game" << endl;
	return 0;
}


/*
All other scripts in this folder are considered part of the
game and are under this same license

Copyright 2020 Nicholas Bradley. All rights reserved.

I grant the right to download and modify the game for your own personal use.
Please do not distribute or sell original or modified copies of this game without express permission.

This software is provided 'as is' with no expressed or implied warranties.
*/
