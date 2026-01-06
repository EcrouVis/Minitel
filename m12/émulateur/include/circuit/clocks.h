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
		void subscribe4800Hz(std::function<void()> f){
			this->CLK4800=f;
		}
		void subscribe9600Hz(std::function<void()> f){
			this->CLK9600=f;
		}
		void subscribeMailbox(std::function<void()> f){
			this->checkMailbox=f;
		}
		void start(){
			while (!this->stop()){
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
		unsigned long div9600=0;
		unsigned long div9600_max=1536;
		unsigned long div9600_4800=0;
		unsigned long div9600_4800_max=2;
		unsigned long div4800_600=0;
		unsigned long div4800_600_max=8;
};
#endif