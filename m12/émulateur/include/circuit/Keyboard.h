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
		}
		void KeyboardChangeIn(keyboard_message* kb_m){
			if (kb_m->focus){
				if (kb_m->scancode==74&&kb_m->action==GLFW_PRESS){//-=take phone
					if (this->line_closed){
						this->SBUF_out_queue.push(0x3C);
						this->SBUF_out_queue.push(0x61);
						this->line_closed=false;
					}
					else{
						this->SBUF_out_queue.push(0x5C);
						this->SBUF_out_queue.push(0x63);
						this->line_closed=true;
					}
					return;
				}
				unsigned char c=0;
				switch (kb_m->scancode){
					case 16:c=0xBB;break;//A
					case 17:c=0xB7;break;//Z
					case 18:c=0xB9;break;//E
					case 19:c=0xA9;break;//R
					case 20:c=0x97;break;//T
					case 21:c=0x87;break;//Y
					case 22:c=0x77;break;//U
					case 23:c=0x67;break;//I
					case 24:c=0x69;break;//O
					case 25:c=0x57;break;//P
					case 30:c=0xBF;break;//Q
					case 31:c=0xBD;break;//S
					case 32:c=0xAB;break;//D
					case 33:c=0x99;break;//F
					case 34:c=0x8B;break;//G
					case 35:c=0x89;break;//H
					case 36:c=0x79;break;//J
					case 37:c=0x6B;break;//K
					case 38:c=0x59;break;//L
					case 39:c=0x3B;break;//M
					case 44:c=0xAD;break;//W
					case 45:c=0x9B;break;//X
					case 46:c=0x8D;break;//C
					case 47:c=0x8F;break;//V
					case 48:c=0x7D;break;//B
					case 49:c=0x7B;break;//N
					case 41:c=0x55;break;//²=on/off
					case 42:c=0xAF;break;//shift
					case 58:c=0x9F;break;//min/maj
					case 29:c=0x9D;break;//ctrl
					case 86:c=0xA3;break;//<>=fnct
					case 82:c=0x2F;break;//0
					case 79:c=0x27;break;//1
					case 80:c=0x17;break;//2
					case 81:c=0x21;break;//3
					case 75:c=0x19;break;//4
					case 76:c=0x29;break;//5
					case 77:c=0x11;break;//6
					case 71:c=0x2B;break;//7
					case 72:c=0x1B;break;//8
					case 73:c=0x2D;break;//9
					case 78:c=0x31;break;//+=mem
					case 55:c=0x95;break;//*=Sommaire
					case 309:c=0x63;break;///=Envoi
					case 328:c=0x5B;break;//flèche haut
					case 331:c=0x5D;break;//flèche gauche
					case 336:c=0x5F;break;//flèche bas
					case 333:c=0x3F;break;//flèche droite
					case 57:c=0x7F;break;//espace
					case 28:c=0x39;break;//entrée
					case 67:c=0xB5;break;//+/-=Connex/Fin
					
				}
				if ((bool)(c&1)&&(kb_m->action==GLFW_PRESS||kb_m->action==GLFW_RELEASE)){
					unsigned char a=(kb_m->action==GLFW_PRESS)?0x00:0x08;
					a|=(a+c+(c>>4))<<4;
					this->SBUF_out_queue.push(a);
					this->SBUF_out_queue.push(c);
				}
			}
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
					this->SBUF_out_queue.push(0xBC);
					this->SBUF_out_queue.push(0xE1);
				}
				this->S_in_low=0;
			}
			this->S_in=b;
		}
	private:
		bool line_closed=false;
	
		bool S_in=false;
		unsigned char S_in_step=0;
		unsigned char S_in_low=0;
		unsigned char S_out_step=0;
		unsigned char SBUF_in=0;
		unsigned char SBUF_out=0;
		std::queue<unsigned char> SBUF_out_queue;
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
		void executeCommand(){
			if (this->cmd_p1==0xC4&&this->cmd_p2==0x17){
				if (this->line_closed){
					this->SBUF_out_queue.push(0x5C);
					this->SBUF_out_queue.push(0x63);
				}
				else{
					this->SBUF_out_queue.push(0x3C);
					this->SBUF_out_queue.push(0x61);
				}
			}
		}
};
#endif