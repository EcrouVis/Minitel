#include "data_path.h"

void setLocalDirectoryExe(){//TODO: long path size
	std::filesystem::path exePath;
	ssize_t exePathSize=0;
#if defined(_WIN32)
	wchar_t path[MAX_PATH+1] = { 0 };
	exePathSize=GetModuleFileNameW(nullptr, path, MAX_PATH+1);
	if (exePathSize==MAX_PATH+1) exePathSize=-1;
	else exePath=std::filesystem::path(path);
#elif defined(__APPLE__) && defined(__MACH__)
	char path[PATH_MAX+1];
	exePathSize=PATH_MAX+1;
	if (_NSGetExecutablePath(path,&exePathSize)!=0) exePathSize=-1;
	else exePath=std::filesystem::path(path);
#elif defined(unix) || defined(__unix__)
	char path[PATH_MAX+1];
	exePathSize = readlink("/proc/self/exe", path, PATH_MAX+1);
	if (exePathSize==PATH_MAX+1) exePathSize=-1;
	else exePath=std::filesystem::path(path);
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