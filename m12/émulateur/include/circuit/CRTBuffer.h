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
			std::unique_lock<std::mutex> lock(this->videoMutex);
			for (int i=0;i<VIDEO_FRAME_SIZE;i++){
				if (v[i]!=this->data[i]){
					change=true;
					this->data[i]=v[i];
				}
			}
			if (change){
				this->newFrame.store(true,std::memory_order_release);
				this->sendSignal();
			}
			
		}
		void subscribeSignal(std::function<void()> f){
			this->sendSignal=f;
		}
		
		void getVideoFrame(unsigned char* buffer){
			std::unique_lock<std::mutex> lock(this->videoMutex);
			this->newFrame.store(false,std::memory_order_release);
			memcpy(buffer,this->data,VIDEO_FRAME_SIZE*sizeof(unsigned char));
		}
		bool frameChanged(){
			return this->newFrame.load(std::memory_order_acquire);
		}
	private:
		unsigned char data[VIDEO_FRAME_SIZE];
		std::function<void()> sendSignal=[](){};
		
		std::mutex videoMutex;
		std::atomic_bool newFrame;
		
};

#endif