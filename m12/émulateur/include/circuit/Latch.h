#ifndef LATCH_H
#define LATCH_H
#include <functional>

class EdgeTriggeredLatchWire{
	public:
		void CChangeIn(bool b){
			if ((!b)&&this->c){
				this->c=b;
				this->sendOUT(this->in);
			}
			else this->c=b;
		}
		void INChangeIn(bool d){
			this->in=d;
		}
		//void subscribeOUT(void (*f)(bool)){
		void subscribeOUT(std::function<void(bool)> f){
			this->sendOUT=f;
		}
	private:
		//void (*sendOUT)(bool);
		std::function<void(bool)> sendOUT=[](bool b){};
		bool in;
		bool c=true;
};
#endif