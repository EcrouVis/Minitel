#ifndef DATA_PATH_H
#define DATA_PATH_H

constexpr char DATA_PATH[]=".";
constexpr char ROM_PATH[]="./rom";
constexpr char RAM_PATH[]="./profils";
constexpr char SCREENSHOT_PATH[]="./captures d'écran";
constexpr char RESOURCE_PATH[]="./ressources";

void setLocalDirectoryExe();

void setupWorkingDirectory();
#endif