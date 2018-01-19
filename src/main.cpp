#include <iostream>
#include "al.h"
#include "alc.h"
extern "C"{
#include <wiringPi.h>
#include <wiringSerial.h>
}

void buffering(unsigned int src) {
	
}

int main() {
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

	int serial_fd;
	if(serial_fd = serialOpen("/dev/ttyAMA0", 31250) < 0) {
		std::cerr << "Couldn't open serial connection" << std::endl;
	}

	while(true) {
		while(serialDataAvail(serial_fd)) {
			std::cout << serialGetchar(serial_fd) << '\n';
		}	
	}
}
