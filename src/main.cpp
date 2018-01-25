#include <iostream>
#include "al.h"
#include "alc.h"
#include <thread>
#include <array>
#include <chrono>
#include <queue>
#include <atomic>

#include <wiringPi.h>
#include <wiringSerial.h>

std::array<unsigned char, 128> midi_note_current_velocities;

unsigned char nibble(unsigned char byte, bool first = true) {
	return first ? byte >> 4 : byte % 16;
}

void midi_input() {
	if(wiringPiSetup() == -1) std::cout << "Setting up wiringPi failed" << std::endl;
	
	int serial_fd;
	if((serial_fd = serialOpen("/dev/ttyAMA0", 38400)) < 0) {
		std::cerr << "Couldn't open serial connection" << std::endl;
	}
	
	std::queue<unsigned char> bytes_read;
	
	while(true) {
		while(serialDataAvail(serial_fd)) {
			auto byte = serialGetchar(serial_fd);
			if (byte != 254) bytes_read.push(byte);
		}

		bool keepReading = true;	
		while (keepReading && !bytes_read.empty()) {
			std::cout << bytes_read.size() << std::endl;
			auto byte = bytes_read.front();
			
			switch(nibble(byte)) {
			case 8: //1000 : noteOff
				{
					if (bytes_read.size() < 3) {
						keepReading = false;
						break;
					}
					std::cout << "note off" << std::endl;
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

int main() {
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

	std::thread midi(midi_input);
	while(true) {
		for (int note = 0; note<128; note++) {
			if (midi_note_current_velocities[note] != 0) {
				std::cout << note << ": " << (unsigned int)midi_note_current_velocities[note] << std::endl;
			}
		}
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	
}
