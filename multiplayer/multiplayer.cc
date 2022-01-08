#ifndef MULTIPLAYER
#define MULTIPLAYER

#include "multiplayer.h"
#include "scrolls/world.h"
#include "scrolls/tiles.h"
#include "scrolls/game.h"
#include "scrolls/entity.h"
#include "scrolls/fileformat.h"

MemBuf::MemBuf(char* data, int max_len) {
	setg(data, data, data+max_len);
	setp(data, data+max_len);
}

int MemBuf::written() const {
	return int(pptr() - pbase());
}

int MemBuf::read() const {
	return int(gptr() - eback());
}


DEFINE_PLUGIN(ServerPacket);
DEFINE_PLUGIN(ClientPacket);

Packet::Packet(): sent(time(NULL)) {
	
}

Packet::Packet(istream& idata): sent(FileFormat::read_variable(idata)) {
	
}

void Packet::pack(ostream& odata) {
	FileFormat::write_variable(odata, sent);
}



ServerPacket::ServerPacket() {
	
}

ServerPacket::ServerPacket(istream& idata): Packet(idata) {
	
}

void ServerPacket::pack(ostream& odata) {
	FileFormat::write_fixed(odata, get_plugin_id().id);
	Packet::pack(odata);
}



ClientPacket::ClientPacket() {
	
}

ClientPacket::ClientPacket(istream& idata): Packet(idata) {
	FileFormat::read_fixed(idata, &clientid);
	cout << "clientpacket from data  id " << clientid << endl;
}

void ClientPacket::pack(ostream& odata) {
	FileFormat::write_fixed(odata, get_plugin_id().id);
	Packet::pack(odata);
	FileFormat::write_fixed(odata, clientid);
}







struct NewIdPacket : ServerPacket {
	PLUGIN_HEAD(NewIdPacket);
	uint32 newclientid;
	
	NewIdPacket(uint32 newid): newclientid(newid) {
		
	}
	
	NewIdPacket(istream& idata): ServerPacket(idata) {
		FileFormat::read_fixed(idata, &newclientid);
	}
		
	virtual void pack(ostream& odata) {
		ServerPacket::pack(odata);
		FileFormat::write_fixed(odata, newclientid);
	}
	
	virtual void run(ClientSocketManager* manager, udp::endpoint from) {
		cout << "Id recieved " << newclientid << endl;
		manager->id = newclientid;
	}
};
EXPORT_PLUGIN(NewIdPacket);

struct JoinPacket : ClientPacket {
	PLUGIN_HEAD(JoinPacket);
	string username;
	
	JoinPacket(string name): username(name) {
		
	}
	
	JoinPacket(istream& idata): ClientPacket(idata), username(FileFormat::read_string(idata)) {
		
	}
	
	virtual void pack(ostream& odata) {
		ClientPacket::pack(odata);
		FileFormat::write_string(odata, username);
	}
	
	virtual void run(ServerSocketManager* manager, udp::endpoint from)  {
		cout << "NEW PLAYER " << username << endl;
		manager->idcounter ++;
		manager->clients[manager->idcounter] = {manager->idcounter, username, from};
		NewIdPacket newidpack (manager->idcounter);
		manager->send(&newidpack, manager->idcounter);
	}
};
EXPORT_PLUGIN(JoinPacket);

struct BlockPacket : ServerPacket {
	PLUGIN_HEAD(BlockPacket);
	Block* block;
	BlockPacket(Block* nblock): block(nblock) {
		
	}
	
	BlockPacket(istream& idata): ServerPacket(idata) {
		int32 px, py, pz, scale;
		
		FileFormat::read_fixed(idata, (uint32*)&px);
		FileFormat::read_fixed(idata, (uint32*)&py);
		FileFormat::read_fixed(idata, (uint32*)&pz);
		FileFormat::read_fixed(idata, (uint32*)&scale);
		
		block = new Block();
		block->set_parent(nullptr, ivec3(px, py, pz), scale);
		block->from_file(idata);
	}
	
	virtual void pack(char* data, int* written) {
		
	}
	
	virtual void run(ClientSocketManager* manager, udp::endpoint from) {
		
	}
};
EXPORT_PLUGIN(BlockPacket);

struct LeavePacket : ClientPacket {
	PLUGIN_HEAD(LeavePacket);
	using ClientPacket::ClientPacket;
	
	virtual void run(ServerSocketManager* manager, udp::endpoint from) {
		cout << "Player leaving " << clientid << endl;
		manager->clients.erase(clientid);
	}
};
EXPORT_PLUGIN(LeavePacket);









ClientSocketManager::ClientSocketManager(): socket(io_service) {
	
}

ClientSocketManager::~ClientSocketManager() {
	cout << id << " id " << endl;
	LeavePacket leavepacket;
	send(&leavepacket);
}

void ClientSocketManager::connect(string ip, int port, string username) {
	server_endpoint = udp::endpoint(address::from_string(ip), port);
	socket.open(udp::v4());
	socket.non_blocking(true);
	JoinPacket joinpack(username);
	send(&joinpack);
}

void ClientSocketManager::send(ClientPacket* packet) {
	cout << "sending packet server " << packet << endl;
	char data[Packet::packetsize];
	MemBuf membuf (data, Packet::packetsize);
	ostream odata (&membuf);
	packet->clientid = id;
	packet->pack(odata);
	socket.send_to(boost::asio::buffer(data, membuf.written()), server_endpoint);
}

ServerPacket* ClientSocketManager::recieve(udp::endpoint* sender) {
	boost::system::error_code ec;
	char data[Packet::packetsize];
	int len = socket.receive_from(boost::asio::buffer(data), *sender, 0, ec);
	if (ec != boost::system::errc::success
		and ec != boost::system::errc::connection_reset
		and ec != boost::system::errc::operation_would_block) {
		cout << ec << endl;
		exit(ec.value());
	}
	
	if (ec == boost::system::errc::operation_would_block) {
		return nullptr;
	} else {
		MemBuf membuf (data, len);
		istream idata (&membuf);
		PluginId id;
		FileFormat::read_fixed(idata, &id.id);
		return ServerPacket::plugnew(id, idata);
	}
}

void ClientSocketManager::tick() {
	ServerPacket* packet;
	udp::endpoint sender;
	while ((packet = recieve(&sender)) != nullptr) {
		packet->run(this, sender);
	}
}
	









ServerSocketManager::ServerSocketManager(int port):
local_endpoint(udp::v4(), port), socket(io_service) {
	socket.open(udp::v4());
	socket.bind(local_endpoint);
	socket.non_blocking(true);
}

ServerSocketManager::~ServerSocketManager() {
	
}

void ServerSocketManager::send(ServerPacket* packet, int clientid) {
	cout << "sending packet " << packet << " to " << clientid << endl;
	char data[Packet::packetsize];
	MemBuf membuf (data, Packet::packetsize);
	ostream odata (&membuf);
	packet->pack(odata);
	socket.send_to(boost::asio::buffer(data, membuf.written()), clients[clientid].endpoint);
}

ClientPacket* ServerSocketManager::recieve(udp::endpoint* sender) {
	boost::system::error_code ec;
	char data[Packet::packetsize];
	int len = socket.receive_from(boost::asio::buffer(data), *sender, 0, ec);
	if (ec != boost::system::errc::success
		and ec != boost::system::errc::connection_reset
		and ec != boost::system::errc::operation_would_block) {
		cout << ec << endl;
		exit(ec.value());
	}
	if (ec == boost::system::errc::operation_would_block) {
		return nullptr;
	} else {
		MemBuf membuf (data, len);
		istream idata (&membuf);
		PluginId id;
		FileFormat::read_fixed(idata, &id.id);
		return ClientPacket::plugnew(id, idata);
	}
}

void ServerSocketManager::tick() {
	ClientPacket* packet;
	udp::endpoint sender;
	while ((packet = recieve(&sender)) != nullptr) {
		packet->run(this, sender);
	}
}


#endif
