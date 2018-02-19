#include <boost/asio.hpp>
#include <queue>
#include <array>
#include <iostream>
#include <thread>
#include "synthesizer.hpp"
#include <iomanip>
#include <Vc/Vc>
#include <RtAudio.h>
#include <future>
#include <sndfile.h>

bool running = true;

unsigned char nibble(unsigned char byte, bool first = true) {
	return first ? byte >> 4 : byte % 16;
}

void midi_input(boost::asio::ip::tcp::socket* socket) {
	std::queue<unsigned char> bytes_read;
	
	while(running) {
		while(socket->available() > 0) {
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
					Synthesizer::set_key_velocity(key, 0);
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
					Synthesizer::set_key_velocity(key, velocity);
					break;
				}
			default: bytes_read.pop();
			}
		}
	}
}

double t = 0;
SNDFILE* file;

int streamCallback (void* output_buf, void* input_buf, unsigned int frame_count, double time_info, unsigned int stream_status, void* userData) {
	if(stream_status) std::cout << "Stream underflow" << std::endl;
	float* out = (float*) output_buf;
	for (int i = 0; i<frame_count; i++) {
		float sample = Synthesizer::get_sample(t);
		
		*out++ = sample;

		sf_writef_float(file, &sample, 1 /*frame*/);
		
		t += (double)1/(double)48000;
		if (t > 5) t = 0;
	}
	return 0;
}

int Vc_CDECL main(int argc, char** argv) {
	boost::asio::io_service io_s;
	
	boost::asio::ip::tcp::socket socket{io_s};
	
	boost::asio::ip::tcp::endpoint endpoint;
	endpoint.address(boost::asio::ip::address::from_string("192.168.1.42"));
	endpoint.port(5001);
	
	socket.connect(endpoint);

	Synthesizer::init();
	Synthesizer::harmonics[0] = 1.f;
	
	std::generate(Synthesizer::harmonics.begin(), Synthesizer::harmonics.end(), [n=0]() mutable {
			n++;
			if (n%2 == 0) return -1/3.14159268/n;
			return 1/3.14159268/n;
		});
		
	
	std::generate(Synthesizer::harmonics.begin(), Synthesizer::harmonics.end(), [n=0]() mutable -> float {
			n++;
			if (n%2 == 0) return 0;
			return 4/n/3.14159268;
		});


	SF_INFO file_info;
	file_info.samplerate = 48000;
	file_info.channels = 1;
	file_info.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;

	file = sf_open("output.wav", SFM_WRITE, &file_info);
	
	std::thread io_service_thread ([&io_s](){io_s.run();});
	std::thread midi_thread(midi_input, &socket);

	RtAudio dac;
	/*
	for (int i = 1; i<=dac.getDeviceCount(); i++) {
		std::cout << dac.getDeviceInfo(i).name << std::endl;	
  	}	
	*/

	RtAudio::StreamParameters params;
	params.deviceId = dac.getDefaultOutputDevice();
	params.nChannels = 1;
	params.firstChannel = 0;
	unsigned int buffer_length = 32;

	dac.openStream(&params, nullptr, RTAUDIO_FLOAT32, std::atoi(argv[1]) /*sample rate*/, &buffer_length /*frames per buffer*/, streamCallback, nullptr /*data ptr*/);

	dac.startStream();
	
	//std::promise<void>().get_future().wait();

	std::cin.get();

	running = false;

	dac.stopStream();
	
	io_service_thread.join();
	midi_thread.join();

	std::cout << sf_strerror(file) << std::endl;
	std::cout << sf_close(file) << std::endl;
}
