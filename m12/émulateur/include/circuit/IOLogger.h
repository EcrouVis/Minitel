#ifndef IOLOGGER_H
#define IOLOGGER_H
#include <cstdio>
#include <iostream>
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
			if(b&&(!this->nOE)&&(!this->nCS)) {
				//(*this->sendD)(r);
				this->sendOUT(this->address,this->data);
				/*if (this->address==0x96){
				int a;
				std::cin>>a;
				}*/
			}
			this->nOE=b;
		}
		void nWEChangeIn(bool b){
			if(b&&(!this->nWE)&&(!this->nCS)){
				this->sendIN(this->address,this->data);
				/*int a;
				std::cin>>a;*/
			}
			this->nWE=b;
		}
		void nCSChangeIn(bool b){
			this->nCS=b;
		}
		void subscribeIN(std::function<void(unsigned char,unsigned char)> f){
			this->sendIN=f;
		}
		void subscribeOUT(std::function<void(unsigned char,unsigned char)> f){
			this->sendOUT=f;
		}
	private:
		bool nCS=true;
		bool nOE=true;
		bool nWE=true;
		bool ALE=true;
		unsigned char address=0;
		unsigned char data=0;
		std::function<void(unsigned char,unsigned char)> sendIN=[](unsigned char a,unsigned char d){};
		std::function<void(unsigned char,unsigned char)> sendOUT=[](unsigned char a,unsigned char d){};
		//void (*sendD)(unsigned char);
};
#endif