
#include "multiplayer.h"
#include "servergame.h"
#include "cross-platform.h"

void server(int port) {
	
	game = new ServerGame(port);
	
	
	while (game->playing) {
		game->gametick();
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
	}
}

int main(int numargs, char** args) {
	if (numargs < 2) {
		cout << "usage: server.exe port" << endl;
		return 0;
	}
	
	setup_backtrace();
	
	int port = atoi(args[1]);
	cout << "starting server on port " << port << endl;
	server(port);
	cout << "competed without errors" << endl;
	return 0;
}
