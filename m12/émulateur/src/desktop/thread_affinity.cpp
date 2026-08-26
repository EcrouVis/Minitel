#if defined(_WIN32)
#include <windows.h>
#else
#include <pthread.h>
#endif
#include <thread>

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

long resetCurrentThreadAffinity(){
	unsigned int n_th=std::thread::hardware_concurrency();
	if (n_th==0) return -1;
#if defined(_WIN32)
	DWORD_PTR mask=(1<<n_th)-1;
	DWORD dw=SetThreadAffinityMask(GetCurrentThread(), mask);
	if (dw==0){
		return GetLastError();
	}
	return 0;
#elif defined(unix) || defined(__unix__)
	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
	for (unsigned int i=0; i<n_th; i++){
		CPU_SET(i,&cpuset);
	}
	return pthread_setaffinity_np(pthread_self(),sizeof(cpuset),&cpuset);
#else
	return 0;
#endif
}