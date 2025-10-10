#include "SRAM_64k.h"
void SRAM_64k::ALChangeIn(unsigned char a){
	this->address&=0xFF00;
	this->address|=(unsigned short)a;
	this->updateState();
}
void SRAM_64k::AHChangeIn(unsigned char a){
	this->address&=0x00FF;
	this->address|=((unsigned short)a)<<8;
	this->updateState();
}
void SRAM_64k::DChangeIn(unsigned char d){
	this->data=d;
	this->updateState();
}
void SRAM_64k::nOEChangeIn(bool b){
	this->nOE=b;
	this->updateState();
}
void SRAM_64k::nWEChangeIn(bool b){
	this->nWE=b;
	this->updateState();
}
void SRAM_64k::nCSChangeIn(bool b){
	this->nCS=b;
	this->updateState();
}
void SRAM_64k::subscribeD(void (*f)(unsigned char)){
	this->sendD=f;
}
bool SRAM_64k::isInput(){
	return !(this->nCS||this->nWE);
}
bool SRAM_64k::isOutput(){
	return this->nWE&&!(this->nCS||this->nOE);
}
void SRAM_64k::updateState(){
	if (this->isInput()){
		this->eRAM[this->address]=this->data;
	}
	else if (this->isOutput()){
		(*(this->sendD))(this->eRAM[this->address]);
	}
}