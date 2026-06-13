#ifndef SPEAKERFILTER_H
#define SPEAKERFILTER_H
#include <atomic>
#include <cmath>
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