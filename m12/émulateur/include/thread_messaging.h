#ifndef THREAD_MESSAGING_H
#define THREAD_MESSAGING_H
#include "GlobalState.h"
#include <atomic>
#include <cstring>

struct thread_message{
	int cmd;
	void* p;
};

enum emulator_video{
	NOTIFICATION_BUZZER,
	NOTIFICATION_REBOOT,
	NOTIFICATION_RED,
	NOTIFICATION_GREEN,
	NOTIFICATION_BLUE,
	NOTIFICATION_ORANGE,
	NOTIFICATION_YELLOW,
	NOTIFICATION_CYAN,
	NOTIFICATION_PURPLE,
	ERAM,
	EROM,
	UC,
	VC,
	CPLD,
	KEYBOARD,
	MODEM,
	CLOCK,
	EMULATOR_READY,
	CRT_BUFFER,
	AUDIO_BUFFER,
	PRINTER,
	PRINT_FINISHED,
	SPEAKER_FILTER,
	BUZZER_FILTER,
	AUTO_START_MODULE,
	WEBSOCKET_DIN,
	AUDIO_PHONE_LINE_ON,
	AUDIO_PHONE_LINE_OFF,
	PHONE_LINE_BUFFER,
	CLOCK_UNRESPONSIVE
};

enum video_emulator{
	EMU_ON,
	EMU_OFF,
	EMU_NEXT_STEP,
	SPECIAL,//for testing purpose
	EMU_SHUTDOWN,
	AUDIO_PHONE_LINE_CALL
};
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