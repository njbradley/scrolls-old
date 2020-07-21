#ifndef MULTIPLAYER_PREDEF
#define MULTIPLAYER_PREDEF

#include <boost/asio.hpp>

using boost::asio::ip::udp;
using boost::asio::ip::address;

class 

class SocketManager { public:
	boost::asio::io_service io_service;
	udp::socket socket;
	
	
}

#endif
