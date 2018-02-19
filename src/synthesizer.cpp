#include "synthesizer.hpp"
#include "utils.hpp"
#include <ratio>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <thread>

#include <Vc/Vc>

std::array<float, nh> Synthesizer::harmonics = {0};
std::chrono::time_point<std::chrono::high_resolution_clock> Synthesizer::start_time, Synthesizer::test_time;
std::array<unsigned char, 128> Synthesizer::key_velocities = {0};
bool Synthesizer::test_flag = false;
void Synthesizer::init() { 
	start_time = clock_t::now();
}

float Synthesizer::get_sample(double t) {
	Vc::float_v result = Vc::float_v::Zero();
	
	for (int i = 0; i<key_velocities.size(); i++) {
		if (key_velocities.at(i) == 0) continue;
		//std::cout << "key" << i << std::endl;
		auto v = get_key_velocity(i);
		float f = utils::midi_to_note_freq(i);
		int j = 0;
		for (;j + Vc::float_v::size() <= nh; j+=Vc::float_v::size()) {
			Vc::float_v twopift = Vc::float_v::generate([f,t,j](int n){return 2*3.14159268*(j+n+1)*f*t;});
			Vc::float_v harms = Vc::float_v::generate([harmonics, j](int n){return harmonics.at(n+j);});
			result += v*harms*Vc::sin(twopift); 
		}
	}
	return result.sum()/512;
}																								 

void Synthesizer::set_key_velocity(unsigned int key, unsigned char velocity) {
	key_velocities.at(key) = velocity;
}

unsigned char Synthesizer::get_key_velocity(unsigned int key) {
	return key_velocities[key];
}


