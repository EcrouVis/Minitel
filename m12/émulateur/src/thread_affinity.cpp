#if defined(_WIN32)
#include <windows.h>
#else
#include <pthread.h>
#endif

long setCurrentThreadAffinity(int i){
#if defined(_WIN32)
	DWORD dw=SetThreadAffinityMask(GetCurrentThread(), DWORD_PTR(1)<<i);
	if (dw==0){
		return GetLastError();
	}
	return 0;
#else
	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
	CPU_SET(i,&cpuset);
	return pthread_setaffinity_np(pthread_self(),sizeof(cpuset),&cpuset);
#endif
}