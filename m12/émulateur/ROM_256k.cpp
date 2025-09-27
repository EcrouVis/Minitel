#include <stdio.h>
#include "ROM_256k.h"

ROM_256k::ROM_256k(FILE* fp){
	fread(rhis->eROM,sizeof(char),16777216,fp);
}
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
void A16ChangeIn(bool b){
	this->address&=0x0002FFFF;
	this->address|=b?0x00010000:0;
	this->updateState();
}
void A17ChangeIn(bool b){
	this->address&=0x0001FFFF;
	this->address|=b?0x00020000:0;
	this->updateState();
}
void nGChangeIn(bool b){
	this->nG=b;
	this->updateState();
}
void SRAM_64k::subscribeD(void (*f)(unsigned char)){
	this->sendD=f;
}
void SRAM_64k::updateState(){
	if (!this->nG){
		(*(this->sendD))(this->eROM[this->address]);
	}
}