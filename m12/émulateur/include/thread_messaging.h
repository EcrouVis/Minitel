#ifndef THREAD_MESSAGING_H
#define THREAD_MESSAGING_H
#include "GlobalState.h"
#include <atomic>
#include <cstring>

struct thread_message{
	int cmd;
	void* p;
};

constexpr int ERAM=1;
constexpr int NOTIFICATION=3;
constexpr int NOTIFICATION_BUZZER=4;
constexpr int NOTIFICATION_REBOOT=15;
constexpr int NOTIFICATION_RED=5;
constexpr int NOTIFICATION_GREEN=6;
constexpr int NOTIFICATION_BLUE=7;
constexpr int NOTIFICATION_ORANGE=8;
constexpr int NOTIFICATION_YELLOW=9;
constexpr int NOTIFICATION_CYAN=10;
constexpr int NOTIFICATION_PURPLE=11;
constexpr int EROM=2;
constexpr int UC=12;
constexpr int VC=13;
constexpr int CPLD=14;
constexpr int KEYBOARD=18;
constexpr int MODEM=19;
constexpr int CLOCK=20;
constexpr int EMULATOR_READY=21;
//constexpr int BUZZER_NOTIFICATION_CONTROL=22;
//constexpr int DIN5_INTERFACE_LOCAL_WEBSOCKET=23;
constexpr int CRT_BUFFER=24;
constexpr int AUDIO_BUFFER=25;
constexpr int PRINTER=27;
constexpr int PRINT_FINISHED=28;
constexpr int SPEAKER_FILTER=29;
constexpr int BUZZER_FILTER=30;
constexpr int AUTO_START_MODULE=31;
constexpr int WEBSOCKET_DIN=32;

/*constexpr int DUMP_ERAM=1;
constexpr int LOAD_ERAM=1<<1;
constexpr int LOAD_EROM=1<<2;*/
constexpr int EMU_ON=1<<3;
constexpr int EMU_OFF=1<<4;
constexpr int EMU_NEXT_STEP=1<<5;
constexpr int SPECIAL=1<<6;//for testing purpose
constexpr int EMU_SHUTDOWN=1<<7;
/*
constexpr int RESTART;*/


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