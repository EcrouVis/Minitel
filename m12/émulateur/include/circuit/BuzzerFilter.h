#ifndef BUZZERFILTER_H
#define BUZZERFILTER_H
#include <atomic>
#include <cmath>
class BuzzerFilter{
	public:
		BuzzerFilter(unsigned long Fc){
			this->Fc=Fc;
		}
		void setVolumeLog(float v){
			this->volume.store((exp(v/100.)-1)/(M_E-1),std::memory_order_relaxed);
		}
		float filter(float s){
			this->lastSample+=this->a*(s-this->lastSample);
			return this->lastSample*this->volume.load(std::memory_order_relaxed);
		}
		void setSampleRate(unsigned long Fs){
			float d=2*M_PI*((float)this->Fc)/((float)Fs);
			this->a=d/(d+1);
		}
	private:
		unsigned long Fc;
		float a=0;
		float lastSample=0;
		std::atomic<float> volume=1.;
};
#endif