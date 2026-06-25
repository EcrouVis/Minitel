#ifndef GLOBALSTATE_H
#define GLOBALSTATE_H
#include <atomic>
#include <cstdio>
#include <mutex>

struct GlobalState{
	//std::atomic_bool shutdown=false;
	std::atomic_bool minitelOn=false;
	std::atomic_bool stepByStep=false;
	
	std::mutex* p_thread_mutex;
	FILE* erom=NULL;
	FILE* eram=NULL;
	
	GlobalState(){
		this->p_thread_mutex=new std::mutex();
	}
	~GlobalState(){
		delete p_thread_mutex;
	}
};

#endif