#include <limits.h>
#include <filesystem>
//TODO: rewrite / could be wrong for certain cases
#if defined(_WIN32)
#include <winsock2.h>
//include winsock2 before windows to avoid issues
#include <windows.h>
#elif defined(__APPLE__) && defined(__MACH__)
#include <mach-o/dyld.h>
#elif defined(unix) || defined(__unix__)
#include <stdlib.h>
#include <unistd.h>
#endif

#include "data_path.h"

void setLocalDirectoryExe(){
	std::filesystem::path exePath;
	ssize_t exePathSize=0;
#if defined(_WIN32)

	ssize_t buffer_size=MAX_PATH+1;
	wchar_t* path=NULL;
	do{
		if (path!=NULL){
			free(path);
			buffer_size=buffer_size*2;
		}
		path=(wchar_t*)malloc(sizeof(wchar_t)*buffer_size);
		exePathSize=GetModuleFileNameW(nullptr, path, buffer_size);
	}
	while (exePathSize==buffer_size);
	if (exePathSize<=0) exePathSize=-1;
	else exePath=std::filesystem::path(path);
	free(path);
	
#elif defined(__APPLE__) && defined(__MACH__)

	ssize_t buffer_size=PATH_MAX+1;
	char* path=(char*)malloc(sizeof(char)*buffer_size);
	if (_NSGetExecutablePath(path,&buffer_size)!=0){
		free(path);
		path=(char*)malloc(sizeof(char)*buffer_size);
		if (_NSGetExecutablePath(path,&buffer_size)!=0) exePathSize=-1;
	}
	if (exePathSize>0) exePath=std::filesystem::path(path);
	free(path);
	
#elif defined(unix) || defined(__unix__)

	ssize_t buffer_size=PATH_MAX+1;
	char* path=NULL;
	do{
		if (path!=NULL){
			free(path);
			buffer_size=buffer_size*2;
		}
		path=(char*)malloc(sizeof(char)*buffer_size);
		exePathSize = readlink("/proc/self/exe", path, buffer_size);
	}
	while (exePathSize==buffer_size);
	if (exePathSize<=0) exePathSize=-1;
	else exePath=std::filesystem::path(path);
	free(path);
	
#endif
	if (exePathSize>0){
		if (is_symlink(exePath)) exePath=read_symlink(exePath);
		std::filesystem::current_path(std::filesystem::path(exePath).parent_path());
	}
}

#if defined(unix) || defined(__unix__)
void setupWorkingDirectory(){
	const char* home = getenv("HOME");
	std::filesystem::path wd;
	if (home==NULL) wd=std::filesystem::path(FALLBACK_DATA_PATH);
	else{
		wd=std::filesystem::path(home);
		wd/=std::filesystem::path(HOME_DATA_PATH);
	}
	
	std::filesystem::create_directories(std::filesystem::path(wd));
	std::filesystem::copy(
		std::filesystem::path(RO_DATA_PATH),
		std::filesystem::path(wd),
		std::filesystem::copy_options::skip_existing|std::filesystem::copy_options::recursive
	);
	
	std::filesystem::current_path(wd);
}
#else
void setupWorkingDirectory(){}
#endif