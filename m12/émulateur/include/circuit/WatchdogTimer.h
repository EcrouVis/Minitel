#ifndef WATCHDOGTIMER_H
#define WATCHDOGTIMER_H
#include <functional>
#include <cstdio>
class WatchdogTimer{//74HC4538 + 7705AC
	public:
		void incrementTimer(){
			if (this->EN){
				if (this->t2!=0){
					this->t2=(this->t2+1)%this->tmax;
					if (this->t2==0) this->changeOutput();
				}
				if (this->t1!=0){
					this->t1=(this->t1+1)%this->tmax;
					if (this->t1==0){
						this->t2=1;
						this->changeOutput();
					}
				}
			}
			if (this->toff!=0) this->toff=(this->toff+1)%this->toff_max;
		}
		void ENChangeIn(bool b){
			if (b&&(!this->EN)){
				
			}
			this->EN=b;
			if (!b){
				this->t1=0;
				this->t2=0;
				this->changeOutput();
			}
		}
		void PWRChangeIn(bool b){
			if (b&&(!this->PWR)) this->toff=1;
			if (!b) this->toff=0;
			this->PWR=b;
			this->changeOutput();
		}
		void KICKChangeIn(bool b){
			if (b&&(!this->KICK)){
				this->KICK=b;
				//printf("%li\n",this->t1);
				this->t1=1;
			}
			else this->KICK=b;
		}
		
		void subscribeRST(std::function<void(bool)> f){
			this->sendRST=f;
		}
	private:
		unsigned long t1=0;//<160000 clock periods -> 10ms
		unsigned long t2=0;
		unsigned long trise=0;
		unsigned long tmax=49900;//3380us@10nF->49900 clock periods
		unsigned long toff=0;
		unsigned long toff_max=5;
		std::function<void(bool)> sendRST=[](bool b){};
		
		bool KICK=false;
		bool EN=true;
		bool PWR=false;
		bool nRST=false;
		
		void changeOutput(){
			this->nRST=((this->t2==0)&&(this->PWR&&(this->toff==0))&&this->EN);
			this->sendRST(!this->nRST);//720us
		}
	
};


#endif