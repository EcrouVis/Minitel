#ifndef TS7514_H
#define TS7514_H
#include <functional>
#include <cstdio>
#include <atomic>
#include <cmath>
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
		
		std::function<void(unsigned char)> debug_cmd=[](unsigned char b){};
		
		void Reset(){
			this->REG[this->RPROG].store(0,std::memory_order_relaxed);
			this->REG[this->RATE].store(0x0F,std::memory_order_relaxed);
			this->REG[this->RWLO].store(0x0C,std::memory_order_relaxed);
			this->REG[this->RPRF].store(0x01,std::memory_order_relaxed);
			this->REG[this->RPTF].store(0x00,std::memory_order_relaxed);
			this->REG[this->RHDL].store(0x00,std::memory_order_relaxed);
			this->REG[this->RPRX].store(0x00,std::memory_order_relaxed);
		}
		
		void MODEMnDTMFChangeIn(bool b){
			//if (b!=(this->MODEMnDTMF)) printf("MODEMnDTMF %i \n",(int)b);
			if (b&&(!this->MODEMnDTMF)&&(!this->MCnBC)) this->writeRegister();
			this->MODEMnDTMF=b;
			this->nRTS_buf=this->nRTS;
			if (this->nRTS){
				this->MODEMnDTMF_buf=this->MODEMnDTMF;
				this->updateTx();
				this->updateRx();
			}
		}
		void MCnBCChangeIn(bool b){
			//if (b!=(this->MCnBC)) printf("MCnBC %i \n",(int)b);
			if (b&&(!this->MCnBC)&&(!this->MODEMnDTMF)) this->writeRegister();
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
				if (this->MCnBC||this->MODEMnDTMF) this->updateTx();
			}
		}
		void nRTSChangeIn(bool b){
			//if (b!=(this->nRTS)) printf("nRTS %i \n",(int)b);
			if (this->MCnBC||this->MODEMnDTMF){
				this->MCnBC_buf=this->MCnBC;
				this->MODEMnDTMF_buf=this->MODEMnDTMF;
				if (b!=this->nRTS){
					this->nRTS=b;
					this->nRTS_buf=this->nRTS;
					this->updateTx();
				}
			}
			else{
				if ((!b)&&this->nRTS){
					this->input_register=(this->input_register>>1)|(this->TxD?0x80:0);
				}
				this->nRTS=b;
			}
			/*if (b){
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
			}*/
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
		void subscribeATO(std::function<void(unsigned short)> f){
			this->sendATO=f;
		}
		
		void RA2ChangeIn(unsigned short s){
			if (s!=this->Rx_dout){
				this->Rx_dout=s;
				this->updateRx();
			}
		}
		
		float getBuzzerSample(unsigned long sampleRate){
			float s=0;
			switch (this->REG[this->RWLO].load(std::memory_order_relaxed)&0x0C){
				case 0x00:
				{
					constexpr float ATx[4]={pow(10.,-4./20.),pow(10.,-14./20.),pow(10.,-25./20.),pow(10.,-36./20.)};
					s=ATx[this->REG[this->RWLO].load(std::memory_order_relaxed)&0x03]*this->getTxInSample();
					break;
				}
				case 0x04://TODO
					break;
				case 0x08:
				{
					constexpr float ABuzzer[4]={1.,0.316227766,0.1,0.028183829};
					s=(this->buzzer_clock_tick*2>sampleRate?-1.:1.)*ABuzzer[this->REG[this->RWLO].load(std::memory_order_relaxed)&0x03];
					this->buzzer_clock_tick=(this->buzzer_clock_tick+2982)%sampleRate;
					break;
				}
				//case 0x0C:
				//	break;
			}
			return s;
		}
		
		float getTxOutSample(){
			constexpr float A[16]={pow(10.,-2./20.), pow(10.,-3./20.), pow(10.,-4./20.), pow(10.,-5./20.), pow(10.,-6./20.), pow(10.,-7./20.), pow(10.,-8./20.), pow(10.,-9./20.),
								   pow(10.,-10./20.), pow(10.,-11./20.), pow(10.,-12./20.), pow(10.,-13./20.), pow(10.,-14./20.), pow(10.,-15./20.), 0, 0};
			
			float sample=this->ATxI_sample_out;
			float amp=A[this->REG[this->RATE].load(std::memory_order_relaxed)];
			sample+=amp*this->getTxInSample();
			return sample;
		}
		
		void CLKTickIn(){
			constexpr unsigned int LF=line_v23_75bps_1|line_v23_75bps_0|line_DTMF_697Hz|line_DTMF_770Hz|line_DTMF_852Hz|line_DTMF_941Hz;
			constexpr unsigned int HF=line_DTMF_1209Hz|line_v23_1200bps_1|line_DTMF_1336Hz|line_DTMF_1477Hz|line_DTMF_1633Hz|line_v23_1200bps_0;
			
			this->resample_clk_div++;
			if (this->resample_clk_div>=12){
				this->resample_clk_div=0;
				this->ATxIFilterUpdate();
				if ((bool)(this->Tx_din&LF)){
					switch (this->Tx_din&LF){
						case line_v23_75bps_1: this->fgen1_phase+=390;break;
						case line_v23_75bps_0: this->fgen1_phase+=450;break;
						case line_DTMF_697Hz: this->fgen1_phase+=697;break;
						case line_DTMF_770Hz: this->fgen1_phase+=770;break;
						case line_DTMF_852Hz: this->fgen1_phase+=852;break;
						case line_DTMF_941Hz: this->fgen1_phase+=941;break;
					}
					if (this->fgen1_phase>=1228800) this->fgen1_phase-=1228800;
				}
				else this->fgen1_phase=0;
				if ((bool)(this->Tx_din&HF)){//DTMF HF+v23 1200Baud
					switch (this->Tx_din&HF){
						case line_DTMF_1209Hz: this->fgen2_phase+=1209;break;
						case line_v23_1200bps_1: this->fgen2_phase+=1300;break;
						case line_DTMF_1336Hz: this->fgen2_phase+=1336;break;
						case line_DTMF_1477Hz: this->fgen2_phase+=1477;break;
						case line_DTMF_1633Hz: this->fgen2_phase+=1633;break;
						case line_v23_1200bps_0: this->fgen2_phase+=2100;break;
					}
					if (this->fgen2_phase>=1228800) this->fgen2_phase-=1228800;
				}
				else this->fgen2_phase=0;
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
		unsigned short Rx_din;
		
		unsigned int buzzer_clock_tick=0;
		
		unsigned int fgen1_phase=0;
		unsigned int fgen2_phase=0;
		
		std::function<void(bool)> sendRxD=[](bool b){};
		std::function<void(bool)> sendWLO=[](bool b){};
		std::function<void(bool)> sendnDCD=[](bool b){};
		std::function<void(unsigned short)> sendATO=[](unsigned short b){};
		
		unsigned char resample_clk_div=0;
		float ATxI_mean_value=0;
		float ATxI_sample_out=0;
		float ATxIFilterBuffer[6];
		
		void ATxIFilterUpdate(){//TODO: parallel implementation + SIMD?
			float x1,x2;
			if ((this->REG[this->RPTF].load(std::memory_order_relaxed)&0x0C)==0x00){//ATxI selected
				if ((this->REG[this->RPTF].load(std::memory_order_relaxed)&0x0E)==0x02){//low pass filter selected
					//Chebyshev type II 6th order low pass filter (biquad filter transposed direct form 2, cutoff frequency: 6000Hz, sampling frequency: 1228800Hz, attenuation: 55dB)
					x1=0.0017234474663048761*(this->ATxI?1:-1);
					
					x2=x1+this->ATxIFilterBuffer[0];
					this->ATxIFilterBuffer[0]=this->ATxIFilterBuffer[1]-1.9859959913572462*x1+1.9609169919559772*x2;
					this->ATxIFilterBuffer[1]=x1-0.9613383234530147*x2;
					
					x1=x2+this->ATxIFilterBuffer[2];
					this->ATxIFilterBuffer[2]=this->ATxIFilterBuffer[3]-1.9981181127959642*x2+1.975834493024344*x1;
					this->ATxIFilterBuffer[3]=x2-0.9761889637026712*x1;
					
					x2=x1+this->ATxIFilterBuffer[4];
					this->ATxIFilterBuffer[4]=this->ATxIFilterBuffer[5]-1.998991279436035*x1+1.9921506528893136*x2;
					this->ATxIFilterBuffer[5]=x1-0.9924574213033137*x2;
				}
				else{
					//Chebyshev type II 6th order low pass filter (biquad filter transposed direct form 2, cutoff frequency: 22000Hz, sampling frequency: 1228800Hz, attenuation: 55dB)
					x1=0.0016506006830212339*(this->ATxI?1:-1);
					
					x2=x1+this->ATxIFilterBuffer[0];
					this->ATxIFilterBuffer[0]=this->ATxIFilterBuffer[1]-1.8192473545858276*x1+1.8598449760063778*x2;
					this->ATxIFilterBuffer[1]=x1-0.8652353024330538*x2;
					
					x1=x2+this->ATxIFilterBuffer[2];
					this->ATxIFilterBuffer[2]=this->ATxIFilterBuffer[3]-1.9747974586511543*x2+1.9107679201003114*x1;
					this->ATxIFilterBuffer[3]=x2-0.9153908270370111*x1;
					
					x2=x1+this->ATxIFilterBuffer[4];
					this->ATxIFilterBuffer[4]=this->ATxIFilterBuffer[5]-1.986454389602778*x1+1.9685314457776844*x2;
					this->ATxIFilterBuffer[5]=x1-0.9726187727577423*x2;
				}
				
				//simple exponential smoothing to remove signal <300Hz / constant
				float alpha=(this->REG[this->RPTF].load(std::memory_order_relaxed)==0x03)?250./1228800.:50./1228800.;//alpha=1-exp(-dt/tau) ~ dt/tau (exponential smoothing)
				this->ATxI_sample_out=x2-this->ATxI_mean_value;
				this->ATxI_mean_value+=alpha*this->ATxI_sample_out;
			}
		}
		
		float getTxInSample(){
			constexpr unsigned int LF=line_v23_75bps_1|line_v23_75bps_0|line_DTMF_697Hz|line_DTMF_770Hz|line_DTMF_852Hz|line_DTMF_941Hz;
			constexpr unsigned int HF=line_DTMF_1209Hz|line_v23_1200bps_1|line_DTMF_1336Hz|line_DTMF_1477Hz|line_DTMF_1633Hz|line_v23_1200bps_0;
			
			float sample=0;
			if ((bool)(this->Tx_din&LF)){//DTMF LF+v23 75Baud
				if ((bool)(this->Tx_din&(line_v23_75bps_1|line_v23_75bps_0))){
					sample+=sin(2*M_PI*this->fgen1_phase/1228800.);
				}
				else{
					sample+=pow(10.,-6./20.)*sin(2*M_PI*this->fgen1_phase/1228800.);
				}
			}
			if ((bool)(this->Tx_din&HF)){//DTMF HF+v23 1200Baud
				if ((bool)(this->Tx_din&(line_v23_1200bps_1|line_v23_1200bps_0))){
					sample+=sin(2*M_PI*this->fgen2_phase/1228800.);
				}
				else{
					sample+=pow(10.,-4./20.)*sin(2*M_PI*this->fgen2_phase/1228800.);
				}
			}
			return sample;
		}
		
		void writeRegister(){
			unsigned char n_reg=(this->input_register>>4)&0x07;
			unsigned char rprog=this->REG[this->RPROG].load(std::memory_order_relaxed);
			bool cmd_ok=(((rprog&3)==0)||
						 ((rprog&3)==3)||
						 ((rprog&3)==1&&(this->input_register&0x80)==0)||
						 ((rprog&3)==2&&(this->input_register&0x80)==0x80)
						)&&((rprog&0x08)==0);
			//printf("TS7514 CMD 0x%02X ",this->input_register);
			if (n_reg==this->RPROG||cmd_ok){
				switch (n_reg){
					case this->RPTF:
						if ((!(bool)(this->input_register&0x0C))&&((bool)(this->REG[this->RPTF].load(std::memory_order_relaxed)&0x0C))){//try to avoid poping when selecting ATxI
							this->ATxI_sample_out=0;
							this->ATxI_mean_value=0;
							for (unsigned int i=0;i<sizeof(this->ATxIFilterBuffer)/sizeof(this->ATxIFilterBuffer[0]);i++) this->ATxIFilterBuffer[i]=0;
						}
						break;
				}
				this->REG[n_reg].store(this->input_register&0x0F,std::memory_order_relaxed);
				//printf("OK\n");
				this->updateTx();
				this->updateRx();
				this->debug_cmd(this->input_register);
				
			}
			/*else{
				printf("NOK\n");
			}*/
		}
		
		void updateTx(){
			unsigned short nTx_d=0;
			if (!this->nRTS_buf){
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
					unsigned char rptf=this->REG[this->RPTF].load(std::memory_order_relaxed);
					unsigned char rdtmf=this->REG[this->RDTMF].load(std::memory_order_relaxed);
					const unsigned short lf[4]={line_DTMF_697Hz,line_DTMF_770Hz,line_DTMF_852Hz,line_DTMF_941Hz};
					const unsigned short hf[4]={line_DTMF_1209Hz,line_DTMF_1336Hz,line_DTMF_1477Hz,line_DTMF_1633Hz};
					if (rptf==0||rptf==0x08) nTx_d|=lf[rdtmf&0x03];
					if (rptf==0||rptf==0x04) nTx_d|=hf[rdtmf>>2];
				}
			}
			if (this->Tx_din!=nTx_d){
				this->Tx_din=nTx_d;
				this->updateRx();
				//////////////////////////////
			}
			unsigned char rate=this->REG[this->RATE].load(std::memory_order_relaxed);
			if ((rate==0x0E||rate==0x0F)&&this->Tx_dout!=0){
				this->Tx_dout=0;
				this->sendATO(this->Tx_dout);
			}
			else if (this->Tx_dout!=this->Tx_din){
				this->Tx_dout=this->Tx_din;
				this->sendATO(this->Tx_dout);
			}
		}
		
		void updateRx(){
			bool channel=(bool)(this->REG[this->RPROG].load(std::memory_order_relaxed)&0x04);
			unsigned char rprf=this->REG[this->RPRF].load(std::memory_order_relaxed);
			unsigned short rx=0;
			if ((rprf&0x03)==0x03){
				rx=this->Tx_din;
			}
			else{
				rx=this->Rx_dout;
			}
			const unsigned short MC_filter=line_DTMF_1209Hz|line_v23_1200bps_1|line_DTMF_1336Hz|line_DTMF_1477Hz|line_DTMF_1633Hz|line_v23_1200bps_0;//1kHz-2.5kHz
			const unsigned short BC_filter=line_v23_75bps_1|line_call_progress_tone|line_v23_75bps_0;//wide: 300Hz-600Hz / narrow: 350Hz-500Hz
			unsigned char rprx=this->REG[this->RPRX].load(std::memory_order_relaxed);
			if (!(bool)(rprf&0x04)){
				if (channel) rx&=(this->MCnBC_buf)?MC_filter:BC_filter;
				else rx&=(this->MCnBC_buf)?BC_filter:MC_filter;
			}
			if (this->Rx_din!=rx){
				printf("Rx %04X / MCnBC %i\n",rx,this->MCnBC_buf);
				this->Rx_din=rx;
				this->sendnDCD(!((bool)rx));//TODO: delay
				this->sendRxD((bool)(rx&(line_v23_75bps_1|line_v23_1200bps_1)));
				//ZCO not implemented
			}
		}
	
};
#endif