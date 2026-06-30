#include <thread>
#include "m12_threads.h"
#include <stdlib.h>
//#include <pthread.h>
#include <cstdio>
#include <locale.h>

#include <ixwebsocket/IXNetSystem.h>

//after because winsock2 should be include before windows
#include "data_path.h"

/*extern "C" {
    __declspec(dllexport) unsigned long NvOptimusEnablement = 1;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}*/

int main(void){
	setlocale(LC_ALL, "fr_FR.UTF-8");
	setlocale(LC_NUMERIC, "en_US.UTF-8");
	
	setLocalDirectory();//set the working directory to the directory where the executable really exist (resolve symlink + move working directory if it is not the local directory)
	
	GlobalState gState;
	Mailbox mb_circuit;
	//Mailbox mb_audio;
	Mailbox mb_video;
	
	ix::initNetSystem();
	
	std::thread thrd_v(thread_video_main,&mb_circuit,&mb_video,&gState);
	//std::thread thrd_a(thread_audio_main,&mb_circuit,&mb_audio,&gState);
	
	//set thread priority
	/*sched_param sch;
    int policy;
    pthread_getschedparam(pthread_self(), &policy, &sch);
	int max_priority = sched_get_priority_max(policy);
	sch.sched_priority = max_priority;
    int result = pthread_setschedparam(pthread_self(), SCHED_OTHER, &sch);
	printf("sched %i\n",result);*/
	
	thread_circuit_main(&mb_circuit,&mb_video,&gState);
	
	//thrd_a.join();
	thrd_v.join();
	
	ix::uninitNetSystem();
	
    exit(EXIT_SUCCESS);
}