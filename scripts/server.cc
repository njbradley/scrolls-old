
#include "multiplayer.h"
#include "servergame.h"

void server(int port) {
	
	ServerGame game(port);
	
	
	while (game.playing) {
		game.gametick();
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
	
}

int main(int numargs, char** args) {
	if (numargs < 2) {
		cout << "usage: server.exe port" << endl;
		return 0;
	}
	
	int port = atoi(args[1]);
	cout << "starting server on port " << port << endl;
	server(port);
	cout << "competed without errors" << endl;
	return 0;
}
