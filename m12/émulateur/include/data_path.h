#ifndef DATA_PATH_H
#define DATA_PATH_H

constexpr char DATA_PATH[]=".";
constexpr char ROM_PATH[]="./rom";
constexpr char RAM_PATH[]="./profils";
constexpr char SCREENSHOT_PATH[]="./captures d'écran";
constexpr char RESOURCE_PATH[]="./ressources";
#if defined(_WIN32)

#elif defined(unix) || defined(__unix__)

constexpr char RO_DATA_PATH[]="./../share/M12";
constexpr char HOME_DATA_PATH[]=".config/M12";
constexpr char FALLBACK_DATA_PATH[]="/var/lib/M12";

#endif

void setLocalDirectoryExe();

void setupWorkingDirectory();
#endif