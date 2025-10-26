#include <stdio.h>
#include "circuit/ROM_256k.h"

void ROM_256k::ALChangeIn(unsigned char a){
	this->address&=0x0003FF00;
	this->address|=(long)a;
	this->updateState();
}
void ROM_256k::AHChangeIn(unsigned char a){
	this->address&=0x000300FF;
	this->address|=((long)a)<<8;
	this->updateState();
}
void ROM_256k::A16ChangeIn(bool b){
	this->address&=0x0002FFFF;
	this->address|=b?0x00010000:0;
	this->updateState();
}
void ROM_256k::A17ChangeIn(bool b){
	this->address&=0x0001FFFF;
	this->address|=b?0x00020000:0;
	this->updateState();
}
void ROM_256k::nGChangeIn(bool b){
	this->nG=b;
	this->updateState();
}
//void ROM_256k::subscribeD(void (*f)(unsigned char)){
void ROM_256k::subscribeD(std::function<void(unsigned char)> f){
	this->sendD=f;
}
void ROM_256k::updateState(){
	if (!this->nG){
		//(*(this->sendD))(this->eROM[this->address].load(std::memory_order_relaxed));
		this->sendD(this->eROM[this->address].load(std::memory_order_relaxed));
		this->last_memory_operation.store(this->address,std::memory_order_relaxed);
	}
}

void ROM_256k::set(unsigned char* array){
	for (int i=0;i<EROM_SIZE;i++){
		this->eROM[i].store(array[i],std::memory_order_relaxed);
	}
}