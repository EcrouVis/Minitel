#ifndef BUZZERBUFFER_H
#define BUZZERBUFFER_H

#include "circuit/AudioBuffer.h"
#include <cstring>

//using BuzzerBuffer=AudioBufferOut;
class BuzzerBuffer: public AudioBufferOut{
	public:
		//emulation thread
		BuzzerBuffer(){
			this->lpf_config = ma_lpf_config_init(ma_format_f32, 1, 48000, 3000, 1);
			ma_lpf_init(&(this->lpf_config), NULL, &(this->filter));
		}
		~BuzzerBuffer(){
			ma_lpf_uninit(&(this->filter),NULL);
		}
		void setSampleRate(ma_uint32 sampleRate){
			this->lpf_config.sampleRate=sampleRate;
			ma_lpf_reinit(&(this->lpf_config), &(this->filter));
		}
		//video/configuration thread
		
		//audio thread
		void AudioOut(float* pOutput, ma_uint32 frameCount){
			float data[frameCount]={0};
			AudioBufferOut::AudioOut(data,frameCount);
			ma_lpf_process_pcm_frames(&(this->filter),data,data,frameCount);
			ma_mix_pcm_frames_f32(pOutput, (const float*)data, frameCount,1,1);
		}
	private:
		ma_lpf_config lpf_config;
		ma_lpf filter;
		
};

#endif