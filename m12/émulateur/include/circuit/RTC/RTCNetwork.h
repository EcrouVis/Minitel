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
			//printf("CALL_INCOMING\n");
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
						//printf("WAIT_PHONE_NUMBER\n");
						this->timer=0;
						this->phoneLineStateOut=line_call_progress_tone;
						this->sendPhoneLine(this->phoneLineStateOut);
						break;
					case WAIT_PHONE_NUMBER:
						this->phoneLineStateIn=state;
						if ((bool)(state&line_DTMF)){
							this->currentState=this->PHONE_DIALING;
							//printf("PHONE_DIALING\n");
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
						//printf("CONNECTED\n");
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
					//printf("IDLE\n");
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
					//printf("WAIT_CALL_END\n");
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
										//printf("WAIT_CALL_END\n");
									}
									else if (pnsg==PHONE_NUMBER_FINISHED){
										this->timer=0;
										this->currentState=this->CONNECTED;
										//printf("CONNECTED\n");
									}
								}
							}
							else{
								this->timer=0;
								this->currentState=this->WAIT_CALL_END;
								//printf("WAIT_CALL_END\n");
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
					//printf("WAIT_CALL_END\n");
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
		
		virtual void phoneLineChangeIn(unsigned short s) override final{
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

class RTCDigitalService: public RTCService{
	protected:
		virtual void Init(){};
		virtual void Uninit(){};
		virtual void CLKTick9600(){};
		virtual void RxUpdate(unsigned char d)=0;
		virtual void RxWaiting(){};
		virtual std::pair<bool,unsigned char> TxNext()=0;
		void setTxSpeed(bool fast){
			if (fast!=this->TxFast){
				this->TxFast=fast;
				this->resetSerialState();
			}
			else this->TxFast=fast;
		}
		bool getTxSpeed(){
			return this->TxFast;
		}
		virtual void RxStateChange(bool carrier){};
		
		void reset(){
			if (this->selected){
				this->Uninit();
				this->resetSerialState();
				this->Init();
			}
		}
		
	public:
		virtual void phoneLineChangeIn(unsigned short s) override final{
			if (((bool)(s&line_Closed))!=this->selected){
				this->selected=!this->selected;
				if (this->selected){
					this->resetSerialState();
					
					this->Init();
				}
				else{
					this->Uninit();
				}
			}
			unsigned short mask=this->TxFast?(line_v23_75bps_0|line_v23_75bps_1):(line_v23_1200bps_0|line_v23_1200bps_1);
			if (((bool)(s&mask))!=this->carrier){
				this->carrier=(bool)(s&mask);
				this->RxStateChange(this->carrier);
			}
			this->phoneLineStateIn=s;
		};
		
		virtual void CLKTickIn9600Hz() override final{
			this->Clk++;
			if ((this->Clk&ClkDivMask75)==0){
				if (this->TxFast) this->RxClkTick();
				else this->TxClkTick();
			}
			if ((this->Clk&ClkDivMask1200)==0){
				if (this->TxFast) this->TxClkTick();
				else this->RxClkTick();
			}
			this->CLKTick9600();
		}
		
		virtual float getPhoneLineSample(unsigned long sr) override final{
			switch (this->phoneLineStateOut&(line_v23_1200bps_0|line_v23_1200bps_1|line_v23_75bps_0|line_v23_75bps_1)){
				case line_v23_1200bps_0:this->v23Phase+=2100;break;
				case line_v23_1200bps_1:this->v23Phase+=1300;break;
				case line_v23_75bps_0:this->v23Phase+=450;break;
				case line_v23_75bps_1:this->v23Phase+=390;break;
				default:this->v23Phase=0;break;
			}
			if (this->v23Phase>sr) this->v23Phase-=sr;
			return pow(10.,-0.5)*sin(2*M_PI*((float)this->v23Phase)/((float)sr));
		}
		
	private:
		bool selected=false;
		bool TxFast=true;
		bool carrier=false;
		
		unsigned short phoneLineStateIn=0;
		unsigned short phoneLineStateOut=0;
		
		unsigned char Clk;
		constexpr static unsigned char ClkDivMask75=0x7F;
		constexpr static unsigned char ClkDivMask1200=0x07;
		
		unsigned char RxState=10;
		unsigned char RxBuf=0;
		unsigned char TxState=0;
		unsigned char TxBuf=0;
		
		unsigned long v23Phase=0;
		
		void TxClkTick(){
			unsigned short b0=this->TxFast?line_v23_1200bps_0:line_v23_75bps_0;
			unsigned short b1=this->TxFast?line_v23_1200bps_1:line_v23_75bps_1;
			switch (this->TxState){
				case 0:
				{
					std::pair<bool,unsigned char> tx=this->TxNext();//1 function to implement atomic operation (to work with multiple threads)
					if (std::get<bool>(tx)){
						this->phoneLineStateOut=b0;
						this->sendPhoneLine(this->phoneLineStateOut);
						this->TxBuf=std::get<unsigned char>(tx);
						this->TxState=1;
					}
					break;
				}
				default:
					this->phoneLineStateOut=((bool)(this->TxBuf&0x01))?b1:b0;
					this->sendPhoneLine(this->phoneLineStateOut);
					this->TxBuf=this->TxBuf>>1;
					this->TxState++;
					break;
				case 9:
					this->phoneLineStateOut=b1;
					this->sendPhoneLine(this->phoneLineStateOut);
					this->TxState=0;
					break;
			}
		}
		
		void RxClkTick(){
			unsigned short mask=this->TxFast?line_v23_75bps_0:line_v23_1200bps_0;
			switch (this->RxState){
				case 0:
					if ((bool)(this->phoneLineStateIn&mask))this->RxState=1;
					else{
						this->RxWaiting();
						this->RxState=11;
					}
					break;
				case 11:
					if ((bool)(this->phoneLineStateIn&mask))this->RxState=1;
					break;
				default:
					this->RxBuf=(this->RxBuf>>1)|(((bool)(this->phoneLineStateIn&mask))?0:0x80);
					this->RxState++;
					break;
				case 9:
					if (!(bool)(this->phoneLineStateIn&mask)) this->RxState=0;
					else this->RxState=10;
					this->RxUpdate(this->RxBuf);
					break;
				case 10:
					if (!(bool)(this->phoneLineStateIn&mask)) this->RxState=0;
					break;
			}
		}
		
		void resetSerialState(){
			this->TxState=0;
			this->RxState=10;
			this->phoneLineStateOut=this->TxFast?line_v23_1200bps_1:line_v23_75bps_1;
			this->sendPhoneLine(this->phoneLineStateOut);
		}
};


class RTCServiceWebsocket: public RTCDigitalService{
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
		
	private:
		unsigned char waitConnectionACK=0;
	
		std::vector<unsigned char> qRx;
		std::queue<unsigned char> qTx;
		
		ix::WebSocket webSocket;
		std::mutex wsMutex;
		
		std::vector<unsigned char> phoneNumber;
		const char* url;
		
		void RxUpdate(unsigned char d) override final{
			std::lock_guard<std::mutex> lock(this->wsMutex);
			constexpr unsigned short P=0b0110100110010110;
			d=((bool)(((P>>(d&0x0F))^(P>>(d>>4)))&0x01))?0x1A:d&0x7F;
			this->qRx.push_back(d);
			
			switch (this->waitConnectionACK){
				case 0:
					if (d==0x13) this->waitConnectionACK=1;
					break;
				case 1:
					if (d==0x53) this->waitConnectionACK=2;
					break;
			}
		}
		
		void RxWaiting() override final{
			if (!this->qRx.empty()){
				if (this->webSocket.getReadyState()==ix::ReadyState::Open) this->webSocket.sendBinary(this->qRx);
				this->qRx.clear();
			}
		}
		
		virtual std::pair<bool,unsigned char> TxNext() override final{
			std::pair<bool,unsigned char> d;
			if (this->waitConnectionACK<2){
				d=std::pair<bool,unsigned char>(false,0);
			}
			else{
				std::lock_guard<std::mutex> lock(this->wsMutex);
				constexpr unsigned short P=0b0110100110010110;
				if ((bool)this->qTx.size()){
					unsigned char v=this->qTx.front();
					v^=((P>>(v&0x0F))^(P>>(v>>4)))<<7;
					d=std::pair<bool,unsigned char>(true,v);
					this->qTx.pop();
				}
				else{
					d=std::pair<bool,unsigned char>(false,0);
				}
			}
			return d;
		}
		
		void Uninit() override final{
			this->webSocket.stop();
		}
		
		void Init() override final{
			this->waitConnectionACK=0;
			
			{
				std::lock_guard<std::mutex> lock(this->wsMutex);
				this->qRx.clear();
				std::queue<unsigned char> empty;
				std::swap(qTx,empty);
			}
			
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
						printf("WS: %s\n",msg->errorInfo.reason.c_str());
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


class RTCServiceMessage: public RTCDigitalService{
	public:
		virtual enum PhoneNumberState isCalled(std::vector<unsigned char>* pPhoneNumber) override final {
			if (pPhoneNumber->size()>this->phoneNumber.size()) return NOT_PHONE_NUMBER;
			for (size_t i=0;i<pPhoneNumber->size();i++){
				if (this->phoneNumber[i]!=(*pPhoneNumber)[i]) return NOT_PHONE_NUMBER;
			}
			if (pPhoneNumber->size()==this->phoneNumber.size()) return PHONE_NUMBER_FINISHED;
			return PHONE_NUMBER_ONGOING;
		}
		
	private:
		std::vector<unsigned char> phoneNumber={0,0,0,1};
		
		VideotexSplitter VDTSplitter;
		std::queue<unsigned char> qTx;
		bool TxEnable=true;
		
		bool service=false;
		
		
		
		unsigned short carrierLostTimer=0;
		unsigned short carrierFoundTimer=0;
		bool requestHighSpeed=false;
		
		virtual void CLKTick9600(){
			if (this->carrierFoundTimer!=0){
				if (this->carrierLostTimer==0) this->carrierFoundTimer=0;
				else{
					switch (this->carrierFoundTimer){
						case 384://40ms
							this->carrierFoundTimer=0;
							this->carrierLostTimer=0;
							this->requestHighSpeed=false;
							break;
					}
					if (this->carrierFoundTimer!=0) this->carrierFoundTimer++;
				}
			}
			
			if (this->carrierLostTimer!=0){
				switch (this->carrierLostTimer){
					case 192://20ms
						if (!this->requestHighSpeed){
							this->carrierFoundTimer=0;
							this->setTxSpeed(false);
						}
						break;
					case 2304://220+20ms
						if (!this->requestHighSpeed){
							this->carrierFoundTimer=0;
							this->setTxSpeed(true);
						}
						break;
					case 4224://440ms
						this->carrierLostTimer=0;
						this->carrierFoundTimer=0;
						this->requestHighSpeed=false;
						this->reset();
						break;
				}
				if (this->carrierLostTimer!=0) this->carrierLostTimer++;
			}
		}
		
		virtual void RxStateChange(bool carrier) override final{
			if (carrier){
				if (this->carrierLostTimer==0) this->carrierFoundTimer=0;
				else this->carrierFoundTimer=1;
			}
			else{
				this->TxEnable=false;
				this->carrierFoundTimer=0;
				if (this->carrierLostTimer==0) this->carrierLostTimer=1;
			}
		}
		
		void requestTxSpeedHigh(){
			if (this->getTxSpeed()) return;
			this->requestHighSpeed=true;
			this->setTxSpeed(true);
		}
		
		void resetTimers(){
			this->setTxSpeed(true);
			this->carrierFoundTimer=0;
			this->carrierLostTimer=0;
			this->requestHighSpeed=false;
		}
		
		
		
		virtual void RxUpdate(unsigned char d) override final{
			constexpr unsigned short P=0b0110100110010110;
			d=((bool)(((P>>(d&0x0F))^(P>>(d>>4)))&0x01))?0x1A:d&0x7F;
			if (this->VDTSplitter.updateSequence(d)){
				constexpr unsigned char IRD[]={0x1B,0x23,0x20,0x2C,0x21,0x3C};
				constexpr unsigned char PRO1_RET1[]={0x1B,0x39,0x6C};
				constexpr unsigned char AC[]={0x1B,0x20,0x2C,0x21,0x39};
				constexpr unsigned char STAT[]={0x1B,0x3A,0x73,0x40};//PCE deactivated
				constexpr unsigned char PRO1_DECONNEXION[]={0,0,0,0x1B,0x39,0x67};
				switch (this->VDTSplitter.getSequenceType()){
					case VideotexSplitter::SequenceType::SEP:
						switch (this->VDTSplitter.sequence[1]){
							case 0x53://ACK connexion
								for (size_t i=0;i<sizeof(IRD)/sizeof(IRD[0]);i++) this->qTx.push(IRD[i]);
								break;
							case 0x4C:
								for (size_t i=0;i<sizeof(PRO1_RET1)/sizeof(PRO1_RET1[0]);i++) this->qTx.push(PRO1_RET1[i]);
								break;
							case 0x4D:
								this->requestTxSpeedHigh();
								break;
							case 0x51:
								this->TxEnable=true;
								break;
						}
						break;
					case VideotexSplitter::SequenceType::ESC_nF:
						if (this->VDTSplitter.sequence.back()==0x38){//DC
							if ((!this->service)&&this->VDTSplitter.sequence.size()==5&&this->VDTSplitter.sequence[1]==0x20&&this->VDTSplitter.sequence[2]==0x2C&&this->VDTSplitter.sequence[3]==0x21){
								for (size_t i=0;i<sizeof(AC)/sizeof(AC[0]);i++) this->qTx.push(AC[i]);
								this->service=true;
							}
						}
						if (this->VDTSplitter.sequence.back()==0x3A){//ILC
							if (this->service&&this->VDTSplitter.sequence.size()==5&&this->VDTSplitter.sequence[1]==0x20&&this->VDTSplitter.sequence[2]==0x2C&&this->VDTSplitter.sequence[3]==0x21){
								this->service=false;
								for (size_t i=0;i<sizeof(PRO1_DECONNEXION)/sizeof(PRO1_DECONNEXION[0]);i++) this->qTx.push(PRO1_DECONNEXION[i]);
							}
						}
						break;
					case VideotexSplitter::SequenceType::ESC_Fp_PRO2:
						if (this->VDTSplitter.sequence[2]==0x69&&this->VDTSplitter.sequence[3]==0x44){//PRO2 START PCE
							for (size_t i=0;i<sizeof(STAT)/sizeof(STAT[0]);i++) this->qTx.push(STAT[i]);
						}
						break;
					case VideotexSplitter::SequenceType::US_STUTEL:
						if (this->service){
							this->parseDDU();
						}
						break;
					default:
						break;
				}
			}
		}
		
		virtual std::pair<bool,unsigned char> TxNext() override final{
			std::pair<bool,unsigned char> d;
			constexpr unsigned short P=0b0110100110010110;
			if (this->TxEnable&&(bool)this->qTx.size()){
				unsigned char v=this->qTx.front();
				v^=((P>>(v&0x0F))^(P>>(v>>4)))<<7;
				d=std::pair<bool,unsigned char>(true,v);
				this->qTx.pop();
			}
			else{
				d=std::pair<bool,unsigned char>(false,0);
			}
			return d;
		}
		
		void Init() override final{
			this->TxEnable=true;
			this->VDTSplitter.reset();
			std::queue<unsigned char> empty;
			std::swap(qTx,empty);
			
			this->service=false;
			this->DSet=false;
			this->TAssociate=false;
			
			this->resetTimers();
		}
		
		bool DSet=false;
		bool TAssociate=false;
		size_t blockNumber=0;
		std::vector<unsigned char> TWriteDataBuffer;
		
		void parseDDU(){
			switch (this->VDTSplitter.sequence[2]){//CI
				case 0x30://D-Response-positive
					break;
				case 0x31://D-Response-negative
					break;
				case 0x39://D-U abort
					this->DSet=false;
					this->TAssociate=false;
					break;
				case 0x50 ... 0x5F://D-Data
					if (this->DSet){
						{
							std::vector<unsigned char> eFields(this->VDTSplitter.sequence.begin()+3,this->VDTSplitter.sequence.end()-1);
							std::vector<unsigned char> dFields;
							if (!DProtocolTranslationMode4Decode(&dFields,&eFields)) this->sendDUAbort();//error in the encoding
							else{
								this->parseTDU(&dFields);
							}
						}
					}
					break;
				case 0x40 ... 0x4F://D-Set mode
					if ((this->VDTSplitter.sequence[2]&0x03)!=0){//translation not in mode 4 / not handled
						this->sendDUAbort();
					}
					else{
						{
							std::vector<unsigned char> eFields(this->VDTSplitter.sequence.begin()+3,this->VDTSplitter.sequence.end()-1);
							std::vector<unsigned char> dFields;
							if (!DProtocolTranslationMode4Decode(&dFields,&eFields)) this->sendDUAbort();//error in the encoding
							else{
								dFields.insert(dFields.begin(),0);
								std::vector<unsigned char> DData;
								if (this->nextTLV(&dFields, NULL, &DData)){
									if (this->parseDSetModeParams(&DData)){
										if ((this->VDTSplitter.sequence[2]&0x0C)==0x0C) this->sendDResponsePositive();
										this->DSet=true;
										this->TAssociate=false;
										this->parseTDU(&dFields);
									}
									else this->sendDUAbort();
								}
								else this->sendDUAbort();
							}
						}
					}
					break;
				case 0x60 ... 0x7F://D-Set mode / BCS/sequence code not handled at the moment
					this->sendDUAbort();
					break;
				default:
					this->sendDUAbort();
					break;
			}
		}
		
		bool parseDSetModeParams(std::vector<unsigned char>* DParams){
			bool CMode=false;
			std::vector<unsigned char> pp;
			unsigned char p;
			while (this->nextTLV(DParams, &p, &pp)){
				if (p==0x23&&pp.size()==1&&(pp[0]&0x07)==0x02){
					CMode=true;
					break;
				}
			}
			return CMode;
		}
		
		void parseTDU(std::vector<unsigned char>* DData){
			std::vector<unsigned char> TDUParams;
			unsigned char TDU;
			while (this->nextTLV(DData,&TDU,&TDUParams)){
				switch (TDU){
					case 0x20:
						this->parseTAssociateParams(&TDUParams);
						break;
					case 0x21:
						this->TAssociate=false;
						this->sendTResponsePositive();
						break;
					case 0x2F:
						this->parseTWriteParams(&TDUParams);
						break;
					case 0x32:
					case 0x33:
						break;
					default:
						this->sendTAbort();
						break;
				}
			}
		}
		
		void parseTAssociateParams(std::vector<unsigned char>* TParams){
			std::vector<unsigned char> pp;
			unsigned char p;
			bool name=false;
			bool sclass=false;
			bool confirm=false;
			while (this->nextTLV(TParams,&p,&pp)){
				switch(p){
					case 0x45:
						name=pp.size()==1&&pp[0]=='M';
						break;
					case 0x51:
						sclass=pp.size()==1&&(pp[0]&0x03)==0x01;
						break;
					case 0x4C:
						confirm=pp.size()==1&&(bool)(pp[0]&0x08);
						break;
				}
			}
			if (name&&sclass){
				this->TAssociate=true;
				if (confirm){
					this->sendTResponsePositive();
				}
			}
			else{
				this->TAssociate=false;
				if (confirm){
					this->sendTResponseNegative();
					this->blockNumber=0;
				}
				else{
					this->sendTAbort();
				}
			}
		}
		
		void parseTWriteParams(std::vector<unsigned char>* TParams){
			if (this->TAssociate){
				std::vector<unsigned char> pp;
				unsigned char p;
				if (this->nextTLV(TParams,&p,&pp)&&p==0x4C&&pp.size()==1&&TParams->size()<=1024){//basic kernel limitations
					if ((pp[0]&0x01)==0x01) this->TWriteDataBuffer.clear();
					this->TWriteDataBuffer.insert(this->TWriteDataBuffer.end(),TParams->begin(),TParams->end());
					if ((pp[0]&0x02)==0x02){
						this->TWriteDataCompleteCallback();
						this->sendTResponsePositive();
					}
					else if ((bool)(pp[0]&0x08)){
						this->sendTResponsePositive();
					}
				}
				else{
					this->sendTAbort();
				}
			}
			else{
				this->sendTAbort();
			}
		}
		
		void sendDUAbort(){
			this->DSet=false;
			this->TAssociate=false;
			this->qTx.push(0x1F);
			this->qTx.push(0x3E);
			this->qTx.push(0x39);
			this->qTx.push(0x0D);
		}
		void sendDResponsePositive(){
			this->qTx.push(0x1F);
			this->qTx.push(0x3E);
			this->qTx.push(0x30);
			this->qTx.push(0x0D);
		}
		void sendTResponsePositive(){
			this->qTx.push(0x1F);
			this->qTx.push(0x3E);
			this->qTx.push(0x50);
			this->qTx.push(0x32);
			this->qTx.push(0x0D);
		}
		void sendTResponseNegative(){
			this->qTx.push(0x1F);
			this->qTx.push(0x3E);
			this->qTx.push(0x50);
			this->qTx.push(0x33);
			this->qTx.push(0x0D);
		}
		void sendTAbort(){
			this->TAssociate=false;
			this->qTx.push(0x1F);
			this->qTx.push(0x3E);
			this->qTx.push(0x50);
			this->qTx.push(0x38);
			this->qTx.push(0x0D);
		}
		void sendTReadRestart(){
			this->qTx.push(0x1F);
			this->qTx.push(0x3E);
			this->qTx.push(0x50);
			this->qTx.push(0x37);
			this->qTx.push(0x0D);
		}
		
		static bool nextTLV(std::vector<unsigned char>* sequence, unsigned char* type, std::vector<unsigned char>* value){
			unsigned short tlvLength;
			if (sequence->size()<3) return false;
			if (type!=NULL) *type=(*sequence)[0];
			if ((*sequence)[1]==0xFF){
				if (sequence->size()<5) return false;
				tlvLength=(((unsigned short)(*sequence)[2])<<8)+(*sequence)[3]+4;
				if (sequence->size()<tlvLength) return false;
				value->assign(sequence->begin()+4,sequence->begin()+tlvLength);
			}
			else{
				tlvLength=(*sequence)[1]+2;
				if (sequence->size()<tlvLength) return false;
				value->assign(sequence->begin()+2,sequence->begin()+tlvLength);
			}
			sequence->erase(sequence->begin(),sequence->begin()+tlvLength);
			return true;
		}
		
		
		
		void TWriteDataCompleteCallback(){
			printf("data ");
			for(int i=0;i<this->TWriteDataBuffer.size();i++){
				if (this->TWriteDataBuffer[i]<0x20||this->TWriteDataBuffer[i]>=0x7F) printf("\\x%02X",this->TWriteDataBuffer[i]);
				else printf("%c",this->TWriteDataBuffer[i]);
			}
			printf("\n");
		}
		
		void TResponseCallback(bool state){
			
		}
		void TAbortCallback(){
			
		}
		void DResponseCallback(bool state){
			
		}
		void DAbortCallback(){
			
		}
		
		void sendMessageUpdateState(){
			
		}
		
};
#endif