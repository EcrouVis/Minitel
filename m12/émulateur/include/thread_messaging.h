#ifndef THREAD_MESSAGING_H
#define THREAD_MESSAGING_H
#include "GlobalState.h"
#include <mutex>
#include <queue>
#include <atomic>

struct thread_message{
	int cmd;
	void* p;
};

struct thread_mailbox{
	std::mutex m;
	std::queue<thread_message> mailbox;
};

void thread_send_message(thread_mailbox*,thread_message*);

long thread_receive_message(thread_mailbox*,thread_message*);

const int ERAM=1;
const int NOTIFICATION=3;
const int NOTIFICATION_BUZZER=4;
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
/*const int LOAD_VRAM;//->custom first screen?
const int SET_DUMP_DIRECTORY;
const int START;
const int STOP;
const int NEXT_TICK;
const int NEXT_INSTRUCTION;
const int RESTART;*/

#endif