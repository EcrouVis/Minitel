#include "FileAccess.h"

void _unloadM(FILE** ppf){
	if (*ppf!=NULL){
		fclose(*ppf);
		*ppf=NULL;
	}
}
void unloadM(std::mutex* mtx, FILE** ppf){
	const std::lock_guard<std::mutex> lock(*mtx);
	_unloadM(ppf);
}

void loadROM(std::mutex* mtx, FILE** ppf,const char* filename){
	const std::lock_guard<std::mutex> lock(*mtx);
	
	_unloadM(ppf);
	*ppf=fopen(filename,"rb");
}

void loadRAM(std::mutex* mtx, FILE** ppf,const char* filename){
	const std::lock_guard<std::mutex> lock(*mtx);
	
	_unloadM(ppf);
	*ppf=fopen(filename,"rb+");
}

void readM(std::mutex* mtx, FILE* pf, unsigned char* array, size_t size){
	const std::lock_guard<std::mutex> lock(*mtx);
	
	std::fill_n(array,size,0);
	if (pf!=NULL){
		fseek(pf,0,SEEK_SET);
		fread(array,sizeof(char),size,pf);
	}
}

void writeM(std::mutex* mtx, FILE* pf, unsigned char* array, size_t size){
	const std::lock_guard<std::mutex> lock(*mtx);
	
	if (pf!=NULL){
		fseek(pf,0,SEEK_SET);
		fwrite(array,sizeof(char),size,pf);
	}
}