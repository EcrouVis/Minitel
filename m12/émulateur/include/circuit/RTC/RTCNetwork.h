#ifndef RTCNETWORK_H
#define RTCNETWORK_H
#include <functional>
#include <cmath>
#include "circuit/PhoneLine.h"
#include "encoding.h"
enum PhoneNumberState{
	NOT_PHONE_NUMBER=0,
	PHONE_NUMBER_ONGOING=1,
	PHONE_NUMBER_FINISHED=2
};
class RTCService{
	public:
		virtual enum PhoneNumberState isCalled(std::vector<unsigned char>* pPhoneNumber)=0;
		std::function<void(unsigned short)> sendPhoneLine=[](unsigned short s){};
		virtual void phoneLineChangeIn(unsigned short s){};
		virtual void CLKTickIn9600Hz(){};
		
};
class RTCNetwork{
	public:
		
		bool requestPhoneLine(RTCService* srv){
			if (this->currentState!=this->IDLE) return false;
			this->currentState=this->CALL_INCOMING;
			printf("CALL_INCOMING\n");
			this->timer=0;
			this->ServiceLinked=srv;
			this->ServiceLinked->sendPhoneLine=this->sendPhoneLine;
			return true;
		}
		
		void phoneLineChangeIn(unsigned short state){
			if (this->phoneLineStateIn==state) return;
			if ((bool)(state&line_Closed)){
				switch (this->currentState){
					case this->IDLE:
						this->phoneLineStateIn=state;
						this->currentState=this->WAIT_PHONE_NUMBER;
						printf("WAIT_PHONE_NUMBER\n");
						this->timer=0;
						this->phoneLineStateOut=line_call_progress_tone;
						this->sendPhoneLine(this->phoneLineStateOut);
						break;
					case this->WAIT_PHONE_NUMBER:
						this->phoneLineStateIn=state;
						if ((bool)(state&line_DTMF)){
							this->currentState=this->PHONE_DIALING;
							printf("PHONE_DIALING\n");
							this->timer=0;
							this->phoneLineStateOut=0;
							this->sendPhoneLine(this->phoneLineStateOut);
							this->phoneNumber.clear();
						}
						break;
					case this->PHONE_DIALING:
						if ((bool)((state^this->phoneLineStateIn)&line_DTMF)){
							this->phoneLineStateIn=state;
							this->timer=0;
						}
						else this->phoneLineStateIn=state;
						break;
					case this->CONNECTED:
						this->phoneLineStateIn=state;
						this->ServiceLinked->phoneLineChangeIn(state);
						break;
					case this->WAIT_CALL_END:
						this->phoneLineStateIn=state;
						break;
					case this->CALL_INCOMING:
						this->phoneLineStateIn=state;
						this->currentState=this->CONNECTED;
						printf("CONNECTED\n");
						this->timer=0;
						this->phoneLineStateOut=0;
						this->sendPhoneLine(this->phoneLineStateOut);
						break;
				}
			}
			else{
				this->phoneLineStateIn=state;
				if (this->currentState!=this->CALL_INCOMING){
					this->currentState=this->IDLE;
					printf("IDLE\n");
					this->timer=0;
					this->phoneLineStateOut=0;
					this->sendPhoneLine(this->phoneLineStateOut);
					
					if (this->ServiceLinked!=NULL){
						this->ServiceLinked->sendPhoneLine=[](unsigned short s){};
						this->ServiceLinked->phoneLineChangeIn(0);
						this->ServiceLinked=NULL;
					}
				}
			}
		}
		
		void CLKTickIn600Hz(){
			//phone dialing timeout
			if (this->currentState==this->WAIT_PHONE_NUMBER){
				this->timer++;
				if (this->timer>=this->wait_phone_number_timeout){
					this->timer=0;
					this->currentState=this->WAIT_CALL_END;
					printf("WAIT_CALL_END\n");
				}
			}
			else if (this->currentState==this->PHONE_DIALING){
				this->timer++;
				if (timer==18){
					switch (this->phoneLineStateIn&line_DTMF){
						case 0:
							if (this->pendingNumber<16){
								this->phoneNumber.push_back(this->pendingNumber);
								//test phone numbers
								{
									unsigned char pnsg=NOT_PHONE_NUMBER;
									for (auto serv: this->RTCServices){
										enum PhoneNumberState pns=serv->isCalled(&(this->phoneNumber));
										if (pns==PHONE_NUMBER_FINISHED){
											this->ServiceLinked=serv;
											this->ServiceLinked->sendPhoneLine=this->sendPhoneLine;
											this->ServiceLinked->phoneLineChangeIn(this->phoneLineStateIn);
										}
										pnsg|=pns;
									}
									if (pnsg==NOT_PHONE_NUMBER){
										this->timer=0;
										this->currentState=this->WAIT_CALL_END;
										printf("WAIT_CALL_END\n");
									}
									else if (pnsg==PHONE_NUMBER_FINISHED){
										this->timer=0;
										this->currentState=this->CONNECTED;
										printf("CONNECTED\n");
									}
								}
							}
							else{
								this->timer=0;
								this->currentState=this->WAIT_CALL_END;
								printf("WAIT_CALL_END\n");
							}
							break;
						case line_DTMF_0:this->pendingNumber=0;break;
						case line_DTMF_1:this->pendingNumber=1;break;
						case line_DTMF_2:this->pendingNumber=2;break;
						case line_DTMF_3:this->pendingNumber=3;break;
						case line_DTMF_4:this->pendingNumber=4;break;
						case line_DTMF_5:this->pendingNumber=5;break;
						case line_DTMF_6:this->pendingNumber=6;break;
						case line_DTMF_7:this->pendingNumber=7;break;
						case line_DTMF_8:this->pendingNumber=8;break;
						case line_DTMF_9:this->pendingNumber=9;break;
						case line_DTMF_S:this->pendingNumber=14;break;
						case line_DTMF_H:this->pendingNumber=15;break;
						default:this->pendingNumber=0xFF;break;
					}
				}
				if (this->timer>=this->dialing_timeout&&!(bool)(this->phoneLineStateIn&line_DTMF)){
					this->timer=0;
					this->currentState=this->WAIT_CALL_END;
					printf("WAIT_CALL_END\n");
				}
			}
			else if (this->currentState==this->WAIT_CALL_END){
				if (this->timer<this->wait_call_end_signal_length){
					this->timer++;
					if ((bool)((this->timer/this->wait_call_end_step_length)&1)){
						if (this->phoneLineStateOut!=line_call_progress_tone){
							this->phoneLineStateOut=line_call_progress_tone;
							this->sendPhoneLine(this->phoneLineStateOut);
						}
					}
					else{
						if (this->phoneLineStateOut!=0){
							this->phoneLineStateOut=0;
							this->sendPhoneLine(this->phoneLineStateOut);
						}
					}
				}
				else if ((bool)this->phoneLineStateOut){
					this->phoneLineStateOut=0;
					this->sendPhoneLine(this->phoneLineStateOut);
				}
			}
			else if (this->currentState==this->CALL_INCOMING){
				if (this->timer<this->call_incoming_timeout){
					this->timer++;
					if ((this->timer%this->call_incoming_retry)>this->call_incoming_ringing_stop){
						if (this->phoneLineStateOut!=0){
							this->phoneLineStateOut=0;
							this->sendPhoneLine(this->phoneLineStateOut);
						}
					}
					else{
						if (this->phoneLineStateOut!=line_Ringing){
							this->phoneLineStateOut=line_Ringing;
							this->sendPhoneLine(this->phoneLineStateOut);
						}
					}
				}
				else{
					this->currentState=this->IDLE;
					this->timer=0;
					if (this->phoneLineStateOut!=0){
						this->phoneLineStateOut=0;
						this->sendPhoneLine(this->phoneLineStateOut);
					}
				}
			}
		}
		void subscribePhoneLine(std::function<void(unsigned short)> f){
			this->sendPhoneLine=f;
		}
		void subscribeService(RTCService* rtcs){
			this->RTCServices.push_back(rtcs);
		}
		
		float getPhoneLineSample(unsigned long sampleRate){
			if ((bool)(this->phoneLineStateOut&line_Ringing)) this->sample_phase+=50;
			else if ((bool)(this->phoneLineStateOut&line_call_progress_tone)) this->sample_phase+=440;
			else{
				this->sample_phase=0;
				return 0;
			}
			if (this->sample_phase>sampleRate) this->sample_phase-=sampleRate;
			return std::sin(2*M_PI*((float)this->sample_phase)/((float)sampleRate));
		}
		
	private:
		unsigned short phoneLineStateIn=0;
		unsigned short phoneLineStateOut=0;
		std::function<void(unsigned short)> sendPhoneLine=[](unsigned short d){};
		
		unsigned short timer=0;
		unsigned short call_incoming_timeout=15000;//25s
		unsigned short call_incoming_retry=3000;//5s
		unsigned short call_incoming_ringing_stop=1020;//1.7s
		unsigned short wait_phone_number_timeout=9000;//15s
		unsigned short dialing_timeout=9000;//15s
		unsigned short wait_call_end_signal_length=720;//1.2s
		unsigned short wait_call_end_step_length=120;//0.2s
		
		unsigned char pendingNumber=0xFF;
		std::vector<unsigned char> phoneNumber;
		
		std::vector<RTCService*> RTCServices;
		RTCService* ServiceLinked;
		
		unsigned long sample_phase=0;
		
		enum State{
			IDLE,
			WAIT_PHONE_NUMBER,
			PHONE_DIALING,
			CONNECTED,
			WAIT_CALL_END,
			CALL_INCOMING
		};
		enum State currentState=this->IDLE;
};

class RTCServiceMinipavi: public RTCService{
	public:
		virtual enum PhoneNumberState isCalled(std::vector<unsigned char>* pPhoneNumber) override final {
			constexpr unsigned char N[]={0,9,7,2,1,0,1,7,2,1};
			if (pPhoneNumber->size()>sizeof(N)/sizeof(N[0])) return NOT_PHONE_NUMBER;
			for (size_t i=0;i<pPhoneNumber->size();i++){
				if (N[i]!=(*pPhoneNumber)[i]) return NOT_PHONE_NUMBER;
			}
			if (pPhoneNumber->size()==sizeof(N)/sizeof(N[0])) return PHONE_NUMBER_FINISHED;
			return PHONE_NUMBER_ONGOING;
		}
		
		virtual void phoneLineChangeIn(unsigned short s) final override{
			//printf("minipavi: %04X\n",s);
			if (((bool)(s&line_Closed))!=this->selected){
				this->selected=!this->selected;
				if (this->selected){
					this->sendPhoneLine(this->OPPO?line_v23_75bps_1:line_v23_1200bps_1);
				}
				else{
					
				}
			}
			this->phoneLineStateIn=s;
		};
		
		virtual void CLKTickIn9600Hz() final override {
			this->Clk++;
			if ((this->Clk&ClkDivMask75)==0){
				if (this->OPPO) this->TxClkTick();
				else this->RxClkTick();
			}
			if ((this->Clk&ClkDivMask1200)==0){
				if (this->OPPO) this->RxClkTick();
				else this->TxClkTick();
			}
		}
	private:
		bool selected=false;
		unsigned short phoneLineStateIn=0;
		unsigned char Clk;
		bool OPPO=true;
		constexpr static unsigned char ClkDivMask75=0x7F;
		constexpr static unsigned char ClkDivMask1200=0x07;
		unsigned char RxState=0;
		unsigned char RxBuf=0;
		std::vector<unsigned char> Rxq;
		unsigned char TxState=0;
		unsigned char TxBuf=0;
		std::queue<unsigned char> Txq;
		constexpr static char test[]="\x1B\x20\x20\x31"
									 "\x1B\x20\x21\x31""Exp\x19""Bediteur"
									 "\x1B\x20\x22\x31""Nom"
									 "\x1B\x20\x23\x31""0123456789"
									 "\x1B\x20\x24\x31""Destinataire"
									 "\x1B\x20\x25\x31""Objet"
									 "\x1B\x20\x26\x31""Corps"
									 "\x1B\x21\x20\x31";
									 
		bool PCE=false;
		unsigned char PCEBuffer[15]={0};
		unsigned char PCEIndex=0;
		unsigned char PCECRC=0;
		constexpr static unsigned char PCECRCG=0x09<<1;
		
		enum Const{
			
			NOT_CMD=0x00,
			CMD_ONGOING=0x01,
			CMD_FINISHED=0x02
		};
		
		void TxClkTick(){
			unsigned short logical_0=this->OPPO?line_v23_75bps_0:line_v23_1200bps_0;
			unsigned short logical_1=this->OPPO?line_v23_75bps_1:line_v23_1200bps_1;
			switch (this->TxState){
				case 0:
					if ((bool)this->Txq.size()){
						this->sendPhoneLine(logical_0);
						this->TxBuf=this->Txq.front();
						this->Txq.pop();
						constexpr unsigned short P=0b0110100110010110;
						this->TxBuf^=((P>>(this->TxBuf&0x0F))^(P>>(this->TxBuf>>4)))<<7;
						this->TxState=1;
					}
					break;
				default:
					this->sendPhoneLine(((bool)(this->TxBuf&0x01))?logical_1:logical_0);
					this->TxBuf=this->TxBuf>>1;
					this->TxState++;
					break;
				case 9:
					this->sendPhoneLine(logical_1);
					this->TxState=0;
					break;
			}
		}
		
		void RxClkTick(){
			unsigned short logical_0=this->OPPO?line_v23_1200bps_0:line_v23_75bps_0;
			unsigned short logical_1=this->OPPO?line_v23_1200bps_1:line_v23_75bps_1;
			switch (this->RxState){
				case 0:
					if ((bool)(this->phoneLineStateIn&logical_0))this->RxState=1;
					break;
				default:
					this->RxBuf=(this->RxBuf>>1)|(((bool)(this->phoneLineStateIn&logical_0))?0:0x80);
					this->RxState++;
					break;
				case 9:
					printf("rx %02X\n",this->RxBuf);
					if (this->PCE) this->PCEUpdate(this->RxBuf);
					this->Rxq.push_back(this->RxBuf&0x7F);
					if ((bool)(this->phoneLineStateIn&logical_1)) this->RxState=0;
					else this->RxState=10;
					this->RxUpdate();
					break;
				case 10:
					if ((bool)(this->phoneLineStateIn&logical_1)) this->RxState=0;
					break;
			}
		}
		
		void RxUpdate(){
			unsigned char cmd=this->NOT_CMD;
			cmd|=this->isISO2022CMD();
			if (cmd==this->CMD_FINISHED){
				if (this->Rxq.size()==6&&this->Rxq[1]==0x23&&this->Rxq[2]==0x20&&this->Rxq[3]==0x2C&&this->Rxq[4]==0x21&&this->Rxq[5]==0x3C){//IRD
					constexpr char DC[]="\x1B\x23\x20\x2C\x21\x38";
					for (size_t i=0;i<sizeof(DC)/sizeof(DC[0]);i++)this->Txq.push((unsigned char)DC[i]);
				}
				if (this->Rxq.size()==5&&this->Rxq[1]==0x20&&this->Rxq[2]==0x2C&&this->Rxq[3]==0x21&&this->Rxq[4]==0x39){//AC
					constexpr char STUTEL1[]="\x1F"">D~S#~Q~R ~YE~QMQ~Q~QL~Q~X\x0D";
					for (size_t i=0;i<sizeof(STUTEL1)/sizeof(STUTEL1[0]);i++)this->Txq.push((unsigned char)STUTEL1[i]);
					/*for (size_t i=0;i<sizeof(this->test)/sizeof(this->test[0]);i++)this->Txq.push((unsigned char)this->test[i]);
					constexpr char ILC[]="\x1B\x20\x2C\x21\x3A";
					for (size_t i=0;i<sizeof(ILC)/sizeof(ILC[0]);i++)this->Txq.push((unsigned char)ILC[i]);*/
				}
				if (this->Rxq.size()==5&&this->Rxq[1]==0x20&&this->Rxq[2]==0x2C&&this->Rxq[3]==0x21&&this->Rxq[4]==0x38){//DC
					constexpr char AC[]="\x1B\x20\x2C\x21\x39";
					for (size_t i=0;i<sizeof(AC)/sizeof(AC[0]);i++)this->Txq.push((unsigned char)AC[i]);
				}
			}
			if (cmd==NOT_CMD){
				cmd|=this->isACKCMD();
				if (cmd==this->CMD_FINISHED&&this->Rxq[1]==0x53){
					constexpr char IRD[]="\x1B\x23\x20\x2C\x21\x3C";
					for (size_t i=0;i<sizeof(IRD)/sizeof(IRD[0]);i++)this->Txq.push((unsigned char)IRD[i]);
				}
			}
			if (cmd==NOT_CMD){
				cmd|=this->isSTUTELCMD();
				if (cmd==this->CMD_FINISHED){
					if ((this->Rxq[2]&0x0C)==0x04){
						constexpr char STUTEL2[]="\x1F"">P2\x0D";
						for (size_t i=0;i<sizeof(STUTEL2)/sizeof(STUTEL2[0]);i++)this->Txq.push((unsigned char)STUTEL2[i]);
					}
					if ((this->Rxq[2]&0x03)==0x00){
						std::vector<unsigned char> data(this->Rxq.begin()+2,this->Rxq.end()-1);
						std::vector<unsigned char>* data2=DProtocolTranslationMode4Decode(&data);
						if (data2!=NULL){
							printf("\nData: ");
							for (size_t i=0;i<data2->size();i++){
								if ((*data2)[i]<0x20||(*data2)[i]>=0x7F) printf("\\x%02X",(*data2)[i]);
								else if ((*data2)[i]=='\\') printf("\\\\");
								else printf("%c",(*data2)[i]);
							}
							printf("\n");
							delete data2;
						}
					}
				}
			}
			if (cmd==NOT_CMD){
				cmd|=this->isPRO2CMD();
				if (cmd==this->CMD_FINISHED){
					if (this->Rxq[2]==0x69&&this->Rxq[3]==0x44){//PCE
						constexpr char PCE[]="\x1B\x3A\x73\x44";
						for (size_t i=0;i<sizeof(PCE)/sizeof(PCE[0]);i++)this->Txq.push((unsigned char)PCE[i]);
						printf("PCE!\n");
						this->PCE=true;
						this->PCEIndex=0;
					}
				}
			}
			if (cmd==this->NOT_CMD||cmd==this->CMD_FINISHED) this->Rxq.clear();
			
		}
		
		unsigned char isISO2022CMD(){
			switch(this->Rxq.size()){
				case 1:
					if (this->Rxq[0]==0x1B) return this->CMD_ONGOING;
					break;
				case 2:
					if (this->Rxq[0]==0x1B&&(this->Rxq[1]&0xF0)==0x20) return this->CMD_ONGOING;
					break;
				default:
					if (this->Rxq[0]==0x1B){
						for (size_t i=1;i<this->Rxq.size()-1;i++){
							if ((this->Rxq[i]&0xF0)!=0x20) return this->NOT_CMD;
						}
						switch (this->Rxq[this->Rxq.size()-1]){
							case 0x20 ... 0x2F:return this->CMD_ONGOING;
							case 0x30 ... 0x7E:return this->CMD_FINISHED;
						}
					}
					break;
			}
			return this->NOT_CMD;
		}
		
		unsigned char isACKCMD(){
			if (this->Rxq.size()<2){
				if (this->Rxq[0]==0x13){
					return this->CMD_ONGOING;
				}
			}
			else if (this->Rxq.size()==2){
				if (this->Rxq[0]==0x13){
					return this->CMD_FINISHED;
				}
			}
			return this->NOT_CMD;
		}
		
		unsigned char isSTUTELCMD(){//D-Protocol mode C with translation mode 4
			switch (this->Rxq.size()){
				case 0:break;
				case 1:
					if (this->Rxq[0]==0x1F) return this->CMD_ONGOING;
					break;
				case 2:
					if (this->Rxq[0]==0x1F&&this->Rxq[1]==0x3E) return this->CMD_ONGOING;
					break;
				default:
					if (this->Rxq[0]==0x1F&&this->Rxq[1]==0x3E){
						if (this->Rxq[this->Rxq.size()-1]==0x0D) return this->CMD_FINISHED;
						else return this->CMD_ONGOING;
					}
					break;
			}
			return this->NOT_CMD;
		}
		
		unsigned char isPRO2CMD(){
			constexpr unsigned char CMD[2]={0x1B,0x3A};
			if (this->Rxq.size()<4){
				if (!(bool)memcmp(this->Rxq.data(),CMD,(this->Rxq.size()<2?this->Rxq.size():2)*sizeof(unsigned char))){
					return this->CMD_ONGOING;
				}
			}
			else if (this->Rxq.size()==4){
				if (!(bool)memcmp(this->Rxq.data(),CMD,2*sizeof(unsigned char))){
					return this->CMD_FINISHED;
				}
			}
			return this->NOT_CMD;
		}
		
		void PCEUpdate(unsigned char d){
			if (this->PCEIndex>=16){
				this->PCEIndex=0;
			}
			else if (this->PCEIndex==15){
				printf("CRC: %02X / V: %02X\n",this->PCECRC>>1,d&0x7F);
				this->PCEIndex++;
			}
			else{
				if (this->PCEIndex==0){
					this->PCECRC=0;
					printf("PCE block\n");
				}
				this->PCEBuffer[this->PCEIndex++]=d;
				this->PCECRC^=d;
				for (int i=0;i<8;i++){
					this->PCECRC=((this->PCECRC&0x80)?this->PCECRCG:0)^(this->PCECRC<<1);
				}
			}
		}
};
#endif