#ifndef TS7514_H
#define TS7514_H
#include <functional>
#include <cstdio>
#include <atomic>
#define MA_NO_DECODING
#define MA_NO_ENCODING
#include "miniaudio/miniaudio.h"
#include "circuit/PhoneLine.h"


class TS7514{
	public:
		
		//std::atomic<float> buzzer_amplitude=0.;
		std::atomic_uchar REG[8];
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
		
		
		
		
		void Reset(){
			this->REG[this->RPROG].store(0,std::memory_order_release);
			this->REG[this->RATE].store(0x0F,std::memory_order_release);
			this->REG[this->RWLO].store(0x0C,std::memory_order_release);
			this->REG[this->RPRF].store(0x01,std::memory_order_release);
			this->REG[this->RPTF].store(0x00,std::memory_order_release);
			this->REG[this->RHDL].store(0x00,std::memory_order_release);
			this->REG[this->RPRX].store(0x00,std::memory_order_release);
		}
		
		void MODEMnDTMFChangeIn(bool b){
			if (b&&(!this->MODEMnDTMF)&&(!this->MCnBC)) this->writeRegister();
			//if (b!=(this->MODEMnDTMF)) printf("MODEMnDTMF %i \n",(int)b);
			this->MODEMnDTMF=b;
			this->nRTS_buf=this->nRTS;
			if (this->nRTS){
				this->MODEMnDTMF_buf=this->MODEMnDTMF;
				this->updateTx();
				this->updateRx();
			}
		}
		void MCnBCChangeIn(bool b){
			if (b&&(!this->MCnBC)&&(!this->MODEMnDTMF)) this->writeRegister();
			//if (b!=(this->MCnBC)) printf("MCnBC %i \n",(int)b);
			this->MCnBC=b;
			this->nRTS_buf=this->nRTS;
			if (this->nRTS){
				this->MCnBC_buf=this->MCnBC;
				this->updateTx();
				this->updateRx();
			}
		}
		void TxDChangeIn(bool b){
			if (b!=this->TxD){ //printf("TxD %i \n",(int)b);
				this->TxD=b;
				this->updateTx();
			}
		}
		void nRTSChangeIn(bool b){
			if (b){
				this->nRTS=b;
				if (this->MCnBC||this->MODEMnDTMF){
					this->MCnBC_buf=this->MCnBC;
					this->MODEMnDTMF_buf=this->MODEMnDTMF;
					this->nRTS_buf=this->nRTS;
					this->updateTx();
				}
			}
			else if ((!b)&&this->nRTS){
				this->nRTS=b;
				this->input_register=(this->input_register>>1)|(this->TxD?0x80:0);
			}
			else{
				this->nRTS=b;
			}
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
		
		void CLKTickIn(){//14745600Hz
			this->clk_div++;
			if (this->clk_div<this->clk_div_max) return;
			this->clk_div=0;
			//1228800Hz
			/*float fATxI;
			if (TS7514_ATxI_Resample_1228800_48000(this->ATxI,&fATxI)){
				//48000Hz -> update IO
				//printf("%f \n",fATxI);
				
			}*/
		}
		
		void RA2ChangeIn(unsigned short s){
			if (s!=this->Rx_dout){
				this->Rx_dout=s;
				this->updateRx();
			}
		}
		
	private:
		unsigned char input_register;
		bool MODEMnDTMF=true;
		bool MCnBC=true;
		bool MODEMnDTMF_buf=true;
		bool MCnBC_buf=true;
		bool nRTS=false;
		bool nRTS_buf=false;
		bool TxD=false;
		bool ATxI=false;
		
		unsigned short Tx_din;
		unsigned short Tx_dout;
		unsigned short Rx_dout;
		
		unsigned char clk_div=0;
		const unsigned char clk_div_max=12;
		
		std::function<void(bool)> sendRxD=[](bool b){};
		std::function<void(bool)> sendWLO=[](bool b){};
		std::function<void(bool)> sendnDCD=[](bool b){};
		
		std::function<void(unsigned char)> sendCMD=[](unsigned char b){};
		
		void writeRegister(){
			unsigned char n_reg=(this->input_register>>4)&0x07;
			unsigned char rprog=this->REG[this->RPROG].load(std::memory_order_relaxed);
			bool cmd_ok=(((rprog&3)==0)||
						 ((rprog&3)==3)||
						 ((rprog&3)==1&&(this->input_register&0x80)==0)||
						 ((rprog&3)==2&&(this->input_register&0x80)==0x80)
						)&&((rprog&0x08)==0);
			printf("TS7514 CMD 0x%02X ",this->input_register);
			if (n_reg==this->RPROG||cmd_ok){
				this->REG[n_reg].store(this->input_register&0x0F,std::memory_order_release);
				/*switch(n_reg){
					case 3:
						if ((this->REG[3]&0x0C)==0x08){
							const float Amp[4]={1.,0.316227766,0.1,0.0316227766};
							buzzer_amplitude.store(Amp[this->REG[3]&0x03],std::memory_order_release);
						}
						else{
							buzzer_amplitude.store(0.,std::memory_order_release);
						}
						break;
				}*/
				this->updateTx();
				this->updateRx();
				printf("OK\n");
				this->sendCMD(this->input_register);
				
			}
			else{
				printf("NOK\n");
			}
		}
		
		void updateTx(){
			unsigned short nTx_d=0;
			if (!this->nRTS){
				if (this->MODEMnDTMF_buf){
					if (this->REG[this->RPTF].load(std::memory_order_relaxed)!=0){
						nTx_d=0;
					}
					else if (this->MCnBC_buf){
						nTx_d=this->TxD?line_v23_1200bps_1:line_v23_1200bps_0;
					}
					else{
						nTx_d=this->TxD?line_v23_75bps_1:line_v23_75bps_0;
					}
				}
				else{
					nTx_d=0;
					unsigned char rptf=this->REG[this->RPTF].load(std::memory_order_relaxed)&0x0F;
					unsigned char rdtmf=this->REG[this->RDTMF].load(std::memory_order_relaxed)&0x0F;
					const unsigned short lf[4]={line_DTMF_697Hz,line_DTMF_770Hz,line_DTMF_852Hz,line_DTMF_941Hz};
					const unsigned short hf[4]={line_DTMF_1209Hz,line_DTMF_1336Hz,line_DTMF_1477Hz,line_DTMF_1633Hz};
					if (rptf==0||rptf==0x08) nTx_d|=lf[rdtmf&0x03];
					if (rptf==0||rptf==0x04) nTx_d|=hf[rdtmf>>2];
				}
			}
			if (this->Tx_din!=nTx_d){
				this->Tx_din=nTx_d;
				//this->updateRx();
				//////////////////////////////
			}
			unsigned char rate=this->REG[this->RATE].load(std::memory_order_relaxed)&0x0F;
			if ((rate==0x0E||rate==0x0F)&&this->Tx_dout!=0){
				this->Tx_dout=0;
				////////////////////////////////
			}
			else if (this->Tx_dout!=this->Tx_din){
				this->Tx_dout=this->Tx_din;
				////////////////////////////////
			}
		}
		
		void updateRx(){
			bool channel=(bool)(this->REG[this->RPROG].load(std::memory_order_relaxed)&0x04);
			unsigned char rprf=this->REG[this->RPRF].load(std::memory_order_relaxed)&0x0F;
			unsigned short rx=0;
			if ((rprf&0x03)==0x03){
				rx=this->Tx_din;
			}
			else{
				rx=this->Rx_dout;
			}
			const unsigned short MC_filter=line_DTMF_1209Hz|line_v23_1200bps_1|line_DTMF_1336Hz|line_DTMF_1477Hz|line_DTMF_1633Hz|line_v23_1200bps_0;//1kHz-2.5kHz
			const unsigned short BC_filter=line_v23_75bps_1|line_call_progress_tone|line_v23_75bps_0;//wide: 300Hz-600Hz / narrow: 350Hz-500Hz
			unsigned char rprx=this->REG[this->RPRX].load(std::memory_order_relaxed)&0x0F;
			if (!(bool)(rprf&0x04)){
				//filter
				
			}
			//TODO: delay nDCD, ZCO
		}
	
};
#endif