#ifndef CLOCKS_H
#define CLOCKS_H
#include <functional>
#include <chrono>
#include <thread>
#include <mutex>
#include <climits>
#include <condition_variable>
class Clocks{
	public:
		/*void setPauseCondition(std::function<bool()> f){
			this->pause=f;
		}*/
		void setPause(bool b){
			this->pause=b;
		}
		/*void setStopCondition(std::function<bool()> f){
			this->stop=f;
		}*/
		void setStop(bool b){
			this->stop=b;
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
		void subscribeAudioSample(std::function<void(unsigned long)> f){
			this->audioSample=f;
		}
		
		void setAudioSampleRate(unsigned long sr){//cannot 
			this->audio_sample_rate.store(sr,std::memory_order_release);
		}
		void requestSamples(unsigned long n,unsigned long nmax=ULONG_MAX){
			{
				std::lock_guard<std::mutex> lock(this->audioMutex);
				this->requestedSamples+=n;
				if (this->requestedSamples>nmax) this->requestedSamples=nmax;
			}
			this->audioCV.notify_one();
		}
		
		void start(){
			std::unique_lock lock(this->audioMutex,std::defer_lock);
			while (true){
				//sync to audio
				this->audio_div_sync+=this->audio_sample_rate.load(std::memory_order_relaxed);
				if (this->audio_div_sync>=this->master_clock_rate){
					this->audio_div_sync-=this->master_clock_rate;
					this->audioSample(this->audio_sample_rate.load(std::memory_order_relaxed));
					
					/*
					because of wait_for there is a case where this->requestedSamples can underflow
					ex: 
						pc was in deep sleep and woke up
						-> program resume running before audio callback is called
						-> audio callback not called
						-> this->audioCV.wait_for timeout multiple times
						-> this->requestedSamples underflow
						-> mailbox not checked
						-> emulation feel not responsive
					fix:
						if ((bool)this->requestedSamples) this->requestedSamples--;
						instead of
						this->requestedSamples--;
						or
						if (!(bool)this->requestedSamples) this->requestedSamples++;
						after this->audioCV.wait_for
					*/
					lock.lock();
					this->requestedSamples--;
					if (this->requestedSamples==0){
						
						//this->t=std::chrono::system_clock::now();
						
						this->audioCV.wait_for(lock, std::chrono::milliseconds(500), [this](){return (bool)this->requestedSamples;});//requested samples
						if (!(bool)this->requestedSamples) this->requestedSamples=1;
						lock.unlock();//avoid blocking audio thread while checking mailbox
						this->checkMailbox();//check less often mailbox
						if (this->stop) return;//if (this->stop()) return;
						
						//std::chrono::time_point<std::chrono::system_clock> n=std::chrono::system_clock::now();
						//printf("%llu\n",(n-this->t).count());
					}
					else lock.unlock();
				}
				
				if (!this->pause){//(!this->pause()){
					this->CLK14745600();
					
					if (!(bool)--this->div9600){//test if not 0 slightly more efficient than testing if greater than div9600_max (use TEST instead of CMP)
						this->div9600=this->div9600_max;
						this->CLK9600();
						
						if (!(bool)--this->div9600_4800){
							this->div9600_4800=this->div9600_4800_max;
							this->CLK4800();
							
							//this->div4800_600++;
							if (!(bool)--this->div4800_600){
								this->div4800_600=this->div4800_600_max;
								this->CLK600();
							}
						}
					}
				}
			}
		}
	private:
		//std::function<bool()> pause=[](){return false;};
		bool pause=false;
		//std::function<bool()> stop=[](){return false;};
		bool stop=false;
		
		std::function<void()> CLK14745600=[](){};
		std::function<void()> CLK600=[](){};
		std::function<void()> CLK4800=[](){};
		std::function<void()> CLK9600=[](){};
		std::function<void()> checkMailbox=[](){};
		std::function<void(unsigned long)> audioSample=[](unsigned long sr){};
		
		constexpr static unsigned long div9600_max=1536;
		unsigned long div9600=this->div9600_max;
		constexpr static unsigned long div9600_4800_max=2;
		unsigned long div9600_4800=this->div9600_4800_max;
		constexpr static unsigned long div4800_600_max=8;
		unsigned long div4800_600=this->div4800_600_max;
		
		std::mutex audioMutex;
		std::condition_variable audioCV;
		unsigned long requestedSamples=1;
		constexpr static unsigned long master_clock_rate=14745600;
		std::atomic<unsigned long> audio_sample_rate=this->master_clock_rate;
		unsigned long audio_div_sync=0;
		
		std::chrono::time_point<std::chrono::system_clock> t=std::chrono::system_clock::now();
		
		/*unsigned long div_sync=0;
		unsigned long div_sync_max=524288;
		std::chrono::time_point<std::chrono::steady_clock> last_t;
		std::chrono::duration<double> dt;
		const std::chrono::duration<double> dt_sleep_max=std::chrono::microseconds(32000000/900);*/
};
#endif