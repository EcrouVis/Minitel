#include <thread>
#include "m12_threads.h"
#include <stdlib.h>

extern "C" {
    __declspec(dllexport) unsigned long NvOptimusEnablement = 1;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

int main(void){
	GlobalState gState;
	thread_mailbox mb_circuit;
	thread_mailbox mb_audio;
	thread_mailbox mb_video;
	thread_mailbox mb_log;
	std::thread thrd_v(thread_video_main,&mb_circuit,&mb_video,&gState);
	std::thread thrd_a(thread_audio_main,&mb_circuit,&mb_audio,&gState);
	std::thread thrd_l(thread_log_main,&mb_log,&gState);
	thread_circuit_main(&mb_circuit,&mb_video,&mb_audio,&mb_log,&gState);
	
	thrd_l.join();
	thrd_a.join();
	thrd_v.join();
	
    exit(EXIT_SUCCESS);
}