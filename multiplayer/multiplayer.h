#ifndef MULTIPLAYER_PREDEF
#define MULTIPLAYER_PREDEF

#include "classes.h"
#include "scrolls/plugins.h"

#include <boost/asio.hpp>
#include <chrono>
#include <time.h>

using boost::asio::ip::udp;
using boost::asio::ip::address;


class MemBuf : public std::streambuf { public:
	MemBuf(char* data, int max_len);
	int written() const;
	int read() const;
};

struct Packet {
	static const int packetsize = 1024;
	time_t sent;
	Packet();
	Packet(istream& idata);
	virtual void pack(ostream& odata);
};

struct ServerPacket : Packet {
	BASE_PLUGIN_HEAD(ServerPacket, (istream& idata));
	ServerPacket();
	ServerPacket(istream& idata);
	virtual void pack(ostream& odata);
	virtual void run(ClientSocketManager* game, udp::endpoint from) = 0;
};

struct ClientPacket : Packet {
	BASE_PLUGIN_HEAD(ClientPacket, (istream& idata));
	uint32 clientid;
	ClientPacket();
	ClientPacket(istream& idata);
	virtual void pack(ostream& odata);
	virtual void run(ServerSocketManager* game, udp::endpoint from) = 0;
};



class ClientSocketManager { public:
	boost::asio::io_service io_service;
	uint32 id = 9999;
	udp::endpoint server_endpoint;
	udp::socket socket;
	
	ClientSocketManager();
	~ClientSocketManager();
	
	void connect(string ip, int port, string username);
	
	void send(ClientPacket* packet);
	ServerPacket* recieve(udp::endpoint* sender);
	void tick();
};

class ClientEndpoint { public:
	uint32 id;
	string username;
	udp::endpoint endpoint;
};

class ServerSocketManager { public:
	boost::asio::io_service io_service;
	udp::endpoint local_endpoint;
	unordered_map<int,ClientEndpoint> clients;
	uint32 idcounter = 0;
	udp::socket socket;
	
	ServerSocketManager(int port);
	~ServerSocketManager();
	
	void send(ServerPacket* packet, int clientid);
	ClientPacket* recieve(udp::endpoint* sender);
	void tick();
};





#endif
