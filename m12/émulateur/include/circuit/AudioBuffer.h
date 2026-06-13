#ifndef AUDIOBUFFER_H
#define AUDIOBUFFER_H

#include <atomic>
#include <cmath>

#include "miniaudio/miniaudio.h"

class AudioBuffer{
	public:
		//emulation thread
		AudioBuffer(){
			ma_pcm_rb_init(ma_format_f32, 1, 1024, NULL, NULL, &(this->rb));//TODO: buffer length
		}
		~AudioBuffer(){
			ma_pcm_rb_uninit(&(this->rb));
		}
		void AudioIn(float s){
			if (ma_pcm_rb_available_write(&(this->rb))!=0){
				ma_uint32 framesToWrite=1;
				void* pMappedBuffer;
				ma_pcm_rb_acquire_write(&(this->rb), &framesToWrite, &pMappedBuffer);
				*((float*)pMappedBuffer)=s;
				ma_pcm_rb_commit_write(&(this->rb), 1);
			}
		}
		//video/configuration thread
		void setVolumeLog(float v){
			this->volume.store((exp(v/100.)-1)/(M_E-1),std::memory_order_relaxed);
		}
		//audio thread
		ma_uint32 AudioOut(float* pOutput, ma_uint32 frameCount){
			ma_uint32 frames=ma_pcm_rb_available_read(&(this->rb));
			ma_uint32 framesRemaining;
			if (frames>=frameCount){
				framesRemaining=frames-frameCount;
				frames=frameCount;
			}
			else framesRemaining=0;
			//else printf("fc %u\n",frameCount-frames);
			ma_uint32 frameIndex=0;
			void* pMappedBuffer;
			while (frames!=frameIndex){
				ma_uint32 framesToRead=frames-frameIndex;
				ma_pcm_rb_acquire_read(&(this->rb), &framesToRead, &pMappedBuffer);
				ma_mix_pcm_frames_f32(&(pOutput[frameIndex]), (const float*)pMappedBuffer, framesToRead,1,this->volume.load(std::memory_order_relaxed));
				frameIndex+=framesToRead;
				ma_pcm_rb_commit_read(&(this->rb), framesToRead);
			}
			return framesRemaining;
		}
	private:
		ma_pcm_rb rb;
		std::atomic<float> volume=1.;
	
};


#endif