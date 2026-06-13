#ifndef RTCNETWORK_H
#define RTCNETWORK_H
#include <functional>
#include <cmath>
#include "circuit/PhoneLine.h"
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
		
};
class RTCNetwork{
	public:
		
		bool requestPhoneLine(RTCService* srv){
			if (this->currentState!=this->IDLE) return false;
			this->currentState=this->CALL_INCOMING;
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
						this->timer=0;
						this->phoneLineStateOut=line_call_progress_tone;
						this->sendPhoneLine(this->phoneLineStateOut);
						break;
					case this->WAIT_PHONE_NUMBER:
						this->phoneLineStateIn=state;
						if ((bool)(state&line_DTMF)){
							this->currentState=this->PHONE_DIALING;
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
						//this->ServiceLinked->phoneLineChangeIn(state);
						break;
					case this->WAIT_CALL_END:
						this->phoneLineStateIn=state;
						break;
					case this->CALL_INCOMING:
						this->phoneLineStateIn=state;
						this->currentState=this->CONNECTED;
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
					this->timer=0;
					this->phoneLineStateOut=0;
					this->sendPhoneLine(this->phoneLineStateOut);
				}
				if (this->ServiceLinked!=NULL){
					this->ServiceLinked->sendPhoneLine=[](unsigned short s){};
					this->ServiceLinked->phoneLineChangeIn(0);
					this->ServiceLinked=NULL;
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
									}
									else if (pnsg==PHONE_NUMBER_FINISHED){
										this->timer=0;
										this->currentState=this->CONNECTED;
									}
								}
							}
							else{
								this->timer=0;
								this->currentState=this->WAIT_CALL_END;
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
			printf("minipavi: %04X\n",s);
			if (((bool)(s&line_Closed))!=this->selected){
				this->selected=!this->selected;
				if (this->selected) this->sendPhoneLine(line_v23_1200bps_1);
			}
		};
	private:
		bool selected=false;
};
#endif