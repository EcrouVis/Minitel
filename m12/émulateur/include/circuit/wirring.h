#ifndef WIRRING_H
#define WIRRING_H

typedef struct Port_node{
	void (*f)(unsigned char);
	struct Port_node* next;
} Port_node_t;

typedef struct Pin_node{
	void (*f)(bool);
	struct Pin_node* next;
} Pin_node_t;

class Wire{
	public:
		void updateWire(bool level){
			if (level!=this->level){
				this->level=level;
				this->sendUpdatePin();
			}
		}
		void subscribePin(void (*f)(bool)){
			Pin_node_t* new_node=(Pin_node_t*)malloc(sizeof(Pin_node_t));
			new_node->f=f;
			new_node->next=this->pin;
			this->pin=new_node;
		}
	private:
		bool level=true;
		Pin_node_t* pin;
		void sendUpdatePin(){
			Pin_node_t* node=this->pin;
			while (node!=NULL){
				node->f(this->level);
				node=node->next;
			}
		}
};

class Bus{
	public:
		void updatePinX(unsigned char x,bool level){
			if (level!=this->getWire(x)){
				this->setWire(x,level);
				this->sendUpdatePin(x);
				this->sendUpdatePort();
			}
		}
		void updatePort(unsigned char level){
			unsigned char diff=level^this->level;
			for (int i=0;i<8;i++){
				if ((diff>>i)&0x01!=0){
					this->level^=1<<i;
					this->sendUpdatePin(i);
				}
			}
			if (diff!=0){
				this->sendUpdatePort();
			}
		}
		void subscribePinX(unsigned char x,void (*f)(bool)){
			Pin_node_t* new_node=(Pin_node_t*)malloc(sizeof(Pin_node_t));
			new_node->f=f;
			new_node->next=this->pin[x];
			this->pin[x]=new_node;
		}
		void subscribePort(void (*f)(unsigned char)){
			Port_node_t* new_node=(Port_node_t*)malloc(sizeof(Port_node_t));
			new_node->f=f;
			new_node->next=this->port;
			this->port=new_node;
		}
	private:
		unsigned char default_level;
		unsigned char level;
		Pin_node_t* pin[8];
		Port_node_t* port;
		void sendUpdatePin(unsigned char x){
			Pin_node_t* node=this->pin[x];
			while (node!=NULL){
				node->f(this->getWire(x));
				node=node->next;
			}
		}
		void sendUpdatePort(){
			Port_node_t* node=this->port;
			while (node!=NULL){
				node->f(this->level);
				node=node->next;
			}
		}
		bool getWire(unsigned char x){
			return (bool)((this->level>>x)&0x01);
		}
		void setWire(unsigned char x,bool level){
			unsigned char mask=1<<x;
			this->level&=~mask;
			this->level|=level?mask:0x00;
		}
};

#endif