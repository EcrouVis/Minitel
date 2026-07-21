#include <thread>
#include "m12_threads.h"
#include <stdlib.h>
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
	
	setLocalDirectoryExe();//set the working directory to the directory where the executable really exist (resolve symlink + move working directory if it is not the local directory)
	
	setupWorkingDirectory();
	
	GlobalState gState;
	Mailbox mb_circuit;
	Mailbox mb_video;
	
	ix::initNetSystem();
	
	std::thread thrd_e(thread_circuit_main,&mb_circuit,&mb_video,&gState);
	
	thread_video_main(&mb_circuit,&mb_video,&gState);
	
	thrd_e.join();
	
	ix::uninitNetSystem();
	
    exit(EXIT_SUCCESS);
}