
#include "multiplayer.h"
#include "clientgame.h"

void client(string destip, int destport, string username) {
	ClientGame game(destip, destport, username);
	int i = 0;
	while (game.playing) {
		cout << i << endl;
		game.gametick();
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		i++;
		if (i > 15) {
			game.playing = false;
		}
	}
}

int main(int numargs, char** args) {
	if (numargs < 4) {
		cout << "usage: client.exe ip port username" << endl;
		return 0;
	}
	
	int destport = atoi(args[2]);
	string destip = args[1];
	string username = args[3];
	
	cout << "starting client to ip " << destip << " port " << destport << " username " << username << endl;
	client(destip, destport, username);
	
	cout << "completed sucessfully " << endl;
	return 0;
}
