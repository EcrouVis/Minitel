#ifndef TS7514_H
#define TS7514_H
#include <functional>
#include <cstdio>
class TS7514{
	public:
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
			if (b&&(!this->MCnBC)&&(!this->MODEMnDTMF)) this->writeRegister();
			this->MODEMnDTMF=b;
		}
		void MCnBCChangeIn(bool b){
			if (b&&(!this->MCnBC)&&(!this->MODEMnDTMF)) this->writeRegister();
			this->MCnBC=b;
		}
		void TxDChangeIn(bool b){
			this->TxD=b;
		}
		void nRTSChangeIn(bool b){
			if ((!b)&&this->nRTS){
				this->input_register=(this->input_register>>1)|(this->TxD?0x80:0);
			}
			this->nRTS=b;
		}
		void ATxIChangeIn(bool b){
			
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
		
		std::function<void(bool)> sendRxD=[](bool b){};
		std::function<void(bool)> sendWLO=[](bool b){};
		std::function<void(bool)> sendnDCD=[](bool b){};
		
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
				printf("OK\n");
			}
			else{
				printf("NOK\n");
			}
		}
	
};
#endif