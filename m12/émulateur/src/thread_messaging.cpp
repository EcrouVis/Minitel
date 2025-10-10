#include "thread_messaging.h"
#include <cstring>

void thread_send_message(thread_mailbox* mb,thread_message* msg){
	mb->m.lock();
	mb->mailbox.push(*msg);
	mb->m.unlock();
}

long thread_receive_message(thread_mailbox* mb,thread_message* msg){
	mb->m.lock();
	long n=(long)mb->mailbox.size();
	if (n>0){
		thread_message msg_r=mb->mailbox.front();
		mb->mailbox.pop();
		memcpy(msg,&msg_r,sizeof(thread_message));
	}
	mb->m.unlock();
	return n-1;
}