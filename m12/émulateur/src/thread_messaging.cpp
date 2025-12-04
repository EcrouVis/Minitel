#include "thread_messaging.h"

Mailbox::Mailbox(){
	Node* n=new Node;//dummy node
	this->first_ms.store(n,std::memory_order_release);
	this->last_ms.store(n,std::memory_order_release);
}
void Mailbox::send(thread_message* ms){
	Node* n=new Node(ms);
	Node* old_last_ms=this->last_ms.exchange(n);//set message as the new last message and retrieve the old last message
	old_last_ms->next.store(n,std::memory_order_release);//add the new message to the mailbox for reception - should be executed last
}
bool Mailbox::receive(thread_message* ms){
	Node* old_first_ms=this->first_ms.exchange(NULL);
	if (old_first_ms==NULL) return false;//another thread is checking if there is a message - mailbox unavailable (work like mutex.test_lock)
	Node* new_first_ms=old_first_ms->next.load(std::memory_order_acquire);
	if (new_first_ms==NULL){//there is no new message
		this->first_ms.store(old_first_ms,std::memory_order_release);//free the mailbox reception
		return false;
	}
	else{//there is a new message
		this->first_ms.store(new_first_ms,std::memory_order_release);//update queue + free the mailbox reception
		memcpy(ms,&(new_first_ms->ms),sizeof(struct thread_message));//read message
		delete old_first_ms;
		return true;
	}
}