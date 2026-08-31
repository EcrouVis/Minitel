#ifndef RTCNETWORK_H
#define RTCNETWORK_H
#include <functional>
#include <cmath>
#ifndef M_PI
#define M_PI (3.14159265358979323846264338327950288)
#endif
#include "circuit/PhoneLine.h"
#include "encoding.h"
#include <mutex>
#include <ixwebsocket/IXWebSocket.h>

#include "desktop/thread_affinity.h"

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
		
		virtual float getPhoneLineSample(unsigned long sr){return 0;};
		virtual void setPhoneLineSample(float s){};
		
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
					case IDLE:
						this->phoneLineStateIn=state;
						this->currentState=this->WAIT_PHONE_NUMBER;
						printf("WAIT_PHONE_NUMBER\n");
						this->timer=0;
						this->phoneLineStateOut=line_call_progress_tone;
						this->sendPhoneLine(this->phoneLineStateOut);
						break;
					case WAIT_PHONE_NUMBER:
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
					case PHONE_DIALING:
						if ((bool)((state^this->phoneLineStateIn)&line_DTMF)){
							this->phoneLineStateIn=state;
							this->timer=0;
						}
						else this->phoneLineStateIn=state;
						break;
					case CONNECTED:
						this->phoneLineStateIn=state;
						this->ServiceLinked->phoneLineChangeIn(state);
						break;
					case WAIT_CALL_END:
						this->phoneLineStateIn=state;
						break;
					case CALL_INCOMING:
						this->phoneLineStateIn=state;
						this->currentState=this->CONNECTED;
						printf("CONNECTED\n");
						this->timer=0;
						this->phoneLineStateOut=0;
						this->sendPhoneLine(this->phoneLineStateOut);
						if (this->ServiceLinked!=NULL) this->ServiceLinked->phoneLineChangeIn(this->phoneLineStateIn);
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
		
		void CLKTickIn9600Hz(){
			if (this->ServiceLinked!=NULL) this->ServiceLinked->CLKTickIn9600Hz();
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
		
		void generatePhoneLineSample(unsigned long sampleRate){
			if (this->ServiceLinked!=NULL){
				this->phoneLineSampleOut=this->ServiceLinked->getPhoneLineSample(sampleRate);
			}
			else{
				if ((bool)(this->phoneLineStateOut&line_Ringing)) this->sample_phase+=50;
				else if ((bool)(this->phoneLineStateOut&line_call_progress_tone)) this->sample_phase+=440;
				else{
					this->sample_phase=0;
					this->phoneLineSampleOut=0;
				}
				if (this->sample_phase>sampleRate) this->sample_phase-=sampleRate;
				this->phoneLineSampleOut=std::sin(2*M_PI*((float)this->sample_phase)/((float)sampleRate));
			}
		}
		
		float getPhoneLineSample(){
			return this->phoneLineSampleOut;
		}
		
		void setPhoneLineSample(float s){
			if (this->ServiceLinked!=NULL && (bool)(this->phoneLineStateIn&line_analog)){
				this->ServiceLinked->setPhoneLineSample(s);
			}
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
		float phoneLineSampleOut=0;
		
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

class RTCServiceAudio: public RTCService{
	public:
		RTCServiceAudio (std::vector<unsigned char> num):phoneNumber(num){
		}
		
		virtual enum PhoneNumberState isCalled(std::vector<unsigned char>* pPhoneNumber) override final {
			if (pPhoneNumber->size()>this->phoneNumber.size()) return NOT_PHONE_NUMBER;
			for (size_t i=0;i<pPhoneNumber->size();i++){
				if (this->phoneNumber[i]!=(*pPhoneNumber)[i]) return NOT_PHONE_NUMBER;
			}
			if (pPhoneNumber->size()==this->phoneNumber.size()) return PHONE_NUMBER_FINISHED;
			return PHONE_NUMBER_ONGOING;
		}
		
		virtual void phoneLineChangeIn(unsigned short s) final override{
			if (((bool)(s&line_Closed))!=this->selected){
				this->selected=!this->selected;
				this->sendIOState(this->selected);
				if (this->selected) this->sendPhoneLine(line_analog);
			}
		}
		virtual float getPhoneLineSample(unsigned long sr) override final{
			return this->sampleIn;
		}
		virtual void setPhoneLineSample(float s) override final{
			this->sampleOut=s-this->sampleIn;//reduce echo
		};
		float getServiceSample(){
			return this->sampleOut;
		}
		void setServiceSample(float s){
			this->sampleIn=s;
		}
		
		void subscribeIOState(std::function<void(bool)> f){
			this->sendIOState=f;
		}
	
	private:
		bool selected=false;
		
		std::function<void(bool)> sendIOState=[](bool b){};
		float sampleIn=0;
		float sampleOut=0;
		
		std::vector<unsigned char> phoneNumber;
		
};

class RTCServiceWebsocket: public RTCService{
	public:
		RTCServiceWebsocket (std::vector<unsigned char> num, const char* url):phoneNumber(num),url(url){
		}
		
		virtual enum PhoneNumberState isCalled(std::vector<unsigned char>* pPhoneNumber) override final {
			if (pPhoneNumber->size()>this->phoneNumber.size()) return NOT_PHONE_NUMBER;
			for (size_t i=0;i<pPhoneNumber->size();i++){
				if (this->phoneNumber[i]!=(*pPhoneNumber)[i]) return NOT_PHONE_NUMBER;
			}
			if (pPhoneNumber->size()==this->phoneNumber.size()) return PHONE_NUMBER_FINISHED;
			return PHONE_NUMBER_ONGOING;
		}
		
		virtual void phoneLineChangeIn(unsigned short s) final override{
			if (((bool)(s&line_Closed))!=this->selected){
				this->selected=!this->selected;
				if (this->selected){
					this->Init();
				}
				else{
					this->Reset();
				}
			}
			this->phoneLineStateIn=s;
		};
		
		virtual void CLKTickIn9600Hz() final override {
			this->Clk++;
			if ((this->Clk&ClkDivMask75)==0){
				this->RxClkTick();
			}
			if ((this->Clk&ClkDivMask1200)==0){
				this->TxClkTick();
			}
		}
		
		virtual float getPhoneLineSample(unsigned long sr) override final{
			if ((this->phoneLineStateOut&(line_v23_1200bps_0|line_v23_1200bps_1))==line_v23_1200bps_0){
				this->v23Phase+=2100;
			}
			else if ((this->phoneLineStateOut&(line_v23_1200bps_0|line_v23_1200bps_1))==line_v23_1200bps_1){
				this->v23Phase+=1300;
			}
			else{
				this->v23Phase=0;
			}
			if (this->v23Phase>sr) this->v23Phase-=sr;
			return pow(10.,-0.5)*sin(2*M_PI*((float)this->v23Phase)/((float)sr));
		}
	private:
		bool selected=false;
		unsigned short phoneLineStateIn=0;
		unsigned short phoneLineStateOut=0;
		unsigned char Clk;
		constexpr static unsigned char ClkDivMask75=0x7F;
		constexpr static unsigned char ClkDivMask1200=0x07;
		unsigned char RxState=10;
		unsigned char RxBuf=0;
		std::vector<unsigned char> qRx;
		unsigned char TxState=0;
		unsigned char TxBuf=0;
		std::queue<unsigned char> qTx;
		
		unsigned long v23Phase=0;
		
		ix::WebSocket webSocket;
		std::mutex wsMutex;
		
		std::vector<unsigned char> phoneNumber;
		const char* url;
		
		enum Const{
			
			NOT_CMD=0x00,
			CMD_ONGOING=0x01,
			CMD_FINISHED=0x02
		};
		
		void TxClkTick(){
			switch (this->TxState){
				case 0:
				{
					std::lock_guard<std::mutex> lock(this->wsMutex);
					if ((bool)this->qTx.size()&&(bool)(this->phoneLineStateIn&(line_v23_75bps_0|line_v23_75bps_1))){
						this->phoneLineStateOut=line_v23_1200bps_0;
						this->sendPhoneLine(this->phoneLineStateOut);
						this->TxBuf=this->qTx.front();
						this->qTx.pop();
						constexpr unsigned short P=0b0110100110010110;
						this->TxBuf^=((P>>(this->TxBuf&0x0F))^(P>>(this->TxBuf>>4)))<<7;
						this->TxState=1;
					}
				}
					break;
				default:
					this->phoneLineStateOut=((bool)(this->TxBuf&0x01))?line_v23_1200bps_1:line_v23_1200bps_0;
					this->sendPhoneLine(this->phoneLineStateOut);
					this->TxBuf=this->TxBuf>>1;
					this->TxState++;
					break;
				case 9:
					this->phoneLineStateOut=line_v23_1200bps_1;
					this->sendPhoneLine(this->phoneLineStateOut);
					this->TxState=0;
					break;
			}
		}
		
		void RxClkTick(){
			switch (this->RxState){
				case 0:
					if ((bool)(this->phoneLineStateIn&line_v23_75bps_0))this->RxState=1;
					else{
						this->RxWaiting();
						this->RxState=11;
					}
					break;
				case 11:
					if ((bool)(this->phoneLineStateIn&line_v23_75bps_0))this->RxState=1;
					break;
				default:
					this->RxBuf=(this->RxBuf>>1)|(((bool)(this->phoneLineStateIn&line_v23_75bps_0))?0:0x80);
					this->RxState++;
					break;
				case 9:
					this->qRx.push_back(this->RxBuf&0x7F);
					if ((bool)(this->phoneLineStateIn&line_v23_75bps_1)) this->RxState=0;
					else this->RxState=10;
					this->RxUpdate();
					break;
				case 10:
					if ((bool)(this->phoneLineStateIn&line_v23_75bps_1)) this->RxState=0;
					break;
			}
		}
		
		void RxUpdate(){
			
		}
		
		void RxWaiting(){
			if (!this->qRx.empty()){
				if (this->webSocket.getReadyState()==ix::ReadyState::Open) this->webSocket.sendBinary(this->qRx);
				this->qRx.clear();
			}
		}
		
		void Reset(){
			this->webSocket.stop();
			{
				std::lock_guard<std::mutex> lock(this->wsMutex);
				this->qRx.clear();
				std::queue<unsigned char> empty;
				std::swap(qTx,empty);
				this->TxState=0;
				this->RxState=10;
			}
		}
		
		void Init(){
			this->phoneLineStateOut=line_v23_1200bps_1;
			this->sendPhoneLine(this->phoneLineStateOut);
			
			this->webSocket.setUrl(this->url);
			this->webSocket.setPingInterval(45);
			this->webSocket.disablePerMessageDeflate();
			this->webSocket.disableAutomaticReconnection();
			ix::SocketTLSOptions tlsOptions;
			tlsOptions.caFile = "NONE";//TODO: when TLS cert error will be fix, remove this option
			this->webSocket.setTLSOptions(tlsOptions);
			this->webSocket.setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg){
				switch (msg->type){
					case ix::WebSocketMessageType::Error:
						printf("%s\n",msg->errorInfo.reason.c_str());
						break;
					case ix::WebSocketMessageType::Open:
						break;
					case ix::WebSocketMessageType::Close:
						break;
					case ix::WebSocketMessageType::Message:
						{
							std::lock_guard<std::mutex> lock(this->wsMutex);
							for (unsigned int i=0;i<msg->str.length();i++) this->qTx.push((unsigned char)msg->str[i]);
						}
						break;
					default:
						break;
				}
			});
			
			resetCurrentThreadAffinity();//reset thread affinity before creating new threads / a bit ugly but works
			this->webSocket.start();
			setCurrentThreadAffinity(getCurrentCPU());
		}
};
#endif