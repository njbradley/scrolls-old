#ifndef MULTIPLAYER_PREDEF
#define MULTIPLAYER_PREDEF

#include "classes.h"

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
	BASE_PLUGIN_HEAD(Packet, (istream& idata));
	static const int packetsize = 1024;
	time_t sent;
	Packet();
	Packet(istream& idata);
	virtual void pack(ostream& odata);
};

struct ServerPacket : Packet {
	ServerPacket(string name);
	ServerPacket(char* data, int* read);
	virtual void pack(char* data, int* written);
	virtual void run(ClientSocketManager* game, udp::endpoint from) = 0;
};

struct ClientPacket : Packet {
	int clientid;
	ClientPacket(string name, int newid);
	ClientPacket(char* data, int* read);
	virtual void pack(char* data, int* written);
	virtual void run(ServerSocketManager* game, udp::endpoint from) = 0;
};

struct JoinPacket : ClientPacket {
	string username;
	JoinPacket(string name);
	JoinPacket(char* data, int* read);
	virtual void pack(char* data, int* written);
	virtual void run(ServerSocketManager* game, udp::endpoint from);
};

struct NewIdPacket : ServerPacket {
	int newclientid;
	World* newworld;
	Player* newplayer;
	NewIdPacket(int newid);
	NewIdPacket(char* data, int* read);
	virtual void pack(char* data, int* written);
	virtual void run(ClientSocketManager* game, udp::endpoint from);
};

struct BlockPacket : ServerPacket {
	Tile* tile;
	BlockPacket(Tile* newtile);
	BlockPacket(char* data, int* read);
	virtual void pack(char* data, int* written);
	virtual void run(ClientSocketManager* game, udp::endpoint from);
};

struct LeavePacket : ClientPacket {
	LeavePacket(int clientid);
	LeavePacket(char* data, int* read);
	virtual void pack(char* data, int* written);
	virtual void run(ServerSocketManager* game, udp::endpoint from);
};

class PacketGroup : Storage<Packet> { public:
	PacketGroup();
	ServerPacket* get_server_packet(char* data);
	ClientPacket* get_client_packet(char* data);
};

class ClientSocketManager { public:
	boost::asio::io_service io_service;
	int id = 9999;
	udp::endpoint server_endpoint;
	PacketGroup allpackets;
	udp::socket socket;
	
	ClientSocketManager(string ip, int port, string username);
	~ClientSocketManager();
	
	void send(ClientPacket* packet);
	ServerPacket* recieve(udp::endpoint* sender);
	void tick();
};

class ClientEndpoint { public:
	int id;
	string username;
	udp::endpoint endpoint;
};

class ServerSocketManager { public:
	boost::asio::io_service io_service;
	udp::endpoint local_endpoint;
	unordered_map<int,ClientEndpoint> clients;
	int idcounter = 10;
	PacketGroup allpackets;
	udp::socket socket;
	
	ServerSocketManager(int port);
	~ServerSocketManager();
	
	void send(ServerPacket* packet, int clientid);
	ClientPacket* recieve(udp::endpoint* sender);
	void tick();
};





#endif
