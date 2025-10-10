#ifndef THREAD_MESSAGING_H
#define THREAD_MESSAGING_H
#include <mutex>
#include <queue>
struct thread_message{
	int cmd;
	int d1;
	int d2;
};

struct thread_mailbox{
	std::mutex m;
	std::queue<thread_message> mailbox;
};

void thread_send_message(thread_mailbox*,thread_message*);

long thread_receive_message(thread_mailbox*,thread_message*);
#endif