#ifndef KEYBOARD_H
#define KEYBOARD_H
#include <functional>
#include <queue>
#include <cstdio>
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
struct keyboard_message{
	bool focus;
	int scancode;
	int action;
	int mods;
};
class Keyboard{
	public:
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
						case 16:queueKey(0xBB,keyPressed);break;//A
						case 17:queueKey(0xB7,keyPressed);break;//Z
						case 18:queueKey(0xB9,keyPressed);break;//E
						case 19:queueKey(0xA9,keyPressed);break;//R
						case 20:queueKey(0x97,keyPressed);break;//T
						case 21:queueKey(0x87,keyPressed);break;//Y
						case 22:queueKey(0x77,keyPressed);break;//U
						case 23:queueKey(0x67,keyPressed);break;//I
						case 24:queueKey(0x69,keyPressed);break;//O
						case 25:queueKey(0x57,keyPressed);break;//P
						case 30:queueKey(0xBF,keyPressed);break;//Q
						case 31:queueKey(0xBD,keyPressed);break;//S
						case 32:queueKey(0xAB,keyPressed);break;//D
						case 33:queueKey(0x99,keyPressed);break;//F
						case 34:queueKey(0x8B,keyPressed);break;//G
						case 35:queueKey(0x89,keyPressed);break;//H
						case 36:queueKey(0x79,keyPressed);break;//J
						case 37:queueKey(0x6B,keyPressed);break;//K
						case 38:queueKey(0x59,keyPressed);break;//L
						case 39:queueKey(0x3B,keyPressed);break;//M
						case 44:queueKey(0xAD,keyPressed);break;//W
						case 45:queueKey(0x9B,keyPressed);break;//X
						case 46:queueKey(0x8D,keyPressed);break;//C
						case 47:queueKey(0x8F,keyPressed);break;//V
						case 48:queueKey(0x7D,keyPressed);break;//B
						case 49:queueKey(0x7B,keyPressed);break;//N
						case 57:queueKey(0x7F,keyPressed);break;//espace
						case 28:queueKey(0x39,keyPressed);break;//entrée
						case 50:queueKey(0x6F,keyPressed);break;//,
						case 51:queueKey(0x6D,keyPressed);break;//;
						case 52:queueKey(0x3D,keyPressed);break;//:
						
						case 82:queueKey(0x2F,keyPressed);break;//numpad 0
						case 79:queueKey(0x27,keyPressed);break;//numpad 1
						case 80:queueKey(0x17,keyPressed);break;//numpad 2
						case 81:queueKey(0x21,keyPressed);break;//numpad 3
						case 75:queueKey(0x19,keyPressed);break;//numpad 4
						case 76:queueKey(0x29,keyPressed);break;//numpad 5
						case 77:queueKey(0x11,keyPressed);break;//numpad 6
						case 71:queueKey(0x2B,keyPressed);break;//numpad 7
						case 72:queueKey(0x1B,keyPressed);break;//numpad 8
						case 73:queueKey(0x2D,keyPressed);break;//numpad 9
						
						case 2:queueKey(0xB3,keyPressed);break;//&
						case 3:queueKey(0xB1,keyPressed);break;//é
						case 4:queueKey(0xA7,keyPressed);break;//"
						case 5:queueKey(0xA1,keyPressed);break;//'
						case 6:queueKey(0x91,keyPressed);break;//(
						case 7:queueKey(0x81,keyPressed);break;//-
						case 8:queueKey(0x71,keyPressed);break;//è
						case 9:queueKey(0x61,keyPressed);break;//_=!
						case 10:queueKey(0x51,keyPressed);break;//ç
						case 11:queueKey(0xD3,keyPressed);break;//à
						case 12:queueKey(0x37,keyPressed);break;//)
						
						case 43:queueKey(0x1F,keyPressed);break;//*
						case 40:queueKey(0x1D,keyPressed);break;//ù=#
						
						case 328:queueKey(0x5B,keyPressed);break;//flèche haut
						case 331:queueKey(0x5D,keyPressed);break;//flèche gauche
						case 336:queueKey(0x5F,keyPressed);break;//flèche bas
						case 333:queueKey(0x3F,keyPressed);break;//flèche droite
						
						case 42:queueKey(0xAF,keyPressed);break;//shift
						case 58:queueKey(0x9F,keyPressed);break;//min/maj
						case 29:queueKey(0x9D,keyPressed);break;//ctrl
						
						case 56:queueKey(0xA3,keyPressed);break;//alt=fnct
						case 1:queueKey(0xA5,keyPressed);break;//échap=Esc
						
						case 41:queueKey(0x55,keyPressed);break;//²=on/off
						case 13:queueKey(0x31,keyPressed);break;//==mem
						case 15:queueKey(0xB5,keyPressed);break;//tab=Connex/Fin
						case 78:queueKey(0x23,keyPressed);break;//numpad +=HP+
						case 74:queueKey(0x13,keyPressed);break;//numpad -=HP-
						case 14:queueKey(0x35,keyPressed);break;//backspace=HP
						case 55:queueKey(0x1F,keyPressed);break;//numpad *=*
						case 309:queueKey(0x1D,keyPressed);break;//numpad /=#
						//.../... bis repertoire annuaire decrochage
						
						case 60:queueKey(0x95,keyPressed);break;//F2=Sommaire
						case 61:queueKey(0x93,keyPressed);break;//F3=Guide
						case 62:queueKey(0x85,keyPressed);break;//F4=Annulation
						case 63:queueKey(0x83,keyPressed);break;//F5=Correction
						case 64:queueKey(0x75,keyPressed);break;//F6=Retour
						case 65:queueKey(0x73,keyPressed);break;//F7=Suite
						case 66:queueKey(0x65,keyPressed);break;//F8=Répétition
						case 67:queueKey(0x63,keyPressed);break;//F9=Envoi
						
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
			printf("keyboard serial in %02X%02X\n",this->cmd_p1,this->cmd_p2);
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
					case 0x05:printf("speaker activated\n");break;
					case 0x07:printf("speaker deactivated\n");break;
					case 0x09:printf("kb cmd 0x09 ????\n");break;
					case 0x0B:printf("kb cmd 0x0B ????\n");break;
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
					
					case 0x21:printf("power off speaker led\n");break;
					case 0x23:printf("power off on/off led\n");break;
					case 0x29:printf("power on speaker led\n");break;
					case 0x2B:printf("power on on/off led\n");break;
					case 0x33:printf("blink on/off led\n");break;
					
					case 0x41:printf("set speaker volume 1\n");break;
					case 0x43:printf("set speaker volume 2\n");break;
					case 0x45:printf("set speaker volume 3\n");break;
					case 0x47:printf("set speaker volume 4\n");break;
					case 0x49:printf("set ringtone volume 1\n");break;
					case 0x4B:printf("set ringtone volume 2\n");break;
					case 0x4D:printf("set ringtone volume 3\n");break;
					case 0x4F:printf("set ringtone volume 4\n");break;
					
					case 0x81:printf("set ringtone 1\n");break;
					case 0x83:printf("set ringtone 2\n");break;
					case 0x85:printf("set ringtone 3\n");break;
					case 0x87:printf("set ringtone 4\n");break;
					case 0x89:printf("set ringtone 5\n");break;
					case 0x8B:printf("play ringtone\n");break;
					
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
};
#endif