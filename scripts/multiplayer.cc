#ifndef MULTIPLAYER
#define MULTIPLAYER

#include "multiplayer.h"

Packet::Packet(char newtype): type(newtype), sent(time(NULL)) {
	
}

Packet::Packet(char* data, int* read): type(getval<char>(data, read)), sent(getval<int>(data, read)) {
	
}

void Packet::pack(char* data, int* written) {
	packval<char>(data, written, type);
	packval<int>(data, written, sent);
}

template <typename T>
void Packet::packval(char* data, int* written, T val) {
	*((T*)(data+*written)) = val;
	*written += sizeof(T);
}

template <>
void Packet::packval<string>(char* data, int* written, string val) {
	for (char c : val) {
		((char*)data)[*written] = c;
		*written += 1;
	}
	((char*)data)[*written] = 0;
	*written += 1;
}

template <typename T>
T Packet::getval(char* data, int* read) {
	T val = *((T*)(data+*read));
	*read += sizeof(T);
	return val;
}

template <>
string Packet::getval<string>(char* data, int* read) {
	char* cdata = (char*)data;
	string result;
	while (cdata[*read] != 0) {
		result.push_back(cdata[*read]);
		*read += 1;
	}
	return result;
}

ServerPacket::ServerPacket(char type): Packet(type) {
	
}

ServerPacket::ServerPacket(char* data, int* read): Packet(data, read) {
	
}

void ServerPacket::pack(char* data, int* written) {
	Packet::pack(data, written);
}



ClientPacket::ClientPacket(char type, int newid): Packet(type), clientid(newid) {
	
}

ClientPacket::ClientPacket(char* data, int* read): Packet(data, read), clientid(getval<int>(data, read)) {
	cout << "clientpacket from data  id " << clientid << endl;
}

void ClientPacket::pack(char* data, int* written) {
	Packet::pack(data, written);
	packval<int>(data, written, clientid);
}


NewIdPacket::NewIdPacket(int newid): ServerPacket('n'), newclientid(newid) {
	cout << "clientid " << newid << endl;
}

NewIdPacket::NewIdPacket(char* data, int* read): ServerPacket(data, read), newclientid(getval<int>(data, read)) {
	cout << "clientid " << newclientid << endl;
}

void NewIdPacket::pack(char* data, int* written) {
	ServerPacket::pack(data, written);
	packval<int>(data, written, newclientid);
}

void NewIdPacket::run(ClientSocketManager* manager, udp::endpoint from) {
	cout << "Id recieved " << newclientid << endl;
	manager->id = newclientid;
}




JoinPacket::JoinPacket(string name): ClientPacket('J', 9999), username(name) {
	
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
