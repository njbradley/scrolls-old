#ifndef MULTIPLAYER_PREDEF
#define MULTIPLAYER_PREDEF

#include "classes.h"

#include <boost/asio.hpp>
#include <chrono>
#include <time.h>

using boost::asio::ip::udp;
using boost::asio::ip::address;

struct Packet {
	static const int packetsize = 1024;
	char type;
	time_t sent;
	Packet(char newtype);
	Packet(char* data, int* read);
	virtual void pack(char* data, int* written);
	
	template <typename T>
	static void packval(char* data, int* written, T val);
	template <typename T>
	static T getval(char* data, int* written);
};

struct ServerPacket : Packet {
	ServerPacket(char type);
	ServerPacket(char* data, int* read);
	virtual void pack(char* data, int* written);
	virtual void run(ClientSocketManager* game, udp::endpoint from) = 0;
};

struct ClientPacket : Packet {
	int clientid;
	ClientPacket(char type, int newid);
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
	NewIdPacket(int newid);
	NewIdPacket(char* data, int* read);
	virtual void pack(char* data, int* written);
	virtual void run(ClientSocketManager* game, udp::endpoint from);
};

struct LeavePacket : ClientPacket {
	LeavePacket(int clientid);
	LeavePacket(char* data, int* read);
	virtual void pack(char* data, int* written);
	virtual void run(ServerSocketManager* game, udp::endpoint from);
};

class PacketGroup { public:
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
