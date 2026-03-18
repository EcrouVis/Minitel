#ifndef FILEACCESS_H
#define FILEACCESS_H
#include <atomic>
#include <cstdio>
#include <mutex>
#include <algorithm>

void unloadM(std::mutex* mtx, FILE** ppf);

void loadROM(std::mutex* mtx, FILE** ppf,const char* filename);

void loadRAM(std::mutex* mtx, FILE** ppf,const char* filename);

void readM(std::mutex* mtx, FILE* pf, unsigned char* array, size_t size);

void writeM(std::mutex* mtx, FILE* pf, unsigned char* array, size_t size);

#endif