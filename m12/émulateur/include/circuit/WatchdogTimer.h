#ifndef WATCHDOGTIMER_H
#define WATCHDOGTIMER_H
#include <functional>
#include <cstdio>

class WatchdogTimer{//74HC4538 + 7705AC
	public:
		void incrementTimer(){
			if ((bool)this->kickTimer1){
				this->kickTimer1--;
				if (!(bool)this->kickTimer1){
					this->kickTimer2=2*this->kickPeriodMax;
					this->updatenWRST();
				}
			}
			if ((bool)this->kickTimer2){
				this->kickTimer2--;
				if (!(bool)this->kickTimer2) this->updatenWRST();
			}
			if ((bool)this->powerUpTimer){
				this->powerUpTimer--;
				if (!(bool)this->powerUpTimer){
					this->powerDownTimer=0;
					this->sendnWRST(true);
					this->sendRST(false);
				}
			}
			if ((bool)this->powerDownTimer){
				this->powerDownTimer--;
				if (!(bool)this->powerDownTimer){
					this->sendRST(true);
				}
			}
		}
		void ENChangeIn(bool b){
			if (this->EN!=b){
				this->EN=b;
				if (!b){
					this->kickTimer1=0;
					this->kickTimer2=0;
					this->updatenWRST();
				}
			}
		}
		void PWRChangeIn(bool b){
			if (b!=this->PWR){
				this->PWR=b;
				this->updatenWRST();
			}
		}
		void KICKChangeIn(bool b){
			if (b!=this->KICK){
				this->KICK=b;
				if (!b){
					if (this->EN){
						this->kickTimer1=this->kickPeriodMax;
					}
				}
			}
		}
		
		void subscribeRST(std::function<void(bool)> f){
			this->sendRST=f;
		}
		void subscribenWRST(std::function<void(bool)> f){
			this->sendnWRST=f;
		}
	private:
		bool KICK=false;
		bool EN=false;
		bool PWR=false;
		unsigned long powerUpTimer=0;
		unsigned long powerDownTimer=0;//just for RST up signal delay
		unsigned long kickTimer1=0;//kickTimer1!=0->out=true
		unsigned long kickTimer2=0;//kickTimer2!=0->out=false
		static constexpr unsigned long kickPeriodMax=500;//fs=100Hz / slope at 400mV/10ms (but doesn't start at 0V)
		static constexpr unsigned long powerUpDelay=183;//measured 19ms
		static constexpr unsigned long powerDownDelay=163;//measured slope at 1V/5ms / must be faster than powerUpDelay
		
		std::function<void(bool)> sendRST=[](bool b){};
		std::function<void(bool)> sendnWRST=[](bool b){};
		
		void updatenWRST(){
			if (this->PWR&&!(bool)this->kickTimer2){
				this->powerUpTimer=this->powerUpDelay;
			}
			else{
				this->powerUpTimer=0;
				this->powerDownTimer=this->powerDownDelay;
				this->sendnWRST(false);
			}
		}
};


#endif