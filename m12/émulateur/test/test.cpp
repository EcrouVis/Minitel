#include <thread>
#include "m12_threads.h"
#include <stdlib.h>
#include <pthread.h>
#include <cstdio>

#include <ixwebsocket/IXNetSystem.h>

extern "C" {
    __declspec(dllexport) unsigned long NvOptimusEnablement = 1;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

int main(void){
	GlobalState gState;
	Mailbox mb_circuit;
	Mailbox mb_audio;
	Mailbox mb_video;
	Mailbox mb_log;
	
	ix::initNetSystem();
	
	std::thread thrd_v(thread_video_main,&mb_circuit,&mb_video,&gState);
	std::thread thrd_a(thread_audio_main,&mb_circuit,&mb_audio,&gState);
	std::thread thrd_l(thread_log_main,&mb_log,&gState);
	
	sched_param sch;
    int policy;
    pthread_getschedparam(pthread_self(), &policy, &sch);
	int max_priority = sched_get_priority_max(policy);
	sch.sched_priority = max_priority;
    int result = pthread_setschedparam(pthread_self(), SCHED_OTHER, &sch);
	printf("sched %i\n",result);
	
	thread_circuit_main(&mb_circuit,&mb_video,&mb_audio,&mb_log,&gState);
	
	thrd_l.join();
	thrd_a.join();
	thrd_v.join();
	
	ix::uninitNetSystem();
	
    exit(EXIT_SUCCESS);
}