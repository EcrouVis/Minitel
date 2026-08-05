#if defined(_WIN32)
#include <windows.h>
#else
#include <pthread.h>
#endif

long setCurrentThreadAffinity(long i){
#if defined(_WIN32)
	DWORD dw=SetThreadAffinityMask(GetCurrentThread(), DWORD_PTR(1)<<i);
	if (dw==0){
		return GetLastError();
	}
	return 0;
#elif defined(unix) || defined(__unix__)
	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
	CPU_SET(i,&cpuset);
	return pthread_setaffinity_np(pthread_self(),sizeof(cpuset),&cpuset);
#else
	return 0;
#endif
}

long getCurrentCPU(){
#if defined(_WIN32)
	return GetCurrentProcessorNumber();
#elif defined(unix) || defined(__unix__)
	long cpu=sched_getcpu();
	if (cpu<0) cpu=0;
	return cpu;
#else
	return 0;
#endif
}