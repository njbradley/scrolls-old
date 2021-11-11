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
	FileFormat::write_variable(sent);
}



ServerPacket::ServerPacket() {
	
}

ServerPacket::ServerPacket(istream& idata): Packet(idata) {
	
}

void ServerPacket::pack(ostream& odata) {
	Packet::pack(odata);
}



ClientPacket::ClientPacket( newid): Packet(name), clientid(newid) {
	
}

ClientPacket::ClientPacket(char* data, int* read): Packet(data, read), clientid(getval<int>(data, read)) {
	cout << "clientpacket from data  id " << clientid << endl;
}

void ClientPacket::pack(char* data, int* written) {
	Packet::pack(data, written);
	packval<int>(data, written, clientid);
}


NewIdPacket::NewIdPacket(int newid): ServerPacket('n'),
newclientid(newid) {
	cout << "clientid " << newid << endl;
}

NewIdPacket::NewIdPacket(char* data, int* read): ServerPacket(data, read), newclientid(getval<int>(data, read)) {
	cout << "clientid " << newclientid << endl;
	// BufIn buf(data, read);
	// istream ifile(&buf);
	// newworld = new World("client", ifile);
	// newplayer = new Player(newworld, ifile);
}

void NewIdPacket::pack(char* data, int* written) {
	ServerPacket::pack(data, written);
	packval<int>(data, written, newclientid);
	// BufOut buf(data, written);
	// ostream ofile(&buf);
	// newworld->save_data_file(ofile);
	// newplayer->save_to_file(ofile);
}

void NewIdPacket::run(ClientSocketManager* manager, udp::endpoint from) {
	cout << "Id recieved " << newclientid << endl;
	manager->id = newclientid;
	// world = newworld;
	// world->player = newplayer;
}




JoinPacket::JoinPacket(string name): ClientPacket("JoinPacket", 9999), username(name) {
	
}

JoinPacket::JoinPacket(char* data, int* read): ClientPacket(data, read), username(getval<string>(data, read)) {
	
}

void JoinPacket::pack(char* data, int* written) {
	ClientPacket::pack(data, written);
	packval<string>(data, written, username);
}

void JoinPacket::run(ServerSocketManager* manager, udp::endpoint from) {
	cout << "NEW PLAYER " << username << endl;
	manager->idcounter ++;
	manager->clients[manager->idcounter] = {manager->idcounter, username, from};
	manager->send(new NewIdPacket(manager->idcounter), manager->idcounter);
}


BlockPacket::BlockPacket(Tile* ntile): ServerPacket("BlockPacket"), tile(ntile) {
	
}

BlockPacket::BlockPacket(char* data, int* read): ServerPacket(data, read) {
	ivec3 pos = getval<ivec3>(data, read);
	BufIn buf(data, read);
	istream ifile(&buf);
	//tile = new Tile(pos, world, ifile);
}

void BlockPacket::pack(char* data, int* written) {
	ServerPacket::pack(data, written);
	packval<ivec3>(data, written, tile->pos);
	BufOut buf(data, written);
	ostream ofile(&buf);
	//tile->save_to_file(ofile);
}

void BlockPacket::run(ClientSocketManager* manager, udp::endpoint from) {
	world->add_tile(tile);
}




LeavePacket::LeavePacket(int clientid): ClientPacket('L', clientid) {
	
}

LeavePacket::LeavePacket(char* data, int* read): ClientPacket(data, read) {
	
}

void LeavePacket::pack(char* data, int* written) {
	ClientPacket::pack(data, written);
}

void LeavePacket::run(ServerSocketManager* manager, udp::endpoint from) {
	cout << "Player leaving " << clientid << endl;
	manager->clients.erase(clientid);
}


ServerPacket* PacketGroup::get_server_packet(char* data) {
	int written = 0;
	Packet packet(data, &written);
	written = 0;
	if (packet.type == 'n') return new NewIdPacket(data, &written);
	return nullptr;
}


ClientPacket* PacketGroup::get_client_packet(char* data) {
	int written = 0;
	Packet packet(data, &written);
	written = 0;
	if (packet.type == 'J') return new JoinPacket(data, &written);
	if (packet.type == 'L') return new LeavePacket(data, &written);
	return nullptr;
}



ClientSocketManager::ClientSocketManager(string ip, int port, string username):
server_endpoint(address::from_string(ip), port), socket(io_service) {
	socket.open(udp::v4());
	socket.non_blocking(true);
	send(new JoinPacket(username));
}

ClientSocketManager::~ClientSocketManager() {
	cout << id << " id " << endl;
	send(new LeavePacket(id));
}

void ClientSocketManager::send(ClientPacket* packet) {
	cout << "sending packet server " << packet << endl;
	char data[Packet::packetsize];
	int written = 0;
	packet->pack(data, &written);
	socket.send_to(boost::asio::buffer(data, written), server_endpoint);
	delete packet;
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
		return allpackets.get_server_packet(data);
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
