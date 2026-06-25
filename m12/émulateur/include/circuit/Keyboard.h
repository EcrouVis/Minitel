#ifndef KEYBOARD_H
#define KEYBOARD_H
#include <functional>
#include <queue>
#include <cstdio>
#include <cmath>
#ifndef M_PI
#define M_PI (3.14159265358979323846264338327950288)
#endif
#include <atomic>
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
#include "circuit/PhoneLine.h"
struct keyboard_message{
	bool focus;
	int scancode;
	int action;
	int mods;
};
const unsigned char LED_OFF=0;
const unsigned char LED_ON=1;
const unsigned char LED_BLINK_FAST=2;
const unsigned char LED_BLINK_SLOW=3;


class Keyboard{
	public:
		
		std::atomic_uchar LED_POWER=LED_OFF;
		std::atomic_uchar LED_SPEAKER=LED_OFF;
	
		void CLKTickIn(){
			
			{
				std::unique_lock<std::mutex> lock(this->SBUF_out_mutex);
				
				if (this->S_out_step>0||this->SBUF_out_queue.size()!=0){
					switch (this->S_out_step){
						case 0://printf("kb out %02X\n",this->SBUF_out_queue.front());
							this->SBUF_out=this->SBUF_out_queue.front();
							this->SBUF_out_queue.pop();
							lock.unlock();
							this->sendSerial(false);
							this->S_out_step++;
							break;
						case 1:
						case 2:
						case 3:
						case 4:
						case 5:
						case 6:
						case 7:
						case 8:
							lock.unlock();
							this->sendSerial((bool)(this->SBUF_out&1));
							this->SBUF_out=this->SBUF_out>>1;
							this->S_out_step++;
							break;
						case 9:
							lock.unlock();
							this->sendSerial(true);
							this->S_out_step++;
							break;
						case 10:
							lock.unlock();
							this->sendSerial(true);
							this->S_out_step=0;
							break;
					}
				}
			}
			if (this->S_in_step>0){
				switch (this->S_in_step){
					case 1:
					case 2:
					case 4:
					case 6:
					case 8:
					case 10:
					case 12:
					case 14:
					case 16:
					case 18:this->S_in_step++;break;
					
					case 3:
					case 5:
					case 7:
					case 9:
					case 11:
					case 13:
					case 15:
					case 17:this->SBUF_in=(this->SBUF_in>>1)|(this->S_in?0x80:0);this->S_in_step++;break;
					
					case 19:
						this->S_in_step=0;
						if (this->S_in){
							this->commandReceived();
						}
						break;
				}
			}
			if ((!this->S_in)&&this->S_in_low<22){
				this->S_in_low++;
			}
			if (this->DTMF_step>0||this->DTMF_queue.size()!=0){
				this->DTMF_step++;
				if (this->DTMF_queue.front()==12){
					if (this->DTMF_step>=600){
						this->DTMF_step=0;
						this->DTMF_queue.pop();
					}
				}
				else{
					if (!(bool)(this->phone_status&0x10)){
						this->phone_status|=0x10;
						this->sendStatus();
						constexpr unsigned short freq[16]={line_DTMF_0,line_DTMF_1,line_DTMF_2,line_DTMF_3,line_DTMF_4,line_DTMF_5,line_DTMF_6,line_DTMF_7,line_DTMF_8,line_DTMF_9,0,0,0,0,line_DTMF_S,line_DTMF_H};
						this->phoneLineStateOut|=freq[this->DTMF_queue.front()];
						this->sendPhoneLine(this->phoneLineStateOut);
					}
					if (this->DTMF_step>=45&&(bool)(this->phoneLineStateOut&line_DTMF)){
						this->phoneLineStateOut&=~line_DTMF;
						this->sendPhoneLine(this->phoneLineStateOut);
					}
					if (this->DTMF_step>=89){
						this->DTMF_step=0;
						this->DTMF_queue.pop();
						this->phone_status&=~0x10;//TODO: re test behaviour
						this->sendStatus();
					}
				}
			}
			else if ((bool)(this->phone_status&0x10)){
				this->phone_status&=~0x10;
				this->sendStatus();
			}
			
		}
		void queueKey(unsigned char keycode,bool keyPressed){
			unsigned char a=keyPressed?0x00:0x08;
			a|=(a+keycode+(keycode>>4))<<4;
			std::lock_guard<std::mutex> lock(this->SBUF_out_mutex);
			this->SBUF_out_queue.push(a);
			this->SBUF_out_queue.push(keycode);
		}
		void subscribeSerial(std::function<void(bool)> f){
			this->sendSerial=f;
		}
		
		void serialChangeIn(bool b){
			if ((!b)&&this->S_in&&this->S_in_step==0) this->S_in_step=1;
			if (b&&(!this->S_in)){
				if(this->S_in_low>=22){
					printf("Keyboard reset !!!!!!!!!!!!!!!!!!!!!!!!!\n");
					/////////////////////////////////////////////////reset?
					this->phone_status|=0x80;
					this->sendStatus();
					this->phone_status&=~0x80;
				}
				this->S_in_low=0;
			}
			this->S_in=b;
		}
		
		void phoneLineChangeIn(unsigned short state){
			if (this->phoneLineStateIn==state) return;
			this->phoneLineStateIn=state;
			if (((bool)(state&line_Ringing))!=((bool)(this->phone_status&0x04))){
				this->phone_status&=~0x04;
				this->phone_status|=((bool)(state&line_Ringing))?0x04:0;
				this->sendStatus();
				if ((bool)(state&line_Ringing)) this->playRingtone();
			}
		}
		
		void subscribePhoneLine(std::function<void(unsigned short)> f){
			this->sendPhoneLine=f;
		}
		
		float getSpeakerSample(unsigned long sampleRate){
			float s;
			s=this->getRingtoneSample(sampleRate);
			if (this->speaker_on) s+=this->phoneLineSample;
			constexpr float v[4]={0.0316227766,0.1,0.316227766,1};//guess TODO
			s*=v[this->ringtone_volume];
			return s;
		}
		void setPhoneLineSample(float f){
			this->phoneLineSample=f;
		}
		float getPhoneLineSample(unsigned long sampleRate){
			switch (this->phoneLineStateOut&line_DTMF_high){
				case 0:this->dtmf_phase1=0;break;
				case line_DTMF_1209Hz:this->dtmf_phase1+=1209;break;
				case line_DTMF_1336Hz:this->dtmf_phase1+=1336;break;
				case line_DTMF_1477Hz:this->dtmf_phase1+=1477;break;
				case line_DTMF_1633Hz:this->dtmf_phase1+=1633;break;
			}
			if (this->dtmf_phase1>sampleRate) this->dtmf_phase1-=sampleRate;
			switch (this->phoneLineStateOut&line_DTMF_low){
				case 0:this->dtmf_phase1=0;break;
				case line_DTMF_697Hz:dtmf_phase2+=697;break;
				case line_DTMF_770Hz:dtmf_phase2+=770;break;
				case line_DTMF_852Hz:dtmf_phase2+=852;break;
				case line_DTMF_941Hz:dtmf_phase2+=941;break;
			}
			if (this->dtmf_phase2>sampleRate) this->dtmf_phase2-=sampleRate;
			return std::sin(2*M_PI*((float)this->dtmf_phase2)/((float)sampleRate))+0.8*std::sin(2*M_PI*((float)this->dtmf_phase1)/((float)sampleRate));
		}
	private:
		unsigned char phone_status=0x61;
		
		unsigned short phoneLineStateIn=0;
		unsigned short phoneLineStateOut=0;
		std::function<void(unsigned short)> sendPhoneLine=[](unsigned short d){};
	
		bool S_in=false;
		unsigned char S_in_step=0;
		unsigned char S_in_low=0;
		unsigned char S_out_step=0;
		unsigned char SBUF_in=0;
		unsigned char SBUF_out=0;
		std::queue<unsigned char> SBUF_out_queue;
		std::mutex SBUF_out_mutex;
		unsigned short DTMF_step=0;
		std::queue<unsigned char> DTMF_queue;
		std::function<void(bool)> sendSerial=[](bool b){};
		bool command_part=false;
		unsigned char cmd_p1;
		unsigned char cmd_p2;
		
		bool ringtone_activated=false;
		
		unsigned int ringtone_tick=0;
		unsigned char ringtone_note=16;
		unsigned int ringtone_phase_tick=0;
		
		unsigned char ringtone=0;
		unsigned char ringtone_volume=0;
		
		constexpr static unsigned int REd_4=622;
		constexpr static unsigned int MI_4=659;
		constexpr static unsigned int FA_4=698;
		constexpr static unsigned int SOL_4=784;
		constexpr static unsigned int SOLd_4=831;
		constexpr static unsigned int LAd_4=932;
		constexpr static unsigned int SI_4=988;
		constexpr static unsigned int DO_5=1047;
		constexpr static unsigned int DOd_5=1109;
		constexpr static unsigned int MI_5=1319;
		constexpr static unsigned int SOLd_5=1661;
		constexpr static unsigned int NO_NOTE=0;
		
		constexpr static unsigned int ringtones[5][16]={//1 note->1/12s
			{MI_5,SOLd_5,MI_5,SOLd_5,MI_5,SOLd_5,MI_5,SOLd_5,MI_5,SOLd_5,MI_5,SOLd_5,MI_5,SOLd_5,MI_5,SOLd_5},
			{REd_4,REd_4,REd_4,REd_4,REd_4,REd_4,REd_4,REd_4,REd_4,REd_4,REd_4,REd_4,REd_4,REd_4,REd_4,REd_4},
			{MI_4,SOLd_4,MI_4,SOLd_4,MI_4,SOLd_4,MI_4,SOLd_4,MI_4,SOLd_4,MI_4,SOLd_4,MI_4,SOLd_4,MI_4,SOLd_4},
			{SI_4,SOLd_4,SOLd_4,FA_4,SI_4,SOLd_4,SOLd_4,FA_4,SI_4,SOLd_4,SOLd_4,FA_4,SI_4,SOLd_4,SOLd_4,FA_4},
			{REd_4,REd_4,SOL_4,LAd_4,DOd_5,DOd_5,DOd_5,LAd_4,  DO_5,DO_5,  DO_5,FA_4,SOLd_4,FA_4,SOLd_4,FA_4}
		};
		
		bool speaker_on=false;
		unsigned char speaker_volume=0;
		
		float phoneLineSample=0;
		
		unsigned long dtmf_phase1=0;
		unsigned long dtmf_phase2=0;
		
		void commandReceived(){
			if (this->command_part){
				if ((this->SBUF_in&1)==1){
					this->cmd_p2=this->SBUF_in;
					if ((this->cmd_p1>>4)==((this->cmd_p1+this->cmd_p2+(this->cmd_p2>>4))&0x0F)) this->executeCommand();
					this->command_part=false;
				}
				else{
					this->cmd_p1=this->SBUF_in;
					this->command_part=true;
				}
			}
			else{
				if ((this->SBUF_in&1)==0){
					this->command_part=true;
					this->cmd_p1=this->SBUF_in;
				}
			}
		}
		void sendStatus(){
			std::lock_guard<std::mutex> lock(this->SBUF_out_mutex);
			this->SBUF_out_queue.push((unsigned char)(((0x0C+this->phone_status+(this->phone_status>>4))<<4)|0x0C));
			this->SBUF_out_queue.push(this->phone_status);
		}
		void executeCommand(){
			//printf("keyboard serial in %02X%02X\n",this->cmd_p1,this->cmd_p2);
			if ((bool)(this->cmd_p1&0x04)){//commande
				switch (this->cmd_p2){
					case 0x01:
						if((this->phone_status&0x48)!=0x08){//TODO: move code
							this->phone_status=(this->phone_status&(~0x48))|0x08;
							this->sendStatus();
						}
						this->phoneLineStateOut|=line_Closed;
						this->sendPhoneLine(this->phoneLineStateOut);
						printf("phone line connected\n");
						break;
					case 0x03:
						if((this->phone_status&0x48)!=0x40){//TODO: move code
							this->phone_status=(this->phone_status&(~0x48))|0x40;
							this->sendStatus();
						}
						this->phoneLineStateOut&=~line_Closed;
						this->sendPhoneLine(this->phoneLineStateOut);
						printf("phone line disconnected\n");
						break;
					case 0x05:
						printf("speaker activated\n");
						this->speaker_on=true;
						break;
					case 0x07:
						printf("speaker deactivated\n");
						this->speaker_on=false;
						break;
					case 0x09:
						printf("ringtone activated\n");
						this->ringtone_activated=true;
						break;
					case 0x0B:
						printf("ringtone deactivated\n");
						this->ringtone_activated=false;
						break;
					
					case 0x11:
						if (!(bool)(this->phone_status&0x40)){
							this->phone_status=this->phone_status|0x40;
							this->sendStatus();
						}
						printf("microphone activated\n");
						break;
					case 0x13:
						if ((bool)(this->phone_status&0x40)){
							this->phone_status=this->phone_status&(~0x40);
							this->sendStatus();
						}
						printf("microphone deactivated\n");
						break;
						
					case 0x17:this->sendStatus();printf("request phone status %02X\n",this->phone_status);break;
					
					case 0x21:this->LED_SPEAKER.store(LED_OFF,std::memory_order_release);printf("power off speaker led\n");break;
					case 0x23:this->LED_POWER.store(LED_OFF,std::memory_order_release);printf("power off on/off led\n");break;
					case 0x29:this->LED_SPEAKER.store(LED_ON,std::memory_order_release);printf("power on speaker led\n");break;
					case 0x2B:this->LED_POWER.store(LED_ON,std::memory_order_release);printf("power on on/off led\n");break;
					case 0x31:this->LED_SPEAKER.store(LED_BLINK_FAST,std::memory_order_release);printf("blink speaker led fast (guess)\n");break;
					case 0x33:this->LED_POWER.store(LED_BLINK_FAST,std::memory_order_release);printf("blink on/off led fast\n");break;
					case 0x39:this->LED_SPEAKER.store(LED_BLINK_SLOW,std::memory_order_release);printf("blink speaker led slow\n");break;
					case 0x3B:this->LED_POWER.store(LED_BLINK_SLOW,std::memory_order_release);printf("blink on/off led slow (guess)\n");break;
					
					case 0x41:
						//printf("set speaker volume 1\n");
						this->speaker_volume=0;
						break;
					case 0x43:
						//printf("set speaker volume 2\n");
						this->speaker_volume=1;
						break;
					case 0x45:
						//printf("set speaker volume 3\n");
						this->speaker_volume=2;
						break;
					case 0x47:
						//printf("set speaker volume 4\n");
						this->speaker_volume=3;
						break;
					case 0x49:
						//printf("set ringtone volume 1\n");
						this->ringtone_volume=0;
						break;
					case 0x4B:
						//printf("set ringtone volume 2\n");
						this->ringtone_volume=1;
						break;
					case 0x4D:
						//printf("set ringtone volume 3\n");
						this->ringtone_volume=2;
						break;
					case 0x4F:
						//printf("set ringtone volume 4\n");
						this->ringtone_volume=3;
						break;
					
					case 0x81:
						//printf("set ringtone 1\n");
						this->stopRingtone();
						this->ringtone=0;
						break;
					case 0x83:
						//printf("set ringtone 2\n");
						this->stopRingtone();
						this->ringtone=1;
						break;
					case 0x85:
						//printf("set ringtone 3\n");
						this->stopRingtone();
						this->ringtone=2;
						break;
					case 0x87:
						//printf("set ringtone 4\n");
						this->stopRingtone();
						this->ringtone=3;
						break;
					case 0x89:
						//printf("set ringtone 5\n");
						this->stopRingtone();
						this->ringtone=4;
						break;
					case 0x8B:
						//printf("play ringtone\n");
						this->playRingtone();
						break;
					
					default:printf("Unknown cmd %02X%02X\n",this->cmd_p1,this->cmd_p2);break;
					
				}
			}
			else if (!(bool)(this->cmd_p2&0xE0)){//tonalités
				const char tone[16]={'0','1','2','3','4','5','6','7','8','9','?','?','-','?','*','#'};
				printf("DTMF tone %c\n",tone[(this->cmd_p2>>1)&0x0F]);
				DTMF_queue.push((this->cmd_p2>>1)&0x0F);
				
				/////////////////////////////
			}
			else{
				printf("Unknown cmd %02X%02X\n",this->cmd_p1,this->cmd_p2);
			}
		}
		
		float getRingtoneSample(unsigned long sampleRate){
			if (this->ringtone_note<16){//ringtone generation
				this->ringtone_tick+=12;
				this->ringtone_phase_tick+=this->ringtones[this->ringtone][this->ringtone_note];
				if (this->ringtone_phase_tick>=sampleRate) this->ringtone_phase_tick-=sampleRate;
				if (this->ringtone_tick>=sampleRate){
					this->ringtone_tick-=sampleRate;
					this->ringtone_note++;
				}
				float a=1.-std::exp(-((float)this->ringtone_note+((float)this->ringtone_tick)/((float)sampleRate))/(12.*0.06));
				return ((this->ringtone_phase_tick*4>sampleRate)?-1.:1.)*a;
			}
			return 0.;
		}
		void playRingtone(){
			this->ringtone_note=0;
			this->ringtone_tick=0;
		}
		void stopRingtone(){
			this->ringtone_note=16;
		}
};

#endif