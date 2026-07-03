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
#include <stdlib.h>
#include <unistd.h>
#endif

constexpr char DATA_PATH[]=".";
constexpr char ROM_PATH[]="./rom";
constexpr char RAM_PATH[]="./profils";
constexpr char SCREENSHOT_PATH[]="./captures d'écran";
constexpr char RESOURCE_PATH[]="./ressources";
#if defined(_WIN32)

#elif defined(unix) || defined(__unix__)

constexpr char RO_DATA_PATH[]="./../share";
constexpr char HOME_DATA_PATH[]=".config/M12";
constexpr char FALLBACK_DATA_PATH[]="/var/lib/M12";

#endif

void setLocalDirectoryExe();

void setupWorkingDirectory();
#endif