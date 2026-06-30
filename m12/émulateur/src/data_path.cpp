#include "data_path.h"

void setLocalDirectory(){
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