#ifndef CLOCKS_H
#define CLOCKS_H
#include <functional>
#include <chrono>
#include <thread>
#include <mutex>
#include <limits>
#include <condition_variable>
class Clocks{
	public:
		Clocks(){
			this->audioMutex=new std::mutex();
		}
		~Clocks(){
			delete this->audioMutex;
		}
	
		void setPauseCondition(std::function<bool()> f){
			this->pause=f;
		}
		void setStopCondition(std::function<bool()> f){
			this->stop=f;
		}
		
		void subscribe14745600Hz(std::function<void()> f){
			this->CLK14745600=f;
		}
		void subscribe600Hz(std::function<void()> f){
			this->CLK600=f;
		}
		void subscribe4800Hz(std::function<void()> f){
			this->CLK4800=f;
		}
		void subscribe9600Hz(std::function<void()> f){
			this->CLK9600=f;
		}
		void subscribeMailbox(std::function<void()> f){
			this->checkMailbox=f;
		}
		void subscribeAudioSample(std::function<void()> f){
			this->audioSample=f;
		}
		
		void setAudioSampleRate(unsigned long sr){
			std::lock_guard<std::mutex> lock(*(this->audioMutex));
			this->audio_sample_rate.store(sr,std::memory_order_release);
			this->audioCV.notify_one();
		}
		void requestSamples(unsigned long n,unsigned long nmax=ULONG_MAX){
			std::lock_guard<std::mutex> lock(*(this->audioMutex));
			this->requestedSamples+=n;
			if (this->requestedSamples>nmax) this->requestedSamples=nmax;
			this->audioCV.notify_one();
		}
		
		void start(){
			this->last_t=std::chrono::steady_clock::now();
			while (!this->stop()){
				
				if (this->audio_sample_rate.load(std::memory_order_relaxed)==0){//clock
					this->div_sync++;
					if (this->div_sync>=this->div_sync_max){
						this->div_sync=0;
						
						std::chrono::time_point<std::chrono::steady_clock> new_t=std::chrono::steady_clock::now();
						this->dt+=std::chrono::duration<double>(new_t-this->last_t)-this->dt_sleep_max;
						//while (std::chrono::steady_clock::now()-new_t<this->dt_sleep_max-this->dt);
						if (this->dt<this->dt_sleep_max) std::this_thread::sleep_for(this->dt_sleep_max-this->dt);
						this->last_t=new_t;
					}
				}
				else{//sync to audio
					this->audio_div_sync+=this->audio_sample_rate.load(std::memory_order_relaxed);
					if (this->audio_div_sync>=this->master_clock_rate){
						this->audio_div_sync=this->audio_div_sync%this->master_clock_rate;
						this->audioSample();
						{
							std::unique_lock<std::mutex> lock(*(this->audioMutex));
							this->requestedSamples--;
							if (this->requestedSamples<=0){
								this->audioCV.wait(lock, [this](){
									return (this->requestedSamples>0)||(this->audio_sample_rate.load(std::memory_order_relaxed)==0);
								});//requested samples or switch to clock sync
							}
						}
					}
				}
				
				this->checkMailbox();
				
				if (!this->pause()){
					this->CLK14745600();
					
					this->div9600++;
					if (this->div9600>=this->div9600_max){
						this->div9600=0;
						this->CLK9600();
						
						this->div9600_4800++;
						if (this->div9600_4800>=this->div9600_4800_max){
							this->div9600_4800=0;
							this->CLK4800();
							
							this->div4800_600++;
							if (this->div4800_600>=this->div4800_600_max){
								this->div4800_600=0;
								this->CLK600();
							}
						}
					}
				}
			}
		}
	private:
		std::function<bool()> pause=[](){return false;};
		std::function<bool()> stop=[](){return false;};
		
		std::function<void()> CLK14745600=[](){};
		std::function<void()> CLK600=[](){};
		std::function<void()> CLK4800=[](){};
		std::function<void()> CLK9600=[](){};
		std::function<void()> checkMailbox=[](){};
		std::function<void()> audioSample=[](){};
		
		unsigned long div9600=0;
		unsigned long div9600_max=1536;
		unsigned long div9600_4800=0;
		unsigned long div9600_4800_max=2;
		unsigned long div4800_600=0;
		unsigned long div4800_600_max=8;
		
		std::mutex* audioMutex;
		std::condition_variable audioCV;
		unsigned long requestedSamples=0;
		std::atomic<unsigned long> audio_sample_rate=0;
		const unsigned long master_clock_rate=14745600;
		unsigned long audio_div_sync=0;
		
		unsigned long div_sync=0;
		unsigned long div_sync_max=524288;
		std::chrono::time_point<std::chrono::steady_clock> last_t;
		std::chrono::duration<double> dt;
		const std::chrono::duration<double> dt_sleep_max=std::chrono::microseconds(32000000/900);
};
#endif