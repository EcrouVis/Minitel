#ifndef KEYBOARD_H
#define KEYBOARD_H
#include <functional>
#include <queue>
#include <cstdio>
#include <cmath>
#include <atomic>
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
struct keyboard_message{
	bool focus;
	int scancode;
	int action;
	int mods;
};
const unsigned char LED_OFF=0;
const unsigned char LED_ON=1;
const unsigned char LED_BLINK=2;


class Keyboard{
	public:
		
		std::atomic_uchar LED_POWER=LED_OFF;
		std::atomic_uchar LED_SPEAKER=LED_OFF;
	
		void CLKTickIn(){
			if (this->S_out_step>0||this->SBUF_out_queue.size()!=0){
				switch (this->S_out_step){
					case 0://printf("kb out %02X\n",this->SBUF_out_queue.front());
						this->sendSerial(false);
						this->SBUF_out=this->SBUF_out_queue.front();
						this->SBUF_out_queue.pop();
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
						this->sendSerial((bool)(this->SBUF_out&1));
						this->SBUF_out=this->SBUF_out>>1;
						this->S_out_step++;
						break;
					case 9:
						this->sendSerial(true);
						this->S_out_step++;
						break;
					case 10:
						this->sendSerial(true);
						this->S_out_step=0;
						break;
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
				if (!(bool)(this->phone_status&0x10)){
					this->phone_status|=0x10;
					this->sendStatus();
				}
				if (this->DTMF_step>=((this->DTMF_queue.front()==12)?600:89)){
					this->DTMF_step=0;
					this->DTMF_queue.pop();
				}
			}
			else if ((bool)(this->phone_status&0x10)){
				this->phone_status&=~0x10;
				this->sendStatus();
			}
			
		}
		void KeyboardChangeIn(keyboard_message* kb_m){
			if (kb_m->focus){
				if (kb_m->action==GLFW_PRESS||kb_m->action==GLFW_RELEASE){
					bool keyPressed=(kb_m->action==GLFW_PRESS);
					switch (kb_m->scancode){
						case 16:this->queueKey(0xBB,keyPressed);break;//A
						case 17:this->queueKey(0xB7,keyPressed);break;//Z
						case 18:this->queueKey(0xB9,keyPressed);break;//E
						case 19:this->queueKey(0xA9,keyPressed);break;//R
						case 20:this->queueKey(0x97,keyPressed);break;//T
						case 21:this->queueKey(0x87,keyPressed);break;//Y
						case 22:this->queueKey(0x77,keyPressed);break;//U
						case 23:this->queueKey(0x67,keyPressed);break;//I
						case 24:this->queueKey(0x69,keyPressed);break;//O
						case 25:this->queueKey(0x57,keyPressed);break;//P
						case 30:this->queueKey(0xBF,keyPressed);break;//Q
						case 31:this->queueKey(0xBD,keyPressed);break;//S
						case 32:this->queueKey(0xAB,keyPressed);break;//D
						case 33:this->queueKey(0x99,keyPressed);break;//F
						case 34:this->queueKey(0x8B,keyPressed);break;//G
						case 35:this->queueKey(0x89,keyPressed);break;//H
						case 36:this->queueKey(0x79,keyPressed);break;//J
						case 37:this->queueKey(0x6B,keyPressed);break;//K
						case 38:this->queueKey(0x59,keyPressed);break;//L
						case 39:this->queueKey(0x3B,keyPressed);break;//M
						case 44:this->queueKey(0xAD,keyPressed);break;//W
						case 45:this->queueKey(0x9B,keyPressed);break;//X
						case 46:this->queueKey(0x8D,keyPressed);break;//C
						case 47:this->queueKey(0x8F,keyPressed);break;//V
						case 48:this->queueKey(0x7D,keyPressed);break;//B
						case 49:this->queueKey(0x7B,keyPressed);break;//N
						case 57:this->queueKey(0x7F,keyPressed);break;//espace
						case 28:this->queueKey(0x39,keyPressed);break;//entrée
						case 50:this->queueKey(0x6F,keyPressed);break;//,
						case 51:this->queueKey(0x6D,keyPressed);break;//;
						case 52:this->queueKey(0x3D,keyPressed);break;//:
						
						case 82:this->queueKey(0x2F,keyPressed);break;//numpad 0
						case 79:this->queueKey(0x27,keyPressed);break;//numpad 1
						case 80:this->queueKey(0x17,keyPressed);break;//numpad 2
						case 81:this->queueKey(0x21,keyPressed);break;//numpad 3
						case 75:this->queueKey(0x19,keyPressed);break;//numpad 4
						case 76:this->queueKey(0x29,keyPressed);break;//numpad 5
						case 77:this->queueKey(0x11,keyPressed);break;//numpad 6
						case 71:this->queueKey(0x2B,keyPressed);break;//numpad 7
						case 72:this->queueKey(0x1B,keyPressed);break;//numpad 8
						case 73:this->queueKey(0x2D,keyPressed);break;//numpad 9
						
						case 2:this->queueKey(0xB3,keyPressed);break;//&
						case 3:this->queueKey(0xB1,keyPressed);break;//é
						case 4:this->queueKey(0xA7,keyPressed);break;//"
						case 5:this->queueKey(0xA1,keyPressed);break;//'
						case 6:this->queueKey(0x91,keyPressed);break;//(
						case 7:this->queueKey(0x81,keyPressed);break;//-
						case 8:this->queueKey(0x71,keyPressed);break;//è
						case 9:this->queueKey(0x61,keyPressed);break;//_=!
						case 10:this->queueKey(0x51,keyPressed);break;//ç
						case 11:this->queueKey(0x53,keyPressed);break;//à
						case 12:this->queueKey(0x37,keyPressed);break;//)
						
						case 43:this->queueKey(0x1F,keyPressed);break;//*
						case 40:this->queueKey(0x1D,keyPressed);break;//ù=#
						
						case 328:this->queueKey(0x5B,keyPressed);break;//flèche haut
						case 331:this->queueKey(0x5D,keyPressed);break;//flèche gauche
						case 336:this->queueKey(0x5F,keyPressed);break;//flèche bas
						case 333:this->queueKey(0x3F,keyPressed);break;//flèche droite
						
						case 42:this->queueKey(0xAF,keyPressed);break;//shift
						case 58:this->queueKey(0x9F,keyPressed);break;//min/maj
						case 285:
						case 29:this->queueKey(0x9D,keyPressed);break;//ctrl
						
						case 56:this->queueKey(0xA3,keyPressed);break;//alt=fnct
						case 1:this->queueKey(0xA5,keyPressed);break;//échap=Esc
						
						case 41:this->queueKey(0x55,keyPressed);break;//²=on/off
						case 13:this->queueKey(0x31,keyPressed);break;//==mem
						case 15:this->queueKey(0xB5,keyPressed);break;//tab=Connex/Fin
						case 78:this->queueKey(0x23,keyPressed);break;//numpad +=HP+
						case 74:this->queueKey(0x13,keyPressed);break;//numpad -=HP-
						case 14:this->queueKey(0x35,keyPressed);break;//backspace=HP
						case 55:this->queueKey(0x1F,keyPressed);break;//numpad *=*
						case 309:this->queueKey(0x1D,keyPressed);break;//numpad /=#
						//.../... bis repertoire annuaire decrochage
						
						case 60:this->queueKey(0x95,keyPressed);break;//F2=Sommaire
						case 61:this->queueKey(0x93,keyPressed);break;//F3=Guide
						case 62:this->queueKey(0x85,keyPressed);break;//F4=Annulation
						case 63:this->queueKey(0x83,keyPressed);break;//F5=Correction
						case 64:this->queueKey(0x75,keyPressed);break;//F6=Retour
						case 65:this->queueKey(0x73,keyPressed);break;//F7=Suite
						case 66:this->queueKey(0x65,keyPressed);break;//F8=Répétition
						case 67:this->queueKey(0x63,keyPressed);break;//F9=Envoi
					}
				}
			}
		}
		void queueKey(unsigned char keycode,bool keyPressed){
			unsigned char a=keyPressed?0x00:0x08;
			a|=(a+keycode+(keycode>>4))<<4;
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
		
		float getSpeakerSample(unsigned long sampleRate){
			float s;
			s=this->getRingtoneSample(sampleRate);
			//TODO
			return s;
		}
	private:
		unsigned char phone_status=0x61;
	
		bool S_in=false;
		unsigned char S_in_step=0;
		unsigned char S_in_low=0;
		unsigned char S_out_step=0;
		unsigned char SBUF_in=0;
		unsigned char SBUF_out=0;
		std::queue<unsigned char> SBUF_out_queue;
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
			this->SBUF_out_queue.push((unsigned char)(((0x0C+this->phone_status+(this->phone_status>>4))<<4)|0x0C));
			this->SBUF_out_queue.push(this->phone_status);
		}
		void executeCommand(){
			//printf("keyboard serial in %02X%02X\n",this->cmd_p1,this->cmd_p2);
			if ((bool)(this->cmd_p1&0x04)){//commande
				switch (this->cmd_p2){
					case 0x01:
						if((this->phone_status&0x48)!=0x08){
							this->phone_status=(this->phone_status&(~0x48))|0x08;
							this->sendStatus();
						}
						printf("phone line connected\n");
						break;
					case 0x03:
						if((this->phone_status&0x48)!=0x40){
							this->phone_status=(this->phone_status&(~0x48))|0x40;
							this->sendStatus();
						}
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
						
					case 0x17:this->sendStatus();break;
					
					case 0x21:this->LED_SPEAKER.store(LED_OFF,std::memory_order_release);printf("power off speaker led\n");break;
					case 0x23:this->LED_POWER.store(LED_OFF,std::memory_order_release);printf("power off on/off led\n");break;
					
					case 0x29:this->LED_SPEAKER.store(LED_ON,std::memory_order_release);printf("power on speaker led\n");break;
					case 0x2B:this->LED_POWER.store(LED_ON,std::memory_order_release);printf("power on on/off led\n");break;
					
					case 0x33:this->LED_POWER.store(LED_BLINK,std::memory_order_release);printf("blink on/off led\n");break;
					
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
			/*if (this->cmd_p1==0xC4&&this->cmd_p2==0x17){
				if (this->line_closed){
					this->SBUF_out_queue.push(0xFC);
					this->SBUF_out_queue.push(0x21);
				}
				else{
					this->SBUF_out_queue.push(0x3C);
					this->SBUF_out_queue.push(0x61);
				}
			}*/
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
				constexpr float v[4]={0.0316227766,0.1,0.316227766,1};//guess TODO
				return ((this->ringtone_phase_tick*4>sampleRate)?-1.:1.)*v[this->ringtone_volume]*a;
			}
			return 0.;
		}
		float getPhoneLineSample(){
			constexpr float v[4]={0.0316227766,0.1,0.316227766,1};//guess TODO
			return v[this->speaker_volume]*0;//TODO
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