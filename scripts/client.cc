#include "multiplayer.h"
#include "clientgame.h"
#include "cross-platform.h"

void client(string destip, int destport, string username) {
	game = new ClientGame(destip, destport, username);
	
	game->setup_gameloop();
	while (game->playing) {
		cout << "loop " << endl;
		game->gametick();
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
	}
	
	delete game;
}

int main(int numargs, char** args) {
	if (numargs < 4) {
		cout << "usage: client.exe ip port username" << endl;
		return 0;
	}
	
	setup_backtrace();
	
	int destport = atoi(args[2]);
	string destip = args[1];
	string username = args[3];
	
	cout << "starting client to ip " << destip << " port " << destport << " username " << username << endl;
	client(destip, destport, username);
	
	cout << "completed sucessfully " << endl;
	return 0;
}
