#ifndef SPEAKERFILTER_H
#define SPEAKERFILTER_H
#include <atomic>
#include <cmath>
#ifndef M_PI
#define M_PI (3.14159265358979323846264338327950288)
#endif
#ifndef M_E
#define M_E  2.7182818284590452354
#endif
class SpeakerFilter{
	public:
		void setVolumeLog(float v){
			this->volume.store((exp(v/100.)-1)/(M_E-1),std::memory_order_relaxed);
		}
		float filter(float s){
			return s*this->volume.load(std::memory_order_relaxed);
		}
	private:
		std::atomic<float> volume=1.;
};
#endif