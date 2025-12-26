#ifndef THREAD_MESSAGING_H
#define THREAD_MESSAGING_H
#include "GlobalState.h"
#include <atomic>
#include <cstring>

struct thread_message{
	int cmd;
	void* p;
};

const int ERAM=1;
const int NOTIFICATION=3;
const int NOTIFICATION_BUZZER=4;
const int NOTIFICATION_REBOOT=15;
const int NOTIFICATION_RED=5;
const int NOTIFICATION_GREEN=6;
const int NOTIFICATION_BLUE=7;
const int NOTIFICATION_ORANGE=8;
const int NOTIFICATION_YELLOW=9;
const int NOTIFICATION_CYAN=10;
const int NOTIFICATION_PURPLE=11;
const int EROM=2;
const int UC=12;
const int VC=13;
const int CPLD=14;

const int DUMP_ERAM=1;
const int LOAD_ERAM=1<<1;
const int LOAD_EROM=1<<2;
const int EMU_ON=1<<3;
const int EMU_OFF=1<<4;
const int EMU_NEXT_STEP=1<<5;
const int KEYBOARD_STATE_UPDATE=1<<6;
/*
const int RESTART;*/

const int BUZZER_AMPLITUDE=1;

class Mailbox{
	public:
		Mailbox();
		void send(thread_message* ms);
		bool receive(thread_message* ms);
	private:
		struct Node{
			thread_message ms;
			std::atomic<Node*> next=NULL;
			Node(thread_message* ms=NULL){if (ms!=NULL) memcpy(&(this->ms),ms,sizeof(struct thread_message));}
		};
		std::atomic<Node*> first_ms;
		std::atomic<Node*> last_ms;
		
};

#endif