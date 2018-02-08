#include <boost/asio.hpp>
#include <queue>
#include <array>
#include <iostream>
#include <thread>

std::array<unsigned char, 128> midi_note_current_velocities;

unsigned char nibble(unsigned char byte, bool first = true) {
	return first ? byte >> 4 : byte % 16;
}

void midi_input(boost::asio::ip::tcp::socket* socket) {
	std::queue<unsigned char> bytes_read;
	
	while(true) {
		while(socket->available() > 0) {
			std::cout << "byte get" << std::endl;
			char byte;
			socket->read_some(boost::asio::buffer(&byte, 1));
			if (byte != 254) bytes_read.push(byte);
		}

		bool keepReading = true;	
		while (keepReading && !bytes_read.empty()) {
			auto byte = bytes_read.front();
			
			switch(nibble(byte)) {
			case 8: //1000 : noteOff
				{
					if (bytes_read.size() < 3) {
						keepReading = false;
						break;
					}
					bytes_read.pop();

					unsigned char key, velocity;
					key = bytes_read.front(); bytes_read.pop();
					velocity = bytes_read.front(); bytes_read.pop();
					midi_note_current_velocities[key] = 0;

					break;
				}
			case 9: //1001 : noteOn
				{
					if (bytes_read.size() < 3) {
						keepReading = false;
						break;
					}
					bytes_read.pop();
					
					unsigned char key, velocity;
					key = bytes_read.front(); bytes_read.pop();
					velocity = bytes_read.front(); bytes_read.pop();
					midi_note_current_velocities[key] = velocity;

					break;
				}
			default: bytes_read.pop();
			}
		}
	}
}

int main(int argc, char** argv) {
	/*
	ALCdevice* dev = NULL;
	ALCcontext* ctx = NULL;

	auto defname = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);

	dev = alcOpenDevice(defname);
	ctx = alcCreateContext(dev, NULL);

	unsigned int src;
	alGenSources(1, &src);

	unsigned int buffers [4];
	alGenBuffers(4, buffers);
	
	for (unsigned int i=0; i<4; i++) {
	}
	*/

	boost::asio::io_service io_s;
	
	boost::asio::ip::tcp::socket socket{io_s};
	
	boost::asio::ip::tcp::endpoint endpoint;
	endpoint.address(boost::asio::ip::address::from_string("192.168.1.41"));
	endpoint.port(5001);

	std::cout << endpoint << std::endl;
	
	socket.connect(endpoint);

	std::thread io_service_thread ([&io_s](){io_s.run();});
	std::thread midi_thread(midi_input, &socket);
	
	while(true) {
		for (int note = 0; note<128; note++) {
			if (midi_note_current_velocities[note] != 0) {
				std::cout << note << ": " << (unsigned int)midi_note_current_velocities[note] << std::endl;
			}
		}
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	
}
