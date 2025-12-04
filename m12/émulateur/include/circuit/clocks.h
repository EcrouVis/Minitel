#ifndef CLOCKS_H
#define CLOCKS_H
#include <functional>
class Clocks{
	public:
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
		void subscribeMailbox(std::function<void()> f){
			this->checkMailbox=f;
		}
		void start(){
			while (!this->stop()){
				this->checkMailbox();
				if (!this->pause()){
					this->div++;
					this->CLK14745600();
					if (this->div>=this->div_max){
						this->div=0;
						this->CLK600();
					}
				}
			}
		}
	private:
		std::function<bool()> pause=[](){return false;};
		std::function<bool()> stop=[](){return false;};
		std::function<void()> CLK14745600=[](){};
		std::function<void()> CLK600=[](){};
		std::function<void()> checkMailbox=[](){};
		unsigned long div=0;
		unsigned long div_max=24576;	
};
#endif