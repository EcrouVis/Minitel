#include <limits.h>
#include <filesystem>
//TODO: rewrite / could be wrong for certain cases
#if defined(_WIN32)
#include <winsock2.h>
//include winsock2 before windows to avoid issues
#include <windows.h>
#include <shlobj.h>
#elif defined(__APPLE__) && defined(__MACH__)
#include <mach-o/dyld.h>
#elif defined(unix) || defined(__unix__)
#include <stdlib.h>
#include <unistd.h>
#endif
#include <cstdlib>

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
	std::filesystem::path wd;
	const char* data = getenv("XDG_DATA_HOME");
	if (data!=NULL){
		wd=std::filesystem::path(data);
			wd/=std::filesystem::path("M12");
	}
	else{
		const char* home = getenv("HOME");
		if (home==NULL) exit(-1);//The POSIX specification requires the OS to set a value for $HOME
		else{
			wd=std::filesystem::path(home);
			wd/=std::filesystem::path(".local/share/M12");
		}
	}
	wd=wd.make_preferred();
	std::filesystem::create_directories(wd);

	//if appimage ensure that the included files are installed (Bz6 ROM + default charset)
#ifdef M12_APPIMAGE
	std::filesystem::path ro_path=std::filesystem::path("./../share/M12");
	if (std::filesystem::exists(ro_path)&&std::filesystem::is_directory(ro_path)){
		std::filesystem::copy(
			ro_path,
			wd,
			std::filesystem::copy_options::skip_existing|std::filesystem::copy_options::recursive
		);
	}
#endif
	
	std::filesystem::current_path(wd);
}
#elif defined(_WIN32)
void setupWorkingDirectory(){
	std::filesystem::path wd;
#ifdef M12_INSTALLED
	TCHAR data[MAX_PATH];
	if ( !SUCCEEDED( SHGetFolderPath( NULL, CSIDL_LOCAL_APPDATA, NULL, 0, data ) ) ) exit(-1);
	wd=std::filesystem::path(data);
	wd/=std::filesystem::path("M12");
	wd=wd.make_preferred();
	std::filesystem::create_directories(wd);
	
	std::filesystem::path ro_path=std::filesystem::path("./data");
	if (std::filesystem::exists(ro_path)&&std::filesystem::is_directory(ro_path)){
		std::filesystem::copy(
			ro_path,
			wd,
			std::filesystem::copy_options::skip_existing|std::filesystem::copy_options::recursive
		);
	}
	
#else
	wd=std::filesystem::path("./data");
#endif
	std::filesystem::current_path(wd);
}
#else
void setupWorkingDirectory(){}
#endif