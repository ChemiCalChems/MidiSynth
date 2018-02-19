#include <array>
#include <chrono>

const unsigned short nh = 64;

class Synthesizer {
	using clock_t = std::chrono::high_resolution_clock;

	
	static std::chrono::time_point<clock_t> start_time;
	static std::array<unsigned char, 128> key_velocities;
	
public:
	static std::chrono::time_point<clock_t> test_time;
	static bool test_flag;
	static std::array<float, nh> harmonics;
	
	static void init();
	static float get_sample(double t);
	static void set_key_velocity(unsigned int key, unsigned char velocity);
	static unsigned char get_key_velocity(unsigned int key);
};
