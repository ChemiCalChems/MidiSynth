#include <iostream>
#include <queue>
#include <thread>

/* Reading MIDI bytes from keyboard */
#include <wiringPi.h>
#include <wiringSerial.h>

/* TCP server and related includes*/
#include <boost/asio.hpp>


void accept_connections(boost::asio::io_service& io_service, boost::asio::ip::tcp::acceptor& acceptor, std::vector<boost::asio::ip::tcp::socket>& client_sockets) {
	boost::asio::ip::tcp::socket* socket = new boost::asio::ip::tcp::socket(io_service);
	std::cout << socket << std::endl;
	acceptor.async_accept(*socket, [socket, &client_sockets, &io_service, &acceptor] (boost::system::error_code ec)
		{
			if (!ec) {client_sockets.push_back(std::move(*socket)); std::cout << "connection established" << std::endl;}
			accept_connections(io_service, acceptor, client_sockets);
		});
}


void read_midi(std::vector<boost::asio::ip::tcp::socket>* client_sockets) {
	if(wiringPiSetup() == -1) std::cout << "Setting up wiringPi failed" << std::endl;
	
	int serial_fd;
	if((serial_fd = serialOpen("/dev/ttyAMA0", 38400)) < 0) {
		std::cerr << "Couldn't open serial connection" << std::endl;
	}
	
	std::vector<unsigned char> bytes_read;
	
	while(true) {
		while(serialDataAvail(serial_fd)) {
			auto byte = serialGetchar(serial_fd);
			if (byte != 254) bytes_read.push_back(byte);
		}
		
		
		for (auto&& client : *client_sockets) {
			try {
				client.send(boost::asio::buffer(bytes_read));	
			} catch (...) {};
		}
		bytes_read.clear();
	}
}


int main() {
	/* Set up TCP server */
	boost::asio::io_service io_serv;
	std::vector<boost::asio::ip::tcp::socket> client_sockets;
	boost::asio::ip::tcp::acceptor client_acceptor {io_serv};
	
	
	boost::asio::ip::tcp::endpoint endpoint {boost::asio::ip::tcp::v4(), 5001};
	client_acceptor.open(endpoint.protocol());
	client_acceptor.bind(endpoint);
	client_acceptor.listen();
	
	accept_connections(io_serv, client_acceptor, client_sockets);

	/* Start midi reading thread */
	std::thread midi_thread{read_midi, &client_sockets};
	
	io_serv.run();
	
}
