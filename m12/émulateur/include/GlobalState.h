#ifndef GLOBALSTATE_H
#define GLOBALSTATE_H
#include <atomic>

struct GlobalState{
	std::atomic_bool shutdown=false;
	std::atomic_bool minitelOn=false;
	std::atomic_bool stepByStep=false;
	std::atomic_bool periConnected=false;
	std::atomic_bool modemConnected=false;
};

#endif