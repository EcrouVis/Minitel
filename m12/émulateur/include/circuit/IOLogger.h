#ifndef IOLOGGER_H
#define IOLOGGER_H
#include <cstdio>
#include <functional>
class IOLogger{
	public:
		void DChangeIn(unsigned char d){
			this->data=d;
		}
		void ALEChangeIn(bool b){
			if (b&&!this->ALE){
				this->address=this->data;
			}
			this->ALE=b;
		}
		void nOEChangeIn(bool b){
			if((!b)&&this->nOE&&(!this->nCS)) {
				unsigned char r=0;
				printf("IO I %#02X %#02X\n",this->address,r);
				//(*this->sendD)(r);
				this->sendD(r);
			}
			this->nOE=b;
		}
		void nWEChangeIn(bool b){
			if(!b&&this->nWE&&(!this->nCS)) printf("IO O %#02X %#02X\n",this->address,this->data);
			this->nWE=b;
		}
		void nCSChangeIn(bool b){
			this->nCS=b;
		}
		//void subscribeD(void (*f)(unsigned char)){
		void subscribeD(std::function<void(unsigned char)> f){
			this->sendD=f;
		}
	private:
		bool nCS=true;
		bool nOE=true;
		bool nWE=true;
		bool ALE=true;
		unsigned char address=0;
		unsigned char data=0;
		std::function<void(unsigned char)> sendD;
		//void (*sendD)(unsigned char);
};
#endif