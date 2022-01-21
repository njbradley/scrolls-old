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



template <typename PacketT>
struct ClientExecutor {
	BASE_PLUGIN_HEAD(ClientExecutor<PacketT>, ());
	virtual void run(PacketT* pack, ClientSocketManager* manager) {};
	virtual void run(PacketT* pack) {};
	
	static void run_all(ClientSocketManager* manager, PacketT* packet) {
		PluginListNow<ClientExecutor<PacketT>> all_execs;
		for (ClientExecutor<PacketT>* exec : all_execs) {
			exec->run(packet, manager); exec->run(packet);
		}
	}
};

template <typename PacketT>
struct ServerExecutor {
	BASE_PLUGIN_HEAD(ServerExecutor<PacketT>, ());
	virtual void run(PacketT* pack, ServerSocketManager* manager) {};
	virtual void run(PacketT* pack) {};
	
	static void run_all(ServerSocketManager* manager, PacketT* packet) {
		PluginListNow<ServerExecutor<PacketT>> all_execs;
		for (ServerExecutor<PacketT>* exec : all_execs) {
			exec->run(packet, manager); exec->run(packet);
		}
	}
};

#define EXEC_METHODS(X) \
	virtual void run_client(ClientSocketManager* manager) { ClientExecutor<X>::run_all(manager, this); } \
	virtual void run_server(ServerSocketManager* manager) { ServerExecutor<X>::run_all(manager, this); }

#define PACKET_HEAD(X) \
	PLUGIN_HEAD(X); \
	EXEC_METHODS(X);

#define DEFINE_PACKET(X) \
	EXPORT_PLUGIN(X); \
	DEFINE_PLUGIN_TEMPLATE(ServerExecutor<X>); \
	DEFINE_PLUGIN_TEMPLATE(ClientExecutor<X>);

struct Packet {
	BASE_PLUGIN_HEAD(Packet, (istream& idata));
	static const int packetsize = 1024;
	time_t sent;
	uint32 clientid;
	Packet();
	Packet(istream& idata);
	virtual void pack(ostream& odata);
	virtual void run_client(ClientSocketManager* manager) = 0;
	virtual void run_server(ServerSocketManager* manager) = 0;
};

struct JoinPacket : Packet {
	PACKET_HEAD(JoinPacket);
	string username;
	JoinPacket(string name): username(name) {}
	JoinPacket(istream& idata);
	virtual void pack(ostream& odata);
};

struct NewIdPacket : Packet {
	PACKET_HEAD(NewIdPacket);
	using Packet::Packet;
};

struct LeavePacket : Packet {
	PACKET_HEAD(LeavePacket);
	using Packet::Packet;
};

struct BlockPacket : Packet {
	PACKET_HEAD(BlockPacket);
	ivec3 globalpos;
	int scale;
	Block* block;
	BlockPacket(Block* block);
	BlockPacket(istream& idata);
	virtual void pack(ostream& odata);
};

struct ControlsPacket : Packet {
	PACKET_HEAD(ControlsPacket);
	bool mouse_buttons[3];
	bool modifiers[4];
	string keys_pressed;
	ControlsPacket(Controls* controls);
	ControlsPacket(istream& idata);
	virtual void pack(ostream& odata);
};



class ClientSocketManager { public:
	boost::asio::io_service io_service;
	uint32 clientid = 0;
	udp::endpoint server_endpoint;
	udp::socket socket;
	
	ClientSocketManager();
	~ClientSocketManager();
	
	void connect(string ip, int port, string username);
	
	void send(Packet* packet);
	Packet* recieve();
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
	unordered_map<uint32,ClientEndpoint> clients;
	uint32 idcounter = 1;
	udp::socket socket;
	
	ServerSocketManager(int port);
	~ServerSocketManager();
	
	void send(Packet* packet, uint32 clientid);
	Packet* recieve();
	void tick();
};





#endif
