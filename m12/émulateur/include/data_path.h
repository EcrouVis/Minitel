#ifndef DATA_PATH_H
#define DATA_PATH_H
#include <limits.h>
#include <filesystem>
//TODO: rewrite / could be wrong for certain cases
#if defined(_WIN32)
#include <Windows.h>
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
constexpr char DATA_PATH[]="./../com/M12";
constexpr char ROM_PATH[]="./../com/M12/rom";
constexpr char RAM_PATH[]="./../com/M12/profils";
constexpr char SCREENSHOT_PATH[]="./../com/M12/captures d'écran";
constexpr char RESOURCE_PATH[]="./../com/M12/ressources";
#endif

void setLocalDirectory();
#endif