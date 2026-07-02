#ifndef DATA_PATH_H
#define DATA_PATH_H
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
#include <unistd.h>
#endif

#if defined(_WIN32)
constexpr char DATA_PATH[]=".";
constexpr char ROM_PATH[]="./rom";
constexpr char RAM_PATH[]="./profils";
constexpr char SCREENSHOT_PATH[]="./captures d'écran";
constexpr char RESOURCE_PATH[]="./ressources";
#else
constexpr char DATA_PATH[]="./../share/M12";
constexpr char ROM_PATH[]="./../share/M12/rom";
constexpr char RAM_PATH[]="./../share/M12/profils";
constexpr char SCREENSHOT_PATH[]="./../share/M12/captures d'écran";
constexpr char RESOURCE_PATH[]="./../share/M12/ressources";
#endif

void setLocalDirectory();
#endif