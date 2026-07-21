#ifndef CRTBUFFER_H
#define CRTBUFFER_H

#include <mutex>
#include <atomic>
#include <functional>
#include <cstring>

#include "circuit/TS9347.h"

class CRTBuffer{
	public:
		void VideoChangeIn(unsigned char* v){
			bool change=false;
			for (int i=0;i<VIDEO_FRAME_SIZE;i++){
				if (v[i]!=this->pFrameWrite[i]){
					change=true;
					this->pFrameWrite[i]=v[i];
				}
			}
			if (change){
				this->pFrameWrite=this->pFrameRead.exchange(this->pFrameWrite,std::memory_order_relaxed);
				this->sendSignal();
			}
			
		}
		void CRTPowerChangeIn(bool b){
			this->power.store(b,std::memory_order_relaxed);
		}
		void subscribeSignal(std::function<void()> f){
			this->sendSignal=f;
		}
		
		void getVideoFrame(unsigned char* buffer){
			if (this->power.load(std::memory_order_relaxed)) memcpy(buffer,this->pFrameRead.load(std::memory_order_acquire),VIDEO_FRAME_SIZE*sizeof(unsigned char));
			else memset(buffer,0,VIDEO_FRAME_SIZE*sizeof(unsigned char));
		}
	private:
		unsigned char frame1[VIDEO_FRAME_SIZE];
		unsigned char frame2[VIDEO_FRAME_SIZE];
		std::atomic<unsigned char*> pFrameRead=frame1;
		unsigned char* pFrameWrite=frame2;
		std::function<void()> sendSignal=[](){};
		
		std::atomic_bool power=false;
		
};

#endif