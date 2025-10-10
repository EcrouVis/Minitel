#ifndef DLATCH_H
#define DLATCH_H
class DLatchBus{
	public:
		DLatchBus(){
			
			this->sendOUT=[](unsigned char d){};
		}
		void CChangeIn(bool b){
			this->c=b;
			this->updateState();
		}
		void INChangeIn(unsigned char d){
			this->in=d;
			this->updateState();
		}
		void subscribeOUT(void (*f)(unsigned char)){
			this->sendOUT=f;
		}
	private:
		void (*sendOUT)(unsigned char);
		unsigned char in;
		bool c=true;
		void updateState(){
			if (c){
				this->sendOUT(this->in);
			}
		}
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
		void subscribeOUT(void (*f)(bool)){
			this->sendOUT=f;
		}
	private:
		void (*sendOUT)(bool);
		bool in;
		bool c=true;
		void updateState(){
			if (c){
				this->sendOUT(this->in);
			}
		}
};
#endif