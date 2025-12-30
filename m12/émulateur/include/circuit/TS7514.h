#ifndef TS7514_H
#define TS7514_H
#include <functional>
#include <cstdio>
#include <atomic>
#include "PhoneLine.h"


class TS7514{
	public:
		
		std::atomic<float> buzzer_amplitude=0.;
		
		void Reset(){
			this->REG[this->RPROG]=0;
			this->REG[this->RATE]=0x0F;
			this->REG[this->RWLO]=0x0C;
			this->REG[this->RPRF]=0x01;
			this->REG[this->RPTF]=0x00;
			this->REG[this->RHDL]=0x00;
			this->REG[this->RPRX]=0x00;
		}
		
		void MODEMnDTMFChangeIn(bool b){
			if (b&&(!this->MODEMnDTMF)&&(!this->MCnBC)) this->writeRegister();
			//if (b!=(this->MODEMnDTMF)) printf("MODEMnDTMF %i \n",(int)b);
			this->MODEMnDTMF=b;
		}
		void MCnBCChangeIn(bool b){
			if (b&&(!this->MCnBC)&&(!this->MODEMnDTMF)) this->writeRegister();
			//if (b!=(this->MCnBC)) printf("MCnBC %i \n",(int)b);
			this->MCnBC=b;
		}
		void TxDChangeIn(bool b){
			//if (b!=this->TxD) printf("TxD %i \n",(int)b);
			this->TxD=b;
		}
		void nRTSChangeIn(bool b){
			//if (b!=this->nRTS) printf("nRTS %i \n",(int)b);
			if ((!b)&&this->nRTS){
				this->input_register=(this->input_register>>1)|(this->TxD?0x80:0);
			}
			this->nRTS=b;
		}
		void ATxIChangeIn(bool b){
			//if (b!=this->ATxI) printf("ATxI %i \n",(int)b);
			this->ATxI=b;
		}
		
		void subscribeRxD(std::function<void(bool)> f){
			this->sendRxD=f;
		}
		void subscribeWLO(std::function<void(bool)> f){
			this->sendWLO=f;
		}
		void subscribenDCD(std::function<void(bool)> f){
			this->sendnDCD=f;
		}
		
		void subscribeCMD(std::function<void(unsigned char)> f){
			this->sendCMD=f;
		}
		
		void updatePhoneLineState(){
			
		}
		
	private:
		enum Constants{
			RPROG=0,
			RDTMF=1,
			RATE=2,
			RWLO=3,
			RPTF=4,
			RPRF=5,
			RHDL=6,
			RPRX=7
		};
		unsigned char REG[8];
		unsigned char input_register;
		bool MODEMnDTMF=true;
		bool MCnBC=true;
		bool nRTS=false;
		bool TxD=false;
		bool ATxI=false;
		
		std::function<void(bool)> sendRxD=[](bool b){};
		std::function<void(bool)> sendWLO=[](bool b){};
		std::function<void(bool)> sendnDCD=[](bool b){};
		
		std::function<void(unsigned char)> sendCMD=[](unsigned char b){};
		
		void writeRegister(){
			unsigned char n_reg=(this->input_register>>4)&0x07;
			unsigned char rprog=this->REG[this->RPROG];
			bool cmd_ok=(((rprog&3)==0)||
						 ((rprog&3)==3)||
						 ((rprog&3)==1&&(this->input_register&0x80)==0)||
						 ((rprog&3)==2&&(this->input_register&0x80)==0x80)
						)&&((rprog&0x08)==0);
			printf("TS7514 CMD 0x%02X ",this->input_register);
			if (n_reg==this->RPROG||cmd_ok){
				this->REG[n_reg]=this->input_register&0x0F;
				switch(n_reg){
					case 3:
						if ((this->REG[3]&0x0C)==0x08){
							const float Amp[4]={1.,0.316227766,0.1,0.0316227766};
							buzzer_amplitude.store(Amp[this->REG[3]&0x03],std::memory_order_release);
						}
						else{
							buzzer_amplitude.store(0.,std::memory_order_release);
						}
						break;
				}
				printf("OK\n");
				this->sendCMD(this->input_register);
				
			}
			else{
				printf("NOK\n");
			}
		}
	
};
#endif