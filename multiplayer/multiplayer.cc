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
	setp(data, data, data+max_len);
}

int MemBuf::written() const {
	return int(pptr - pbase);
}

int MemBuf::read() const {
	return int(gptr - gbase);
}



Packet::Packet(): sent(time(NULL)) {
	
}

Packet::Packet(istream& idata): sent(FileFormat::read_variable(idata)) {
	
}

void Packet::pack(ostream& odata) {
	FileFormat::write_fixed(odata, get_plugin_id());
	FileFormat::write_variable(odata, sent);
}



ServerPacket::ServerPacket() {
	
}

ServerPacket::ServerPacket(istream& idata): Packet(idata) {
	
}

void ServerPacket::pack(ostream& odata) {
	Packet::pack(odata);
}



ClientPacket::ClientPacket(uint32 newid) clientid(newid) {
	
}

ClientPacket::ClientPacket(istream& idata): Packet(data, read) {
	FileFormat::read_fixed(idata, &clientid);
	cout << "clientpacket from data  id " << clientid << endl;
}

void ClientPacket::pack(ostream& odata) {
	Packet::pack(odata);
	FileFormat::write_fixed(odata, clientid);
}










struct JoinPacket : ClientPacket {
	PLUGIN_HEAD(JoinPacket);
	string username;
	
	JoinPacket(string name): ClientPacket(9999), username(name) {
		
	}
	
	JoinPacket(istream& idata): ClientPacket(idata), username(FileFormat::read_string(idata)) {
		
	}
	
	virtual void pack(ostream& odata) {
		ClientPacket::pack(odata);
		FileFormat::write_string(idata);
	}
	
	virtual void run(ServerSocketManager* manager, udp::endpoint from)  {
		cout << "NEW PLAYER " << username << endl;
		manager->idcounter ++;
		manager->clients[manager->idcounter] = {manager->idcounter, username, from};
		NewIdPacket newidpack (manager->idcounter);
		manager->send(&newidpack, manager->idcounter);
	}
};

struct NewIdPacket : ServerPacket {
	PLUGIN_HEAD(NewIdPacket);
	uint32 newclientid;
	
	NewIdPacket(int newid, Player* player): newclientid(newid), newplayer(player) {
		
	}
	
	NewIdPacket(istream& idata): ServerPacket(idata) {
		FileFormat::read_fixed(idata, &newclientid);
	}
		
	virtual void pack(ostream& odata) {
		ServerPacket::pack(odata);
		FileFormat::write_fixed(odata, newclientid);
	}
	
	virtual void run(ClientSocketManager* game, udp::endpoint from) {
		cout << "Id recieved " << newclientid << endl;
		manager->id = newclientid;
	}
};

struct BlockPacket : ServerPacket {
	PLUGIN_HEAD(BlockPacket);
	Block* block;
	BlockPacket(Block* nblock): block(nblock) {
		
	}
	
	BlockPacket(istream& idata): ServerPacket(idata) {
		uint32 px, py, pz, scale;
		
		FileFormat::read_fixed(idata, px);
		FileFormat::read_fixed(idata, py);
		FileFormat::read_fixed(idata, pz);
		FileFormat::read_fixed(idata, scale);
		
		block = new Block();
		block->set_parent(nullptr, ivec3(px, py, pz), scale);
		block->from_file(idata);
	}
	
	virtual void pack(char* data, int* written);
	virtual void run(ClientSocketManager* game, udp::endpoint from);
};

struct LeavePacket : ClientPacket {
	PLUGIN_HEAD(LeavePacket);
	using ClientPacket::ClientPacket;
	
	virtual void run(ServerSocketManager* game, udp::endpoint from) {
		cout << "Player leaving " << clientid << endl;
		manager->clients.erase(clientid);
	}
};









ClientSocketManager::ClientSocketManager(string ip, int port, string username):
server_endpoint(address::from_string(ip), port), socket(io_service) {
	socket.open(udp::v4());
	socket.non_blocking(true);
	JoinPacket joinpack(username);
	send(joinpack);
}

ClientSocketManager::~ClientSocketManager() {
	cout << id << " id " << endl;
	LeavePacket leavepacket(id);
	send(leavepacket);
}

void ClientSocketManager::send(ClientPacket* packet) {
	cout << "sending packet server " << packet << endl;
	char data[Packet::packetsize];
	int written = 0;
	packet->pack(data, &written);
	socket.send_to(boost::asio::buffer(data, written), server_endpoint);
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
		PluginId id;
		FileFormat::read_fixed(idata, &id.id);
		return Packet::plugnew(id, idata);
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
	int written = 0;
	packet->pack(data, &written);
	socket.send_to(boost::asio::buffer(data, written), clients[clientid].endpoint);
	delete packet;
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
		return allpackets.get_client_packet(data);
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
