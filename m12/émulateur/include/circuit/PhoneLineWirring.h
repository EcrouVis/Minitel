#ifndef PHONELINEWIRRING_H
#define PHONELINEWIRRING_H
#include <functional>
#include "circuit/PhoneLine.h"
class PhoneLineWire{//simplify wirring+relay for the phone line
	public:
		void closeRelay(bool b){
			if (this->RelayState==b) return;
			this->RelayState=b;
			if (b){
				unsigned short st=this->ModemState|this->KeyboardState|this->RTCState;
				this->sendWireModem(st&~line_Closed);
				this->sendWireLine(st);
			}
			else{
				unsigned short st=this->KeyboardState|this->RTCState;
				this->sendWireLine(st);
				this->sendWireModem(this->ModemState&~line_Closed);
			}
		}
		void wireModemIn(unsigned short s){
			printf("M %04X\n",s);
			this->ModemState=s&~line_Closed;
			if (this->RelayState){
				unsigned short st=this->ModemState|this->KeyboardState|this->RTCState;
				this->sendWireModem(st&~line_Closed);
				this->sendWireLine(st);
			}
			else{
				this->sendWireModem(this->ModemState&~line_Closed);
			}
		}
		void wireKeyboardIn(unsigned short s){
			printf("K %04X\n",s);
			this->KeyboardState=s;
			if (this->RelayState){
				unsigned short st=this->ModemState|this->KeyboardState|this->RTCState;
				this->sendWireModem(st&~line_Closed);
				this->sendWireLine(st);
			}
			else{
				unsigned short st=this->KeyboardState|this->RTCState;
				this->sendWireLine(st);
			}
		}
		void wireRTCIn(unsigned short s){
			printf("R %04X\n",s);
			this->RTCState=s;
			if (this->RelayState){
				unsigned short st=this->ModemState|this->KeyboardState|this->RTCState;
				this->sendWireModem(st&~line_Closed);
				this->sendWireLine(st);
			}
			else{
				unsigned short st=this->KeyboardState|this->RTCState;
				this->sendWireLine(st);
			}
		}
		void subscribeWireModem(std::function<void(unsigned short)> f){
			this->sendWireModem=f;
		}
		void subscribeWireLine(std::function<void(unsigned short)> f){
			this->sendWireLine=f;
		}
		void setModemSample(float f){
			this->ModemSample=f;
		}
		void setKeyboardSample(float f){
			this->KeyboardSample=f;
		}
		void setRTCSample(float f){
			this->RTCSample=f;
		}
		float getModemSample(){
			if (this->RelayState){
				return this->ModemSample+this->KeyboardSample+this->RTCSample;
			}
			else{
				return this->ModemSample;
			}
		}
		float getPhoneLineSample(){
			if (this->RelayState){
				return this->ModemSample+this->KeyboardSample+this->RTCSample;
			}
			else{
				return this->KeyboardSample+this->RTCSample;
			}
		}
	private:
		unsigned short ModemState=0;
		unsigned short KeyboardState=0;
		unsigned short RTCState=0;
		float ModemSample=0;
		float KeyboardSample=0;
		float RTCSample=0;
		bool RelayState=false;
		std::function<void(unsigned short)> sendWireModem=[](unsigned short s){};
		std::function<void(unsigned short)> sendWireLine=[](unsigned short s){};
};
#endif