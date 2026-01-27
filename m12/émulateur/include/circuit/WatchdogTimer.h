#ifndef WATCHDOGTIMER_H
#define WATCHDOGTIMER_H
#include <functional>
#include <cstdio>
class WatchdogTimer{//74HC4538 + 7705AC
	public:
		void incrementTimer(){
			if (this->toff!=0&&this->toff!=this->toff_max){
				this->toff=this->toff+1;
				if (this->toff==this->toff_reset){
					this->sendRST(true);
				}
				if (this->toff==this->toff_max&&this->PWR){
					this->toff=0;
					this->sendnWRST(true);
					this->sendRST(false);
				}
			}
			if (this->t1!=0){
				this->t1+=1;
				if (this->t1==this->tmax){
					this->t1=0;
					this->toff=1;
					this->sendnWRST(false);
				}
			}
		}
		void ENChangeIn(bool b){
			if (!b){
				this->t1=0;
			}
			this->EN=b;
		}
		void PWRChangeIn(bool b){
			if (b!=this->PWR&&this->toff==0){
				this->toff=1;
				this->sendnWRST(false);
			}
			if (b&&this->toff==this->toff_max){
				this->toff=0;
				this->sendnWRST(true);
				this->sendRST(false);
			}
			this->PWR=b;
		}
		void KICKChangeIn(bool b){
			if ((!b)&&this->KICK){
				if (this->EN) this->t1=1;
			}
			this->KICK=b;
		}
		
		void subscribeRST(std::function<void(bool)> f){
			this->sendRST=f;
		}
		void subscribenWRST(std::function<void(bool)> f){
			this->sendnWRST=f;
		}
	private:
		//delta between warn and reset: 1128 clock periods minimum -> ISR 0x0003
		unsigned long t1=0;//timeout: t1==tmax
		unsigned long tmax=1474560;//guess (0.1s)
		unsigned long toff=0;
		unsigned long toff_reset=1536;//1128 (maximum time the interrupt can take)<t<1917 (toff_max)
		unsigned long toff_max=1917;//130us@10nF->1917 clock periods (guess)
		std::function<void(bool)> sendRST=[](bool b){};
		std::function<void(bool)> sendnWRST=[](bool b){};
		
		bool KICK=false;
		bool EN=true;
		bool PWR=false;
	
};


#endif