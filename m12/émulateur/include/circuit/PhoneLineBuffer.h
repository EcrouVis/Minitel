#ifndef PHONELINEBUFFER_H
#define PHONELINEBUFFER_H

#include <atomic>
#include <cmath>
#ifndef M_E
#define M_E  2.7182818284590452354
#endif
#include "miniaudio/miniaudio.h"

class PhoneLineBuffer{
	public:
		//emulation thread
		PhoneLineBuffer(){
			ma_pcm_rb_init(ma_format_f32, 1, 1024, NULL, NULL, &(this->rbPlayback));//TODO: buffer length
			ma_pcm_rb_init(ma_format_f32, 1, 1024, NULL, NULL, &(this->rbCapture));
		}
		~PhoneLineBuffer(){
			ma_pcm_rb_uninit(&(this->rbPlayback));
			ma_pcm_rb_uninit(&(this->rbCapture));
		}
		void AudioEmulatorOut(float s){
			if (ma_pcm_rb_available_write(&(this->rbPlayback))!=0){
				ma_uint32 framesToWrite=1;
				void* pMappedBuffer;
				ma_pcm_rb_acquire_write(&(this->rbPlayback), &framesToWrite, &pMappedBuffer);
				*((float*)pMappedBuffer)=s;
				ma_pcm_rb_commit_write(&(this->rbPlayback), 1);
			}
		}
		float AudioEmulatorIn(){
			float s=0;
			if (ma_pcm_rb_available_read(&(this->rbCapture))!=0){
				ma_uint32 framesToRead=1;
				void* pMappedBuffer;
				ma_pcm_rb_acquire_read(&(this->rbCapture), &framesToRead, &pMappedBuffer);
				s=*((float*)pMappedBuffer);
				ma_pcm_rb_commit_read(&(this->rbCapture), 1);
			}
			return s;
		}
		//video/configuration thread
		void setVolumeOutdB(float v){
			this->volumeOut.store(pow(10.,v/20.),std::memory_order_relaxed);
		}
		void setVolumeIndB(float v){
			this->volumeIn.store(pow(10.,v/20.),std::memory_order_relaxed);
		}
		//audio thread
		ma_uint32 AudioIO(float* pInput, float* pOutput, ma_uint32 frameCount){
			ma_uint32 frames=ma_pcm_rb_available_read(&(this->rbPlayback));
			ma_uint32 framesRemaining;
			if (frames>=frameCount){
				framesRemaining=frames-frameCount;
				frames=frameCount;
			}
			else framesRemaining=0;
			
			ma_uint32 frameIndex=0;
			void* pMappedBuffer;
			while (frames!=frameIndex){
				ma_uint32 framesToRead=frames-frameIndex;
				ma_pcm_rb_acquire_read(&(this->rbPlayback), &framesToRead, &pMappedBuffer);
				ma_mix_pcm_frames_f32(&(pOutput[frameIndex]), (const float*)pMappedBuffer, framesToRead,1,this->volumeOut.load(std::memory_order_relaxed));
				frameIndex+=framesToRead;
				ma_pcm_rb_commit_read(&(this->rbPlayback), framesToRead);
			}
			
			frames=ma_pcm_rb_available_write(&(this->rbCapture));
			if (frames>=frameCount){
				frames=frameCount;
			}
			frameIndex=0;
			while (frames!=frameIndex){
				ma_uint32 framesToWrite=frames-frameIndex;
				ma_pcm_rb_acquire_write(&(this->rbCapture), &framesToWrite, &pMappedBuffer);
				//ma_copy_pcm_frames(pMappedBuffer, (const float*)&(pInput[frameIndex]), framesToWrite, ma_format_f32, 1);
				ma_copy_and_apply_volume_factor_pcm_frames_f32((float*)pMappedBuffer, (const float*)&(pInput[frameIndex]), framesToWrite, 1, this->volumeIn.load(std::memory_order_relaxed));
				frameIndex+=framesToWrite;
				ma_pcm_rb_commit_write(&(this->rbCapture), framesToWrite);
			}
			
			return framesRemaining;
		}
	private:
		ma_pcm_rb rbPlayback;
		ma_pcm_rb rbCapture;
		std::atomic<float> volumeOut=1.;
		std::atomic<float> volumeIn=1.;
	
};


#endif