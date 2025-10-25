#ifndef LATCH_H
#define LATCH_H
#include <functional>
class DLatchBus{
	public:
		void CChangeIn(bool b){
			this->c=b;
			this->updateState();
		}
		void INChangeIn(unsigned char d){
			this->in=d;
			this->updateState();
		}
		//void subscribeOUT(void (*f)(unsigned char)){
		void subscribeOUT(std::function<void(unsigned char)> f){
			this->sendOUT=f;
		}
	private:
		//void (*sendOUT)(unsigned char);
		std::function<void(unsigned char)> sendOUT=[](unsigned char d){};
		unsigned char in;
		bool c=true;
		void updateState(){
			if (this->c){
				this->sendOUT(this->in);
			}
		}
};
class EdgeTriggeredLatchBus{
	public:
		void CChangeIn(bool b){
			if (b&&!this->c){
				this->c=b;
				this->sendOUT(this->in);
			}
			else{
				this->c=b;
			}
		}
		void INChangeIn(unsigned char d){
			this->in=d;
		}
		//void subscribeOUT(void (*f)(unsigned char)){
		void subscribeOUT(std::function<void(unsigned char)> f){
			this->sendOUT=f;
		}
	private:
		//void (*sendOUT)(unsigned char);
		std::function<void(unsigned char)> sendOUT=[](unsigned char d){};
		unsigned char in;
		bool c=true;
};
class DLatchWire{
	public:
		void CChangeIn(bool b){
			this->c=b;
			this->updateState();
		}
		void INChangeIn(bool d){
			this->in=d;
			this->updateState();
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
		void updateState(){
			if (this->c){
				this->sendOUT(this->in);
			}
		}
};
class EdgeTriggeredLatchWire{
	public:
		void CChangeIn(bool b){
			if (b&&!this->c){
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