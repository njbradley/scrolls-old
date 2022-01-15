#ifndef MULTIPLAYER
#define MULTIPLAYER

#include "multiplayer.h"
#include "scrolls/world.h"
#include "scrolls/tiles.h"
#include "scrolls/game.h"
#include "scrolls/entity.h"
#include "scrolls/fileformat.h"
#include "scrolls/player.h"

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





DEFINE_PLUGIN(Packet);

Packet::Packet(): sent(time(NULL)), clientid(0) {
	
}

Packet::Packet(istream& idata): sent(FileFormat::read_variable(idata)) {
	FileFormat::read_fixed(idata, &clientid);
}

void Packet::pack(ostream& odata) {
	FileFormat::write_fixed(odata, get_plugin_id().id);
	FileFormat::write_variable(odata, sent);
	FileFormat::write_fixed(odata, clientid);
}



JoinPacket::JoinPacket(istream& idata): Packet(idata), username(FileFormat::read_string(idata)) {
	
}

void JoinPacket::pack(ostream& odata) {
	Packet::pack(odata);
	FileFormat::write_string(odata, username);
}
DEFINE_PACKET(JoinPacket);


BlockPacket::BlockPacket(Block* nblock): block(nblock) {
	
}

BlockPacket::BlockPacket(istream& idata): Packet(idata) {
	int32 px, py, pz, scale;
	
	FileFormat::read_fixed(idata, (uint32*)&px);
	FileFormat::read_fixed(idata, (uint32*)&py);
	FileFormat::read_fixed(idata, (uint32*)&pz);
	FileFormat::read_fixed(idata, (uint32*)&scale);
	
	cout << "got block " << px << ' ' << py << ' ' << pz << ' ' << scale << endl;
	// block = new Block();
	// block->set_parent(nullptr, ivec3(px, py, pz), scale);
	// block->from_file(idata);
}

void BlockPacket::pack(ostream& odata) {
	Packet::pack(odata);
	FileFormat::write_fixed(odata, (uint32) block->parentpos.x);
	FileFormat::write_fixed(odata, (uint32) block->parentpos.y);
	FileFormat::write_fixed(odata, (uint32) block->parentpos.z);
	FileFormat::write_fixed(odata, (uint32) block->scale);
}
DEFINE_PACKET(BlockPacket);


DEFINE_PACKET(NewIdPacket);
DEFINE_PACKET(LeavePacket);


ControlsPacket::ControlsPacket(Controls* controls) {
	for (int i = 0; i < 3; i ++) {
		mouse_buttons[i] = controls->mouse_button(i+1);
	}
	modifiers[0] = controls->key_pressed(controls->KEY_SHIFT);
	modifiers[1] = controls->key_pressed(controls->KEY_CTRL);
	modifiers[2] = controls->key_pressed(controls->KEY_ALT);
	modifiers[3] = controls->key_pressed(controls->KEY_TAB);
	for (char let = ' '; let <= ']'; let ++) {
		if (controls->key_pressed(let)) {
			keys_pressed.push_back(let);
		}
	}
}

ControlsPacket::ControlsPacket(istream& idata): Packet(idata) {
	for (int i = 0; i < 3; i ++) {
		mouse_buttons[i] = idata.get();
	}
	for (int i = 0; i < 4; i ++) {
		modifiers[i] = idata.get();
	}
	keys_pressed = FileFormat::read_string(idata);
}

void ControlsPacket::pack(ostream& odata) {
	Packet::pack(odata);
	for (int i = 0; i < 3; i ++) {
		odata.put(mouse_buttons[i]);
	}
	for (int i = 0; i < 4; i ++) {
		odata.put(modifiers[i]);
	}
	FileFormat::write_string(odata, keys_pressed);
}
DEFINE_PACKET(ControlsPacket);




struct JoinExecutor : ServerExecutor<JoinPacket> {
	PLUGIN_HEAD(JoinExecutor);
	virtual void run(JoinPacket* pack, ServerSocketManager* manager) {
		cout << " executing join packet " << endl;
		manager->clients[pack->clientid].username = pack->username;
		NewIdPacket idpack;
		manager->send(&idpack, pack->clientid);
	}
};
EXPORT_PLUGIN(JoinExecutor);

struct LeaveExecutor : ServerExecutor<LeavePacket> {
	PLUGIN_HEAD(JoinExecutor);
	virtual void run(LeavePacket* pack, ServerSocketManager* manager) {
		cout << " executing leave packet " << endl;
		manager->clients.erase(pack->clientid);
	}
};
EXPORT_PLUGIN(LeaveExecutor);













ClientSocketManager::ClientSocketManager(): socket(io_service) {
	
}

ClientSocketManager::~ClientSocketManager() {
	cout << clientid << " id " << endl;
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

void ClientSocketManager::send(Packet* packet) {
	cout << "sending packet server " << packet << endl;
	char data[Packet::packetsize];
	MemBuf membuf (data, Packet::packetsize);
	ostream odata (&membuf);
	packet->clientid = clientid;
	packet->pack(odata);
	socket.send_to(boost::asio::buffer(data, membuf.written()), server_endpoint);
}

Packet* ClientSocketManager::recieve() {
	udp::endpoint sender;
	boost::system::error_code ec;
	char data[Packet::packetsize];
	int len = socket.receive_from(boost::asio::buffer(data), sender, 0, ec);
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
		Packet* packet = Packet::plugnew(id, idata);
		if (clientid == 0) {
			clientid = packet->clientid;
			cout << " got client id " << clientid << endl;
		}
		cout << " got packet " << packet << ' ' << packet->clientid << endl;
		return packet;
	}
}

void ClientSocketManager::tick() {
	Packet* packet;
	while ((packet = recieve()) != nullptr) {
		packet->run_client(this);
		delete packet;
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

void ServerSocketManager::send(Packet* packet, uint32 clientid) {
	cout << "sending packet " << packet << " to " << clientid << endl;
	char data[Packet::packetsize];
	MemBuf membuf (data, Packet::packetsize);
	ostream odata (&membuf);
	packet->clientid = clientid;
	packet->pack(odata);
	socket.send_to(boost::asio::buffer(data, membuf.written()), clients[clientid].endpoint);
}

Packet* ServerSocketManager::recieve() {
	udp::endpoint sender;
	boost::system::error_code ec;
	char data[Packet::packetsize];
	int len = socket.receive_from(boost::asio::buffer(data), sender, 0, ec);
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
		Packet* packet = Packet::plugnew(id, idata);
		if (packet->clientid == 0) {
			for (const pair<uint32,ClientEndpoint>& kvpair : clients) {
				if (kvpair.second.endpoint == sender) {
					packet->clientid = kvpair.first;
					break;
				}
			}
			if (packet->clientid == 0) {
				idcounter ++;
				clients[idcounter] = {idcounter, "", sender};
				packet->clientid = idcounter;
				cout << " added client " << idcounter << ' ' << sender << endl;
			}
		}
		cout << " recieved packet " << packet << ' ' << id.name() << ' ' << endl;
		return packet;
	}
}

void ServerSocketManager::tick() {
	Packet* packet;
	while ((packet = recieve()) != nullptr) {
		packet->run_server(this);
		delete packet;
	}
}

void ServerSocketManager::send_tile(uint32 id, Tile* tile) {
	BlockPacket blockpack(tile->chunk);
	send(&blockpack, id);
}

void ServerSocketManager::update_block(uint32 id, Block* block) {
	if (block != nullptr) {
		if (block->flags & Block::CHANGE_FLAG) {
			BlockPacket blockpack(block);
			send(&blockpack, id);
			block->reset_all_flags(Block::CHANGE_FLAG | Block::CHANGE_PROP_FLAG);
		} else if ((block->flags & Block::CHANGE_PROP_FLAG) and block->continues) {
			for (int i = 0; i < csize3; i ++) {
				update_block(id, block->children[i]);
			}
			block->reset_flag(Block::CHANGE_PROP_FLAG);
		}
	}
}
					

#endif
