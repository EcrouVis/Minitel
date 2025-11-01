#include <limits.h>
#define NIBBLE_MAX 15
#include "circuit/80c32.h"
#include <cstdio>
#include <iostream>

m80C32::m80C32(){
	for (int i=0;i<SFR_SIZE;i++){
		this->SFR[i].store(0xFF,std::memory_order_relaxed);
	}
	this->Reset();
	auto vc=[](unsigned char d){};
	auto vb=[](bool b){};
	this->sendP0=vc;
	this->sendP1=vc;
	this->sendP2=vc;
	this->sendP3=vc;
	this->sendTxD=vb;
	this->sendRxD=vb;
	this->sendALE=vb;
	this->sendnPSEN=vb;
}

/*
=============== Helpers ===============
*/		
void m80C32::bitaddress2address(unsigned char* address, unsigned char* bit){
	*bit=(*address)&0x07;
	if ((bool)((*address)&0x80)){
		*address=(*address)&0xF8;
	}
	else{
		*address=0x20+((*address)>>3);
	}
}
unsigned char m80C32::getBitMask(unsigned char address){
	return 1<<(address&0x07);
}
unsigned char m80C32::getBitDirectAddress(unsigned char address){
	if ((bool)(address&0x80)){
		return address&0xF8;
	}
	else{
		return 0x20+(address>>3);
	}
}
bool m80C32::getBitIn(unsigned char address){
	unsigned char bit;
	this->bitaddress2address(&address,&bit);
	return (bool)((this->getDirectByteIn(address)>>bit)&0x01);
}
bool m80C32::getBitOut(unsigned char address){
	unsigned char bit;
	this->bitaddress2address(&address,&bit);
	return (bool)((this->getDirectByteOut(address)>>bit)&0x01);
}
//change state + callback for PX port change
void m80C32::setBitIn(unsigned char address, bool b){
	unsigned char bit;
	this->bitaddress2address(&address,&bit);
	unsigned char mask=1<<bit;
	if ((bool)(address&0x80)){
		unsigned char v=this->getSFRByteIn(address);
		v&=~mask;
		v|=b?mask:0x00;
		this->setSFRByte(address,v);
	}
	else{
		unsigned char v=this->getRAMByte(address);
		v&=~mask;
		v|=b?mask:0x00;
		this->setRAMByte(address,v);
	}
}
void m80C32::setBitOut(unsigned char address, bool b){
	unsigned char bit;
	this->bitaddress2address(&address,&bit);
	unsigned char mask=1<<bit;
	if ((bool)(address&0x80)){
		unsigned char v=this->getSFRByteOut(address);
		v&=~mask;
		v|=b?mask:0x00;
		this->setSFRByte(address,v);
	}
	else{
		unsigned char v=this->getRAMByte(address);
		v&=~mask;
		v|=b?mask:0x00;
		this->setRAMByte(address,v);
	}
}
unsigned char m80C32::getRAMByte(unsigned char address){
	this->last_memory_operation.store(address,std::memory_order_relaxed);
	return this->iRAM[address].load(std::memory_order_relaxed);
}
unsigned char m80C32::getSFRByteIn(unsigned char address){
	switch (address){
		case this->P0:
		case this->P1:
		case this->P2:
		case this->P3:
		case this->ACC:
		case this->IE:
		case this->IP:
		case this->PCON:
		case this->SBUF:
		case this->B:
		case this->DPH:
		case this->DPL:
		case this->PSW:
		case this->RCAP2H:
		case this->RCAP2L:
		case this->SCON:
		case this->SP:
		case this->TCON:
		case this->T2CON:
		case this->TH0:
		case this->TH1:
		case this->TH2:
		case this->TL0:
		case this->TL1:
		case this->TL2:
		case this->TMOD:
			return this->SFR[address&0x7F].load(std::memory_order_relaxed);
		default:
			printf("read sfr in %02X\n",address);
			return 0xFF;
	}
}
unsigned char m80C32::getSFRByteOut(unsigned char address){
	switch (address){
		case this->P0:
			return this->PX_out[0];
		case this->P1:
			return this->PX_out[1];
		case this->P2:
			return this->PX_out[2];
		case this->P3:
			return this->PX_out[3];
		case this->ACC:
		case this->IE:
		case this->IP:
		case this->PCON:
		case this->SBUF:
		case this->B:
		case this->DPH:
		case this->DPL:
		case this->PSW:
		case this->RCAP2H:
		case this->RCAP2L:
		case this->SCON:
		case this->SP:
		case this->TCON:
		case this->T2CON:
		case this->TH0:
		case this->TH1:
		case this->TH2:
		case this->TL0:
		case this->TL1:
		case this->TL2:
		case this->TMOD:
			return this->SFR[address&0x7F].load(std::memory_order_relaxed);
		default:
			printf("read sfr out %02X\n",address);
			return 0xFF;
	}
}
unsigned char m80C32::getDirectByteIn(unsigned char address){
	return ((bool)(address&0x80))?this->getSFRByteIn(address):this->getRAMByte(address);
}
unsigned char m80C32::getDirectByteOut(unsigned char address){
	return ((bool)(address&0x80))?this->getSFRByteOut(address):this->getRAMByte(address);
}
//change state + callback for PX port change
void m80C32::setRAMByte(unsigned char address, unsigned char d){
	this->last_memory_operation.store(((unsigned int)address)+IRAM_SIZE,std::memory_order_relaxed);
	this->iRAM[address].store(d,std::memory_order_relaxed);
}
void m80C32::setSFRByte(unsigned char address, unsigned char d){
	switch (address){
		case this->ACC:
			this->SFR[address&0x7F].store(d,std::memory_order_relaxed);
			this->setACCParity();
			break;
		case this->IE:
		case this->IP:
			this->SFR[address&0x7F].store(d,std::memory_order_relaxed);
			this->interrupt_change=true;
			break;
		case this->P0:
			this->PX_out[0]=d;
			if ((this->getSFRByteIn(address)^d)!=0){
				this->sendP0(d);
			}
			break;
		case this->P1:
			this->PX_out[1]=d;
			if ((this->getSFRByteIn(address)^d)!=0){
				this->sendP1(d);
			}
			break;
		case this->P2:
			this->PX_out[2]=d;
			if ((this->getSFRByteIn(address)^d)!=0){
				this->sendP2(d);
			}
			break;
		case this->P3:
			this->PX_out[3]=d;
			if ((this->getSFRByteIn(address)^d)!=0){
				this->sendP3(d);
			}
			break;
		case this->PCON:
			this->SFR[address&0x7F].store(d,std::memory_order_relaxed);
			this->PCONChange();
			break;
		case this->SBUF:
			this->SBUF_out=d;
			this->TEN=true;
			break;
		case this->B:
		case this->DPH:
		case this->DPL:
		case this->PSW:
		case this->RCAP2H:
		case this->RCAP2L:
		case this->SCON:
		case this->SP:
		case this->TCON:
		case this->T2CON:
		case this->TH0:
		case this->TH1:
		case this->TH2:
		case this->TL0:
		case this->TL1:
		case this->TL2:
		case this->TMOD:
			this->SFR[address&0x7F].store(d,std::memory_order_relaxed);
			break;
		default:
			printf("write sfr %02X\n",address);
			break;
	}
}
void m80C32::setDirectByte(unsigned char address, unsigned char d){
	if ((bool)(address&0x80)) this->setSFRByte(address,d);
	else this->setRAMByte(address,d);
}

unsigned char m80C32::getR(unsigned char r){
	return (this->getSFRByteIn(this->PSW)&0x18)|r;
}

/*
=============== IO ===============
*/
void m80C32::CLKTickIn(){
	this->fixedSerialClockTick();
	
	this->period++;
	if ((this->period&0x01)==1) return;// f/2->state time
	
	unsigned char t2con=this->getSFRByteIn(this->T2CON);
	unsigned char t2con_mask1=1<<(this->C_nT2&0x07);
	unsigned char t2con_mask2=(1<<(this->C_nT2&0x07))|(1<<(this->RCLK&0x07))|(1<<(this->TCLK&0x07));
	if ((t2con&t2con_mask1)==0&&(t2con&t2con_mask2)!=0) this->T2Tick();
	
	if (this->period<this->periodPerCycle) return;
	this->period=0;
	if ((t2con&t2con_mask2)==0) this->T2Tick();
	unsigned char tmod=this->getSFRByteIn(this->TMOD);
	if (!(bool)(tmod&(1<<this->C_T_0))) this->T0Tick();
	if (!(bool)(tmod&(1<<this->C_T_1))) this->T1Tick();
	
	this->ResetCountdown();
	unsigned char pd_mask=1<<this->PD;
	unsigned char idl_mask=1<<this->IDL;
	unsigned char power_mode=this->getSFRByteIn(this->PCON)&(pd_mask|idl_mask);
	if ((power_mode&pd_mask)==0){
		if ((power_mode&idl_mask)==0){
			this->nextCycleALU();
		}
	}
	if (this->i_cycle[this->instruction[0]]-this->i_cycle_n<=0){
		this->checkInterrupts();
	}
}
void m80C32::ResetChangeIn(bool level){
	this->reset_level=level;
}
void m80C32::PXChangeIn(unsigned char x,unsigned char d){
	const unsigned char ax[4]={this->P0,this->P1,this->P2,this->P3};
	x=ax[x];
	unsigned char v=this->getSFRByteIn(x);//this->SFR[x&0x7F];
	this->SFR[x&0x7F].store(d,std::memory_order_relaxed);//don't trigger infinite loop + don't write to TX_out
	this->checkPortChangeConsequences(x,v^d);
	
}
void m80C32::PXYChangeIn(unsigned char x,unsigned char y,bool b){
	const unsigned char ax[4]={this->P0,this->P1,this->P2,this->P3};
	x=ax[x];
	y=y&0x07;
	y=1<<y;
	unsigned char v=this->getSFRByteIn(x);//this->SFR[x&0x7F];
	unsigned char d=(v&(~y))|(b?y:0);
	this->SFR[x&0x7F].store(d,std::memory_order_relaxed);//don't trigger infinite loop + don't write to TX_out
	this->checkPortChangeConsequences(x,v^d);
	
}
void m80C32::checkPortChangeConsequences(unsigned char a,unsigned char mask){
	if (a==this->P3){
		mask=mask&(~this->getSFRByteIn(a));//port fall mask
		//nINT0
		//nINT1
		if (((mask>>(this->nINT0&0x07))&0x01)!=0){
			this->nINT0Fall();
		}
		if (((mask>>(this->nINT1&0x07))&0x01)!=0){
			this->nINT1Fall();
		}
		//T0
		//T1
		if (((mask>>(this->T0&0x07))&0x01)!=0){
			unsigned char tmod=this->getSFRByteIn(this->TMOD);
			if ((bool)(tmod&(1<<this->C_T_0))){
				this->T0Tick();
			}
		}
		if (((mask>>(this->T1&0x07))&0x01)!=0){
			unsigned char tmod=this->getSFRByteIn(this->TMOD);
			if ((bool)(tmod&(1<<this->C_T_1))){
				this->T1Tick();
			}
		}
	}
	else if (a==this->P1){
		mask=mask&(~this->getSFRByteIn(a));//port fall mask
		if (((mask>>(this->T2EX&0x07))&0x01)!=0){
			this->T2EXFall();
		}
		if (((mask>>(this->T2&0x07))&0x01)!=0){
			if (this->getBitIn(this->C_nT2)){
				this->T2Tick();
			}
		}
	}
}



/*
=============== POWER + RESET ===============
*/
void m80C32::ResetCountdown(){
	if (this->reset_level){
		if (this->reset_count!=0) this->reset_count--;
	}
	else{
		if (this->reset_count==0){
			this->Reset();
		}
		this->reset_count=2;
	}
}
void m80C32::Reset(){
	//reset registers
	this->PC=0;
	this->setSFRByte(this->ACC,0x00);
	this->setSFRByte(this->B,0x00);
	this->setSFRByte(this->DPH,0x00);
	this->setSFRByte(this->DPL,0x00);
	this->setBitIn(this->EA,false);
	this->setBitIn(this->ET2,false);
	this->setBitIn(this->ES,false);
	this->setBitIn(this->ET1,false);
	this->setBitIn(this->EX1,false);
	this->setBitIn(this->ET0,false);
	this->setBitIn(this->EX0,false);
	this->setBitIn(this->PT2,false);
	this->setBitIn(this->PS,false);
	this->setBitIn(this->PT1,false);
	this->setBitIn(this->PX1,false);
	this->setBitIn(this->PT0,false);
	this->setBitIn(this->PX0,false);
	this->setSFRByte(this->P0,0xFF);
	this->setSFRByte(this->P1,0xFF);
	this->setSFRByte(this->P2,0xFF);
	this->setSFRByte(this->P3,0xFF);
	this->setSFRByte(this->PCON,(this->getSFRByteIn(this->PCON))&(~(1<<(this->SMOD))));
	this->setSFRByte(this->PSW,0x00);
	this->setSFRByte(this->RCAP2H,0x00);
	this->setSFRByte(this->RCAP2L,0x00);
	this->setSFRByte(this->SCON,0x00);
	this->setSFRByte(this->SP,0x07);
	this->setSFRByte(this->TCON,0x00);
	this->setSFRByte(this->T2CON,0x00);
	this->setSFRByte(this->TH0,0x00);
	this->setSFRByte(this->TH1,0x00);
	this->setSFRByte(this->TH2,0x00);
	this->setSFRByte(this->TL0,0x00);
	this->setSFRByte(this->TL1,0x00);
	this->setSFRByte(this->TL2,0x00);
	this->setSFRByte(this->TMOD,0x00);
	
	//reset power state
	this->setSFRByte(this->PCON,this->getSFRByteIn(this->PCON)&(~((1<<this->PD)|(1<<this->IDL))));
	
	//reset pins
	/*(*this->sendALE)(true);
	(*this->sendnPSEN)(true);*/
	this->sendALE(false);
	this->sendnPSEN(true);
	
	//reset ALU
	this->i_cycle_n=0xFF;
	
	//reset interrupts
	this->interrupt_level=0;
	this->interrupt_change=false;
}
void m80C32::Idle(){
	this->sendALE(true);
	this->sendnPSEN(true);
}
void m80C32::PowerDown(){
	/*(*this->sendALE)(false);
	(*this->sendnPSEN)(false);*/
	this->sendALE(false);
	this->sendnPSEN(false);
}
void m80C32::PCONChange(){
	unsigned char P=this->getSFRByteIn(this->PCON);
	if ((P&(1<<this->PD))!=0){
		this->PowerDown();
	}
	else if ((P&(1<<this->IDL))!=0){
		this->Idle();
	}
}

/*
=============== SERIAL + TIMERS/COUNTERS ===============
*/
void m80C32::T2EXFall(){
	if (this->getBitIn(this->EXEN2)){
		this->setBitIn(this->EXF2,true);
		
		bool t2rx=this->getBitIn(this->RCLK);
		bool t2tx=this->getBitIn(this->TCLK);
		if ((!t2rx)&&(!t2tx)){
			if (this->getBitIn(this->CP_nRL2)){//capture mode
				this->setSFRByte(this->RCAP2L,this->getSFRByteIn(this->TL2));
				this->setSFRByte(this->RCAP2H,this->getSFRByteIn(this->TH2));
			}
			else{//auto reload mode
				this->setSFRByte(this->TL2,this->getSFRByteIn(this->RCAP2L));
				this->setSFRByte(this->TH2,this->getSFRByteIn(this->RCAP2H));
			}	
		}
	}
}
void m80C32::T2Tick(){
	if (this->getBitIn(this->TR2)){
		unsigned short t2=(unsigned short)this->getSFRByteIn(this->TL2);
		t2|=((unsigned short)this->getSFRByteIn(this->TH2))<<8;
		t2++;
		if (t2==0){
			bool t2rx=this->getBitIn(this->RCLK);
			bool t2tx=this->getBitIn(this->TCLK);
			if (t2rx||t2tx){//baudrate generator mode
				this->setSFRByte(this->TL2,this->getSFRByteIn(this->RCAP2L));
				this->setSFRByte(this->TH2,this->getSFRByteIn(this->RCAP2H));
				this->T2SerialClockTick();
			}
			else{
				this->setBitIn(this->TF2,true);
				if (!this->getBitIn(this->CP_nRL2)){//auto reload mode
					this->setSFRByte(this->TL2,this->getSFRByteIn(this->RCAP2L));
					this->setSFRByte(this->TH2,this->getSFRByteIn(this->RCAP2H));	
				}
			}
		}
	}
}
void m80C32::nINT0Fall(){
	this->setBitIn(this->IE0,true);
}
void m80C32::nINT1Fall(){
	this->setBitIn(this->IE1,true);
}
void m80C32::T0Tick(){
	unsigned char tmod=this->getSFRByteIn(this->TMOD);
	if (this->getBitIn(this->TR0)&&((!(bool)(tmod&(1<<this->GATE_0)))||this->getBitIn(this->nINT0))){
		bool m0=(bool)(tmod&(1<<this->M0_0));
		bool m1=(bool)(tmod&(1<<this->M1_0));
		if (m0){
			if (m1){//mode 3: split -> 8 bit TL0
				unsigned char tl0=this->getSFRByteIn(this->TL0);
				tl0++;
				if (tl0==0) this->setBitIn(this->TF0,true);
				this->setSFRByte(this->TL0,tl0);
			}
			else{//mode 1: 16 bit
				unsigned short t0=(unsigned short)this->getSFRByteIn(this->TL0);
				t0|=((unsigned short)this->getSFRByteIn(this->TH0))<<8;
				t0++;
				if (t0==0) this->setBitIn(this->TF0,true);
				this->setSFRByte(this->TH0,(unsigned char)(t0>>8));
				this->setSFRByte(this->TL0,(unsigned char)t0);
			}
		}
		else{
			if (m1){//mode 2: 8 bit auto reload
				unsigned char tl0=this->getSFRByteIn(this->TL0);
				tl0++;
				if (tl0==0){
					tl0=this->getSFRByteIn(this->TH0);
					this->setBitIn(this->TF0,true);
				}
				this->setSFRByte(this->TL0,tl0);
			}
			else{//mode 0: 13 bit
				unsigned char tl0=this->getSFRByteIn(this->TL0);
				unsigned short t0=(unsigned short)(tl0<<3);
				t0|=((unsigned short)this->getSFRByteIn(this->TH0))<<8;
				t0+=1<<3;
				if (t0<(1<<3)) this->setBitIn(this->TF0,true);
				this->setSFRByte(this->TH0,(unsigned char)(t0>>8));
				this->setSFRByte(this->TL0,(((unsigned char)(t0>>3))&0x1F)|(tl0&0xE0));
			}
		}
	}
}
void m80C32::T1Tick(){
	unsigned char tmod=this->getSFRByteIn(this->TMOD);
	bool split=((bool)(tmod&(1<<this->M0_1)))&&((bool)(tmod&(1<<this->M1_1)));
	bool inc=this->getBitIn(this->TR1)&&((!(bool)(tmod&(1<<this->GATE_1)))||this->getBitIn(this->nINT1));
	if (split&&inc){//8 bit TH0
		unsigned char th0=this->getSFRByteIn(this->TH0);
		th0++;
		if (th0==0) this->setBitIn(this->TF1,true);
		this->setSFRByte(this->TH0,th0);
	}
	if (split||inc){
		bool m0=(bool)(tmod&(1<<this->M0_1));
		bool m1=(bool)(tmod&(1<<this->M1_1));
		if (m0){
			if (!m1){//mode 1: 16 bit
				unsigned short t1=(unsigned short)this->getSFRByteIn(this->TL1);
				t1|=((unsigned short)this->getSFRByteIn(this->TH1))<<8;
				t1++;
				if (t1==0){
					if (!split) this->setBitIn(this->TF1,true);
					this->T1SerialClockTick();
				}
				this->setSFRByte(this->TH1,(unsigned char)(t1>>8));
				this->setSFRByte(this->TL1,(unsigned char)t1);
			}
		}
		else{
			if (m1){//mode 2: 8 bit auto reload
				unsigned char tl1=this->getSFRByteIn(this->TL1);
				tl1++;
				if (tl1==0){
					tl1=this->getSFRByteIn(this->TH1);
					if (!split) this->setBitIn(this->TF1,true);
					this->T1SerialClockTick();
				}
				this->setSFRByte(this->TL1,tl1);
			}
			else{//mode 0: 13 bit
				unsigned char tl1=this->getSFRByteIn(this->TL1);
				unsigned short t1=(unsigned short)(tl1<<3);
				t1|=((unsigned short)this->getSFRByteIn(this->TH1))<<8;
				t1+=1<<3;
				if (t1<(1<<3)){
					if (!split) this->setBitIn(this->TF1,true);
					this->T1SerialClockTick();
				}
				this->setSFRByte(this->TH1,(unsigned char)(t1>>8));
				this->setSFRByte(this->TL1,(((unsigned char)(t1>>3))&0x1F)|(tl1&0xE0));
			}
		}
	}
}
void m80C32::T2SerialClockTick(){
	if (!this->getBitIn(this->SM1)) return;
	if (this->getBitIn(this->TCLK)){//f_t2/16
		this->tx_prescaler+=12;
		TXClockTickX4();
	}
	if (this->getBitIn(this->RCLK)){//f_t2/16
		this->rx_prescaler+=12;
		RXClockTickX4();
	}
}
void m80C32::T1SerialClockTick(){
	if (!this->getBitIn(this->SM1)) return;
	bool smod=(bool)((this->getSFRByteIn(this->PCON)>>this->SMOD)&0x01);
	if (!this->getBitIn(this->TCLK)){
		if (smod){//f_t1/16
			this->tx_prescaler+=12;
		}
		else{//f_t1/32
			this->tx_prescaler+=6;
		}
		TXClockTickX4();
	}
	if (!this->getBitIn(this->RCLK)){
		if (smod){//f_t1/16
			this->rx_prescaler+=12;
		}
		else{//f_t1/32
			this->rx_prescaler+=6;
		}
		RXClockTickX4();
	}
}
void m80C32::fixedSerialClockTick(){
	if (this->getBitIn(this->SM1)) return;
	if (this->getBitIn(this->SM0)){
		bool smod=(bool)((this->getSFRByteIn(this->PCON)>>this->SMOD)&0x01);
		if (smod){//f/32
			this->tx_prescaler+=6;
			this->rx_prescaler+=6;
		}
		else{//f/64
			this->tx_prescaler+=3;
			this->rx_prescaler+=3;
		}
	}
	else{//f/12
		this->tx_prescaler+=16;
		this->rx_prescaler+=16;
	}
	TXClockTickX4();
	RXClockTickX4();
}
void m80C32::RXClockTickX4(){
	if (this->rx_prescaler<48) return;
	this->rx_prescaler=this->rx_prescaler%48;
	this->rx_subtick++;
	this->rx_subtick&=0x03;
	
	this->updateRX();
}
void m80C32::TXClockTickX4(){
	if (this->tx_prescaler<48) return;
	this->tx_prescaler=this->tx_prescaler%48;
	this->tx_subtick++;
	this->tx_subtick&=0x03;
	
	this->updateTX();
}
void m80C32::updateRX(){
	if (!this->getBitIn(this->REN)) return;
	if (this->getBitIn(this->SM0)||this->getBitIn(this->SM1)){//mode 1 2 3
		if (this->RX_bit>38) this->RX_bit=0;
		bool rx_state;
		unsigned char sbuf;
		bool rb8;
		switch (this->RX_bit){//RX_bit: 0bBBBBBBSS B->bit S->sub bit
			case 0:
				rx_state=this->getBitIn(this->RxD);
				if (this->RX_state&&(!rx_state))this->RX_bit++;//falling edge
				this->RX_state=rx_state;
				break;
			default:
				this->RX_bit++;
				if ((this->RX_bit&0x03)!=2) break;
				sbuf=this->SBUF_in;
				sbuf=sbuf>>1;
				sbuf|=this->getBitIn(this->RxD)?0x80:0x00;
				this->SBUF_in=sbuf;
				break;
			case 38://stop bit/RB8 (n_bit+1)*4-3: 10*4-2
				rb8=this->getBitIn(this->RxD);
				this->setSFRByte(this->SBUF,this->SBUF_in);
				this->setBitIn(this->RB8,rb8);
				rb8=rb8||(!this->getBitIn(this->SM2));
				this->setBitIn(this->RI,rb8);
				this->RX_bit=0;
		}
	}
	else{//mode 0
		if (this->RX_bit>=8) this->RX_bit=0;
		bool rx_state=this->getBitIn(this->TxD);
		if (this->RX_state&&(!rx_state)){
			this->RX_bit++;
			unsigned char sbuf=this->SBUF_in;
			sbuf=sbuf>>1;
			sbuf|=this->getBitIn(this->RxD)?0x80:0x00;
			this->SBUF_in=sbuf;
			if (this->RX_bit>=8){
				this->setSFRByte(this->SBUF,this->SBUF_in);
				this->setBitIn(this->RI,true);
				this->RX_bit=0;
			}
		}
		this->RX_state=rx_state;
	}
}
void m80C32::updateTX(){
	if ((!this->TEN)) return;
	unsigned char n_bit;
	if (this->getBitIn(this->SM0)){//mode 2 3
		if ((this->TX_bit&0x03)!=0) return;
		n_bit=11;
		if (this->TX_bit>=n_bit*4) this->TX_bit=0;
		switch (this->TX_bit>>2){
			case 0:
				//(*this->sendTxD)(false);
				this->sendTxD(false);
				break;
			case 10:
				//(*this->sendTxD)((bool)(this->PX_out[3]&0x02));
				this->sendTxD((bool)(this->PX_out[3]&0x02));
				break;
			case 9:
				//(*this->sendTxD)(this->getBitIn(this->TB8)&&((bool)(this->PX_out[3]&0x02)));
				this->sendTxD(this->getBitIn(this->TB8)&&((bool)(this->PX_out[3]&0x02)));
				break;
			default:
				//(*this->sendTxD)(((bool)(this->SBUF_out&0x01))&&((bool)(this->PX_out[3]&0x02)));
				this->sendTxD(((bool)(this->SBUF_out&0x01))&&((bool)(this->PX_out[3]&0x02)));
				this->SBUF_out=this->SBUF_out>>1;
		}
	}
	else if (this->getBitIn(this->SM1)){//mode 1
		if ((this->TX_bit&0x03)!=0) return;
		n_bit=10;
		if (this->TX_bit>=n_bit*4) this->TX_bit=0;
		switch (this->TX_bit>>2){
			case 0:
				//(*this->sendTxD)(false);
				this->sendTxD(false);
				break;
			case 9:
				//(*this->sendTxD)((bool)(this->PX_out[3]&0x02));
				this->sendTxD((bool)(this->PX_out[3]&0x02));
				break;
			default:
				//(*this->sendTxD)(((bool)(this->SBUF_out&0x01))&&((bool)(this->PX_out[3]&0x02)));
				this->sendTxD(((bool)(this->SBUF_out&0x01))&&((bool)(this->PX_out[3]&0x02)));
				this->SBUF_out=this->SBUF_out>>1;
		}
	}
	else{//mode 0
		n_bit=8;
		if (this->TX_bit>=n_bit*4) this->TX_bit=0;
		if ((this->TX_bit&0x03)==0){
			//(*this->sendRxD)(((bool)(this->SBUF_out&0x01))&&((bool)(this->PX_out[3]&0x01)));
			this->sendRxD(((bool)(this->SBUF_out&0x01))&&((bool)(this->PX_out[3]&0x01)));
			this->setBitIn(this->RxD,(bool)(this->SBUF_out&0x01));
			this->SBUF_out=this->SBUF_out>>1;
			//(*this->sendTxD)((bool)(this->PX_out[3]&0x02));
			this->sendTxD((bool)(this->PX_out[3]&0x02));
		}
		else if ((this->TX_bit&0x03)==2){
			//(*this->sendTxD)(false);
			this->sendTxD(false);
		}
	}
	this->TX_bit++;
	if (this->TX_bit<n_bit*4) return;
	this->TX_bit=0;
	this->TEN=false;
	this->setBitIn(this->TI,true);
	/*(*this->sendTxD)((bool)(this->PX_out[3]&0x02));
	(*this->sendRxD)((bool)(this->PX_out[3]&0x01));*/
	this->sendTxD((bool)(this->PX_out[3]&0x02));
	this->sendRxD((bool)(this->PX_out[3]&0x01));
}

/*
=============== INTERRUPTS ===============
*/
void m80C32::checkInterrupts(){
	if (this->getBitIn(this->EA)&&!this->interrupt_change&&this->instruction[0]!=0x32){
		unsigned char interrupt_signal=this->getBitIn(this->IE0)?0x01:0x00;
		interrupt_signal|=this->getBitIn(this->TF0)?0x02:0x00;
		interrupt_signal|=this->getBitIn(this->IE1)?0x04:0x00;
		interrupt_signal|=this->getBitIn(this->TF1)?0x08:0x00;
		interrupt_signal|=(this->getBitIn(this->RI)||this->getBitIn(this->TI))?0x10:0x00;
		interrupt_signal|=(this->getBitIn(this->TF2)||this->getBitIn(this->EXF2))?0x20:0x00;
		interrupt_signal&=this->getSFRByteIn(this->IE)&0x3F;
		unsigned char ip=this->getSFRByteIn(this->IP);
		
		bool int_high=((this->interrupt_level&ip)!=0);
		if (int_high) interrupt_signal&=ip;
		
		for(int i=0;i<8;i++){
			unsigned char mask=1<<i;
			if ((this->interrupt_level&mask)!=0) break;
			if ((interrupt_signal&mask)!=0){
				this->i_cycle_n=this->i_cycle[0x12];
				this->instruction[0]=0x12;
				this->instruction[1]=0x00;
				switch (i){
					case 0:
						if (this->getBitIn(this->IT0)){
							this->setBitIn(this->IE0,false);//clear when falling edge
						}
						this->instruction[2]=0x03;
						break;
					case 1:
						this->setBitIn(this->TF0,false);
						this->instruction[2]=0x0B;
						break;
					case 2:
						if (this->getBitIn(this->IT1)){
							this->setBitIn(this->IE1,false);
						}
						this->instruction[2]=0x13;
						break;
					case 3:
						this->setBitIn(this->TF1,false);
						this->instruction[2]=0x1B;
						break;
					case 4:
						this->instruction[2]=0x23;
						break;
					case 5:
						this->instruction[2]=0x2B;
						break;
				}
				this->i_part_n=3;
				this->i_cycle_n=0;
				this->interrupt_level|=mask;
				this->setSFRByte(this->PCON,this->getSFRByteIn(this->PCON)&(~(1<<this->IDL)));
				break;
			}
		}
	}
	this->interrupt_change=false;
}
void m80C32::decreaseInterruptLevel(){
	unsigned char il=this->interrupt_level;
	unsigned char ip=this->getSFRByteIn(this->IP);
	if ((il&ip)!=0) il&=ip;
	for(int i=0;i<8;i++){
		if ((bool)((il>>i)&1)){
			this->interrupt_level&=~(1<<i);
			break;
		}
	}
}

/*
=============== ALU ===============
*/
void m80C32::nextCycleALU(){
	if (this->i_cycle_n>=this->i_cycle[this->instruction[0]]){
		this->i_cycle_n=0;
		this->i_part_n=0;
	}
	unsigned char external_rw_action=0;
	while (external_rw_action<2&&this->i_part_n<this->i_length[this->instruction[0]]){
		//printf("PC 0x%04X %02X\n",this->PC,this->PX_out[3]);
		this->sendP0((unsigned char)(this->PC&0xFF));
		this->sendP2((unsigned char)(this->PC>>8));
		this->sendALE(true);
		this->sendALE(false);
		this->sendnPSEN(false);
		this->instruction[this->i_part_n]=this->getSFRByteIn(this->P0);
		this->sendnPSEN(true);
		this->sendP0(this->PX_out[0]);
		this->sendP2(this->PX_out[2]);
		this->PC++;
		this->i_part_n++;
		external_rw_action++;
	}
	this->i_cycle_n++;
	
	if (this->i_cycle[this->instruction[0]]-this->i_cycle_n<=0){
		/*char l=this->i_length[this->instruction[0]];
		printf("instruction ");
		for (int i=0;i<l;i++){
			printf("%02X",this->instruction[i]);
		}
		printf(" 0x%06X %i\n",this->PC-l,l);*/
		//if (this->PC-l==0x74) printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		this->exec_instruction=true;
		this->execInstruction();
	}
	
	
}
void m80C32::setACCParity(){
	unsigned short p=0b0110100110010110;
	unsigned char a=this->getSFRByteIn(this->ACC);
	p=((p>>(a&0x0F))^(p>>(a>>4)))&0x01;
	this->setBitIn(this->P,p!=0);
}
void m80C32::execInstruction(){
	unsigned char d;
	unsigned char a;
	switch (this->instruction[0]){
		default:printf("erreur instruction %02X\n",this->instruction[0]);break;
		case 0xA5:printf("erreur instruction propriétaire\n");break;//reserved
		
		case 0x11:this->ACALL(this->instruction[1]);break;
		case 0x31:this->ACALL(0x0100|this->instruction[1]);break;
		case 0x51:this->ACALL(0x0200|this->instruction[1]);break;
		case 0x71:this->ACALL(0x0300|this->instruction[1]);break;
		case 0x91:this->ACALL(0x0400|this->instruction[1]);break;
		case 0xB1:this->ACALL(0x0500|this->instruction[1]);break;
		case 0xD1:this->ACALL(0x0600|this->instruction[1]);break;
		case 0xF1:this->ACALL(0x0700|this->instruction[1]);break;
		
		case 0x24:this->ADD_A(this->instruction[1]);break;
		case 0x25:this->ADD_A(this->getDirectByteIn(this->instruction[1]));break;
		case 0x26:this->ADD_A(this->getRAMByte(this->getRAMByte(this->getR(0))));break;
		case 0x27:this->ADD_A(this->getRAMByte(this->getRAMByte(this->getR(1))));break;
		case 0x28:this->ADD_A(this->getRAMByte(this->getR(0)));break;
		case 0x29:this->ADD_A(this->getRAMByte(this->getR(1)));break;
		case 0x2A:this->ADD_A(this->getRAMByte(this->getR(2)));break;
		case 0x2B:this->ADD_A(this->getRAMByte(this->getR(3)));break;
		case 0x2C:this->ADD_A(this->getRAMByte(this->getR(4)));break;
		case 0x2D:this->ADD_A(this->getRAMByte(this->getR(5)));break;
		case 0x2E:this->ADD_A(this->getRAMByte(this->getR(6)));break;
		case 0x2F:this->ADD_A(this->getRAMByte(this->getR(7)));break;
		
		case 0x34:this->ADDC_A(this->instruction[1]);break;
		case 0x35:this->ADDC_A(this->getDirectByteIn(this->instruction[1]));break;
		case 0x36:this->ADDC_A(this->getRAMByte(this->getRAMByte(this->getR(0))));break;
		case 0x37:this->ADDC_A(this->getRAMByte(this->getRAMByte(this->getR(1))));break;
		case 0x38:this->ADDC_A(this->getRAMByte(this->getR(0)));break;
		case 0x39:this->ADDC_A(this->getRAMByte(this->getR(1)));break;
		case 0x3A:this->ADDC_A(this->getRAMByte(this->getR(2)));break;
		case 0x3B:this->ADDC_A(this->getRAMByte(this->getR(3)));break;
		case 0x3C:this->ADDC_A(this->getRAMByte(this->getR(4)));break;
		case 0x3D:this->ADDC_A(this->getRAMByte(this->getR(5)));break;
		case 0x3E:this->ADDC_A(this->getRAMByte(this->getR(6)));break;
		case 0x3F:this->ADDC_A(this->getRAMByte(this->getR(7)));break;
			
		case 0x01:this->AJMP(this->instruction[1]);break;
		case 0x21:this->AJMP(0x0100|this->instruction[1]);break;
		case 0x41:this->AJMP(0x0200|this->instruction[1]);break;
		case 0x61:this->AJMP(0x0300|this->instruction[1]);break;
		case 0x81:this->AJMP(0x0400|this->instruction[1]);break;
		case 0xA1:this->AJMP(0x0500|this->instruction[1]);break;
		case 0xC1:this->AJMP(0x0600|this->instruction[1]);break;
		case 0xE1:this->AJMP(0x0700|this->instruction[1]);break;
			
		case 0x52:this->setDirectByte(this->instruction[1],this->ANL(this->getDirectByteOut(this->instruction[1]),this->getSFRByteIn(this->ACC)));break;
		case 0x53:this->setDirectByte(this->instruction[1],this->ANL(this->getDirectByteOut(this->instruction[1]),this->instruction[2]));break;
		case 0x54:this->setSFRByte(this->ACC,this->ANL(this->getSFRByteIn(this->ACC),this->instruction[1]));break;
		case 0x55:this->setSFRByte(this->ACC,this->ANL(this->getSFRByteIn(this->ACC),this->getDirectByteIn(this->instruction[1])));break;
		case 0x56:this->setSFRByte(this->ACC,this->ANL(this->getSFRByteIn(this->ACC),this->getRAMByte(this->getRAMByte(this->getR(0)))));break;
		case 0x57:this->setSFRByte(this->ACC,this->ANL(this->getSFRByteIn(this->ACC),this->getRAMByte(this->getRAMByte(this->getR(1)))));break;
		case 0x58:this->setSFRByte(this->ACC,this->ANL(this->getSFRByteIn(this->ACC),this->getRAMByte(this->getR(0))));break;
		case 0x59:this->setSFRByte(this->ACC,this->ANL(this->getSFRByteIn(this->ACC),this->getRAMByte(this->getR(1))));break;
		case 0x5A:this->setSFRByte(this->ACC,this->ANL(this->getSFRByteIn(this->ACC),this->getRAMByte(this->getR(2))));break;
		case 0x5B:this->setSFRByte(this->ACC,this->ANL(this->getSFRByteIn(this->ACC),this->getRAMByte(this->getR(3))));break;
		case 0x5C:this->setSFRByte(this->ACC,this->ANL(this->getSFRByteIn(this->ACC),this->getRAMByte(this->getR(4))));break;
		case 0x5D:this->setSFRByte(this->ACC,this->ANL(this->getSFRByteIn(this->ACC),this->getRAMByte(this->getR(5))));break;
		case 0x5E:this->setSFRByte(this->ACC,this->ANL(this->getSFRByteIn(this->ACC),this->getRAMByte(this->getR(6))));break;
		case 0x5F:this->setSFRByte(this->ACC,this->ANL(this->getSFRByteIn(this->ACC),this->getRAMByte(this->getR(7))));break;
		
		case 0x82:this->ANL_C(this->getBitIn(this->instruction[1]));break;
		case 0xB0:this->ANL_C(!this->getBitIn(this->instruction[1]));break;
			
		case 0xB4:this->CJNE(this->getSFRByteIn(this->ACC),this->instruction[1],(signed char)this->instruction[2]);break;
		case 0xB5:this->CJNE(this->getSFRByteIn(this->ACC),this->getDirectByteIn(this->instruction[1]),(signed char)this->instruction[2]);break;
		case 0xB6:this->CJNE(this->getRAMByte(this->getRAMByte(this->getR(0))),this->instruction[1],(signed char)this->instruction[2]);break;
		case 0xB7:this->CJNE(this->getRAMByte(this->getRAMByte(this->getR(1))),this->instruction[1],(signed char)this->instruction[2]);break;
		case 0xB8:this->CJNE(this->getRAMByte(this->getR(0)),this->instruction[1],(signed char)this->instruction[2]);break;
		case 0xB9:this->CJNE(this->getRAMByte(this->getR(1)),this->instruction[1],(signed char)this->instruction[2]);break;
		case 0xBA:this->CJNE(this->getRAMByte(this->getR(2)),this->instruction[1],(signed char)this->instruction[2]);break;
		case 0xBB:this->CJNE(this->getRAMByte(this->getR(3)),this->instruction[1],(signed char)this->instruction[2]);break;
		case 0xBC:this->CJNE(this->getRAMByte(this->getR(4)),this->instruction[1],(signed char)this->instruction[2]);break;
		case 0xBD:this->CJNE(this->getRAMByte(this->getR(5)),this->instruction[1],(signed char)this->instruction[2]);break;
		case 0xBE:this->CJNE(this->getRAMByte(this->getR(6)),this->instruction[1],(signed char)this->instruction[2]);break;
		case 0xBF:this->CJNE(this->getRAMByte(this->getR(7)),this->instruction[1],(signed char)this->instruction[2]);break;
			
		case 0xE4:this->CLR_A();break;
		
		case 0xC2:
			a=this->getBitDirectAddress(this->instruction[1]);
			this->setDirectByte(a,this->CLR_bit(this->getDirectByteOut(a),this->getBitMask(this->instruction[1])));
			break;
		case 0xC3:
			a=this->getBitDirectAddress(this->CY);
			this->setSFRByte(a,this->CLR_bit(this->getSFRByteIn(a),this->getBitMask(this->CY)));
			break;
			
		case 0xF4:this->CPL_A();break;
		
		case 0xB2:
			a=this->getBitDirectAddress(this->instruction[1]);
			this->setDirectByte(a,this->CPL_bit(this->getDirectByteOut(a),this->getBitMask(this->instruction[1])));
			break;
		case 0xB3:
			a=this->getBitDirectAddress(this->CY);
			this->setSFRByte(a,this->CPL_bit(this->getSFRByteIn(a),this->getBitMask(this->CY)));
			break;
		
		case 0xD4:this->DA();break;
			
		case 0x14:this->setSFRByte(this->ACC,this->DEC(this->getSFRByteIn(this->ACC)));break;
		case 0x15:this->setDirectByte(this->instruction[1],this->DEC(this->getDirectByteOut(this->instruction[1])));break;
		case 0x16:
			a=this->getRAMByte(this->getR(0));
			this->setRAMByte(a,this->DEC(this->getRAMByte(a)));
			break;
		case 0x17:
			a=this->getRAMByte(this->getR(1));
			this->setRAMByte(a,this->DEC(this->getRAMByte(a)));
			break;
		case 0x18:this->setRAMByte(this->getR(0),this->DEC(this->getRAMByte(this->getR(0))));break;
		case 0x19:this->setRAMByte(this->getR(1),this->DEC(this->getRAMByte(this->getR(1))));break;
		case 0x1A:this->setRAMByte(this->getR(2),this->DEC(this->getRAMByte(this->getR(2))));break;
		case 0x1B:this->setRAMByte(this->getR(3),this->DEC(this->getRAMByte(this->getR(3))));break;
		case 0x1C:this->setRAMByte(this->getR(4),this->DEC(this->getRAMByte(this->getR(4))));break;
		case 0x1D:this->setRAMByte(this->getR(5),this->DEC(this->getRAMByte(this->getR(5))));break;
		case 0x1E:this->setRAMByte(this->getR(6),this->DEC(this->getRAMByte(this->getR(6))));break;
		case 0x1F:this->setRAMByte(this->getR(7),this->DEC(this->getRAMByte(this->getR(7))));break;
			
		case 0x84:this->DIV();break;
		
		case 0xD5:this->setDirectByte(this->instruction[1],this->DJNZ(this->getDirectByteOut(this->instruction[1]),(signed char)this->instruction[2]));break;
		case 0xD8:this->setRAMByte(this->getR(0),this->DJNZ(this->getRAMByte(this->getR(0)),(signed char)this->instruction[1]));break;
		case 0xD9:this->setRAMByte(this->getR(1),this->DJNZ(this->getRAMByte(this->getR(1)),(signed char)this->instruction[1]));break;
		case 0xDA:this->setRAMByte(this->getR(2),this->DJNZ(this->getRAMByte(this->getR(2)),(signed char)this->instruction[1]));break;
		case 0xDB:this->setRAMByte(this->getR(3),this->DJNZ(this->getRAMByte(this->getR(3)),(signed char)this->instruction[1]));break;
		case 0xDC:this->setRAMByte(this->getR(4),this->DJNZ(this->getRAMByte(this->getR(4)),(signed char)this->instruction[1]));break;
		case 0xDD:this->setRAMByte(this->getR(5),this->DJNZ(this->getRAMByte(this->getR(5)),(signed char)this->instruction[1]));break;
		case 0xDE:this->setRAMByte(this->getR(6),this->DJNZ(this->getRAMByte(this->getR(6)),(signed char)this->instruction[1]));break;
		case 0xDF:this->setRAMByte(this->getR(7),this->DJNZ(this->getRAMByte(this->getR(7)),(signed char)this->instruction[1]));break;
		
		case 0x04:this->setSFRByte(this->ACC,this->INC(this->getSFRByteIn(this->ACC)));break;
		case 0x05:this->setDirectByte(this->instruction[1],this->INC(this->getDirectByteOut(this->instruction[1])));break;
		case 0x06:
			a=this->getRAMByte(this->getR(0));
			this->setRAMByte(a,this->INC(this->getRAMByte(a)));
			break;
		case 0x07:
			a=this->getRAMByte(this->getR(1));
			this->setRAMByte(a,this->INC(this->getRAMByte(a)));
			break;
		case 0x08:this->setRAMByte(this->getR(0),this->INC(this->getRAMByte(this->getR(0))));break;
		case 0x09:this->setRAMByte(this->getR(1),this->INC(this->getRAMByte(this->getR(1))));break;
		case 0x0A:this->setRAMByte(this->getR(2),this->INC(this->getRAMByte(this->getR(2))));break;
		case 0x0B:this->setRAMByte(this->getR(3),this->INC(this->getRAMByte(this->getR(3))));break;
		case 0x0C:this->setRAMByte(this->getR(4),this->INC(this->getRAMByte(this->getR(4))));break;
		case 0x0D:this->setRAMByte(this->getR(5),this->INC(this->getRAMByte(this->getR(5))));break;
		case 0x0E:this->setRAMByte(this->getR(6),this->INC(this->getRAMByte(this->getR(6))));break;
		case 0x0F:this->setRAMByte(this->getR(7),this->INC(this->getRAMByte(this->getR(7))));break;
		
		case 0xA3:this->INC_DPTR();break;
			
		case 0x20:this->JB(this->getDirectByteIn(this->getBitDirectAddress(this->instruction[1])),this->getBitMask(this->instruction[1]),(signed char)this->instruction[2]);break;
			
		case 0x10://jbc
			a=this->getBitDirectAddress(this->instruction[1]);
			d=this->JBC(this->getDirectByteOut(a),this->getBitMask(this->instruction[1]),(signed char)this->instruction[2]);break;
			if (d!=0xFF) this->setDirectByte(a,d);
			break;
			
		case 0x40:this->JC((signed char)this->instruction[1]);break;
			
		case 0x73:this->JMP();break;
		
		case 0x30:this->JNB(this->getDirectByteIn(this->getBitDirectAddress(this->instruction[1])),this->getBitMask(this->instruction[1]),(signed char)this->instruction[2]);break;
			
		case 0x50:this->JNC((signed char)this->instruction[1]);break;
			
		case 0x70:this->JNZ((signed char)this->instruction[1]);break;
			
		case 0x60:this->JZ((signed char)this->instruction[1]);break;
			
		case 0x12:this->LCALL((((unsigned short)this->instruction[1])<<8)|((unsigned short)this->instruction[2]));break;
			
		case 0x02:this->LJMP((((unsigned short)this->instruction[1])<<8)|((unsigned short)this->instruction[2]));break;
			
		case 0x74:this->setSFRByte(this->ACC,this->instruction[1]);break;
		case 0x75:this->setDirectByte(this->instruction[1],this->instruction[2]);break;
		case 0x76:this->setRAMByte(this->getRAMByte(this->getR(0)),this->instruction[1]);break;
		case 0x77:this->setRAMByte(this->getRAMByte(this->getR(1)),this->instruction[1]);break;
		case 0x78:this->setRAMByte(this->getR(0),this->instruction[1]);break;
		case 0x79:this->setRAMByte(this->getR(1),this->instruction[1]);break;
		case 0x7A:this->setRAMByte(this->getR(2),this->instruction[1]);break;
		case 0x7B:this->setRAMByte(this->getR(3),this->instruction[1]);break;
		case 0x7C:this->setRAMByte(this->getR(4),this->instruction[1]);break;
		case 0x7D:this->setRAMByte(this->getR(5),this->instruction[1]);break;
		case 0x7E:this->setRAMByte(this->getR(6),this->instruction[1]);break;
		case 0x7F:this->setRAMByte(this->getR(7),this->instruction[1]);break;
		case 0x85:this->setDirectByte(this->instruction[2],this->getDirectByteIn(this->instruction[1]));break;//!!!!!!!!!!!
		case 0x86:this->setDirectByte(this->instruction[1],this->getRAMByte(this->getRAMByte(this->getR(0))));break;
		case 0x87:this->setDirectByte(this->instruction[1],this->getRAMByte(this->getRAMByte(this->getR(1))));break;
		case 0x88:this->setDirectByte(this->instruction[1],this->getRAMByte(this->getR(0)));break;
		case 0x89:this->setDirectByte(this->instruction[1],this->getRAMByte(this->getR(1)));break;
		case 0x8A:this->setDirectByte(this->instruction[1],this->getRAMByte(this->getR(2)));break;
		case 0x8B:this->setDirectByte(this->instruction[1],this->getRAMByte(this->getR(3)));break;
		case 0x8C:this->setDirectByte(this->instruction[1],this->getRAMByte(this->getR(4)));break;
		case 0x8D:this->setDirectByte(this->instruction[1],this->getRAMByte(this->getR(5)));break;
		case 0x8E:this->setDirectByte(this->instruction[1],this->getRAMByte(this->getR(6)));break;
		case 0x8F:this->setDirectByte(this->instruction[1],this->getRAMByte(this->getR(7)));break;
		case 0xA6:this->setRAMByte(this->getRAMByte(this->getR(0)),this->getDirectByteIn(this->instruction[1]));break;
		case 0xA7:this->setRAMByte(this->getRAMByte(this->getR(1)),this->getDirectByteIn(this->instruction[1]));break;
		case 0xA8:this->setRAMByte(this->getR(0),this->getDirectByteIn(this->instruction[1]));break;
		case 0xA9:this->setRAMByte(this->getR(1),this->getDirectByteIn(this->instruction[1]));break;
		case 0xAA:this->setRAMByte(this->getR(2),this->getDirectByteIn(this->instruction[1]));break;
		case 0xAB:this->setRAMByte(this->getR(3),this->getDirectByteIn(this->instruction[1]));break;
		case 0xAC:this->setRAMByte(this->getR(4),this->getDirectByteIn(this->instruction[1]));break;
		case 0xAD:this->setRAMByte(this->getR(5),this->getDirectByteIn(this->instruction[1]));break;
		case 0xAE:this->setRAMByte(this->getR(6),this->getDirectByteIn(this->instruction[1]));break;
		case 0xAF:this->setRAMByte(this->getR(7),this->getDirectByteIn(this->instruction[1]));break;
		case 0xE5:this->setSFRByte(this->ACC,this->getDirectByteIn(this->instruction[1]));break;
		case 0xE6:this->setSFRByte(this->ACC,this->getRAMByte(this->getRAMByte(this->getR(0))));break;
		case 0xE7:this->setSFRByte(this->ACC,this->getRAMByte(this->getRAMByte(this->getR(1))));break;
		case 0xE8:this->setSFRByte(this->ACC,this->getRAMByte(this->getR(0)));break;
		case 0xE9:this->setSFRByte(this->ACC,this->getRAMByte(this->getR(1)));break;
		case 0xEA:this->setSFRByte(this->ACC,this->getRAMByte(this->getR(2)));break;
		case 0xEB:this->setSFRByte(this->ACC,this->getRAMByte(this->getR(3)));break;
		case 0xEC:this->setSFRByte(this->ACC,this->getRAMByte(this->getR(4)));break;
		case 0xED:this->setSFRByte(this->ACC,this->getRAMByte(this->getR(5)));break;
		case 0xEE:this->setSFRByte(this->ACC,this->getRAMByte(this->getR(6)));break;
		case 0xEF:this->setSFRByte(this->ACC,this->getRAMByte(this->getR(7)));break;
		case 0xF5:this->setDirectByte(this->instruction[1],this->getSFRByteIn(this->ACC));break;
		case 0xF6:this->setRAMByte(this->getRAMByte(this->getR(0)),this->getSFRByteIn(this->ACC));break;
		case 0xF7:this->setRAMByte(this->getRAMByte(this->getR(1)),this->getSFRByteIn(this->ACC));break;
		case 0xF8:this->setRAMByte(this->getR(0),this->getSFRByteIn(this->ACC));break;
		case 0xF9:this->setRAMByte(this->getR(1),this->getSFRByteIn(this->ACC));break;
		case 0xFA:this->setRAMByte(this->getR(2),this->getSFRByteIn(this->ACC));break;
		case 0xFB:this->setRAMByte(this->getR(3),this->getSFRByteIn(this->ACC));break;
		case 0xFC:this->setRAMByte(this->getR(4),this->getSFRByteIn(this->ACC));break;
		case 0xFD:this->setRAMByte(this->getR(5),this->getSFRByteIn(this->ACC));break;
		case 0xFE:this->setRAMByte(this->getR(6),this->getSFRByteIn(this->ACC));break;
		case 0xFF:this->setRAMByte(this->getR(7),this->getSFRByteIn(this->ACC));break;
		
		case 0x92:this->setBitOut(this->instruction[1],this->getBitIn(this->CY));break;
		case 0xA2:this->setBitIn(this->CY,this->getBitIn(this->instruction[1]));break;
		
		case 0x90:
			this->setSFRByte(this->DPH,this->instruction[1]);
			this->setSFRByte(this->DPL,this->instruction[2]);
			break;
			
		case 0x83:this->MOVC(this->PC);break;
		case 0x93:this->MOVC((((unsigned short)this->getSFRByteIn(this->DPH))<<8)|((unsigned short)this->getSFRByteIn(this->DPL)));break;
			
		case 0xE0:this->MOVX_I((unsigned short)((((unsigned short)this->getSFRByteIn(this->DPH))<<8)|((unsigned short)this->getSFRByteIn(this->DPL))));break;
		case 0xE2:this->MOVX_I(this->getRAMByte(this->getR(0)));break;
		case 0xE3:this->MOVX_I(this->getRAMByte(this->getR(1)));break;
		case 0xF0:this->MOVX_O((unsigned short)((((unsigned short)this->getSFRByteIn(this->DPH))<<8)|((unsigned short)this->getSFRByteIn(this->DPL))));break;
		case 0xF2:this->MOVX_O(this->getRAMByte(this->getR(0)));break;
		case 0xF3:this->MOVX_O(this->getRAMByte(this->getR(1)));break;
			
		case 0xA4:this->MUL();break;
		
		case 0:break;//NOP
			
		case 0x42:this->setDirectByte(this->instruction[1],this->ORL(this->getDirectByteOut(this->instruction[1]),this->getSFRByteIn(this->ACC)));break;
		case 0x43:this->setDirectByte(this->instruction[1],this->ORL(this->getDirectByteOut(this->instruction[1]),this->instruction[2]));break;
		case 0x44:this->setSFRByte(this->ACC,this->ORL(this->getSFRByteIn(this->ACC),this->instruction[1]));break;
		case 0x45:this->setSFRByte(this->ACC,this->ORL(this->getSFRByteIn(this->ACC),this->getDirectByteIn(this->instruction[1])));break;
		case 0x46:this->setSFRByte(this->ACC,this->ORL(this->getSFRByteIn(this->ACC),this->getRAMByte(this->getRAMByte(this->getR(0)))));break;
		case 0x47:this->setSFRByte(this->ACC,this->ORL(this->getSFRByteIn(this->ACC),this->getRAMByte(this->getRAMByte(this->getR(1)))));break;
		case 0x48:this->setSFRByte(this->ACC,this->ORL(this->getSFRByteIn(this->ACC),this->getRAMByte(this->getR(0))));break;
		case 0x49:this->setSFRByte(this->ACC,this->ORL(this->getSFRByteIn(this->ACC),this->getRAMByte(this->getR(1))));break;
		case 0x4A:this->setSFRByte(this->ACC,this->ORL(this->getSFRByteIn(this->ACC),this->getRAMByte(this->getR(2))));break;
		case 0x4B:this->setSFRByte(this->ACC,this->ORL(this->getSFRByteIn(this->ACC),this->getRAMByte(this->getR(3))));break;
		case 0x4C:this->setSFRByte(this->ACC,this->ORL(this->getSFRByteIn(this->ACC),this->getRAMByte(this->getR(4))));break;
		case 0x4D:this->setSFRByte(this->ACC,this->ORL(this->getSFRByteIn(this->ACC),this->getRAMByte(this->getR(5))));break;
		case 0x4E:this->setSFRByte(this->ACC,this->ORL(this->getSFRByteIn(this->ACC),this->getRAMByte(this->getR(6))));break;
		case 0x4F:this->setSFRByte(this->ACC,this->ORL(this->getSFRByteIn(this->ACC),this->getRAMByte(this->getR(7))));break;
			
		case 0x72:this->ORL_C(this->getBitIn(this->instruction[1]));break;
		case 0xA0:this->ORL_C(!this->getBitIn(this->instruction[1]));break;
			
		case 0xD0:this->setDirectByte(this->instruction[1],this->POP());break;
			
		case 0xC0:this->PUSH(this->getDirectByteIn(this->instruction[1]));break;
		
		case 0x22:this->RET();break;
			
		case 0x32:this->RETI();break;
			
		case 0x23:this->RL();break;
			
		case 0x33:this->RLC();break;
			
		case 0x03:this->RR();break;
			
		case 0x13:this->RRC();break;
			
		case 0xD2:this->setBitOut(this->instruction[1],true);break;
		case 0xD3:this->setBitIn(this->CY,true);break;
			
		case 0x80:this->SJMP((signed char)this->instruction[1]);break;
			
		case 0x94:this->SUBB(this->instruction[1]);break;
		case 0x95:this->SUBB(this->getDirectByteIn(this->instruction[1]));break;
		case 0x96:this->SUBB(this->getRAMByte(this->getRAMByte(this->getR(0))));break;
		case 0x97:this->SUBB(this->getRAMByte(this->getRAMByte(this->getR(1))));break;
		case 0x98:this->SUBB(this->getRAMByte(this->getR(0)));break;
		case 0x99:this->SUBB(this->getRAMByte(this->getR(1)));break;
		case 0x9A:this->SUBB(this->getRAMByte(this->getR(2)));break;
		case 0x9B:this->SUBB(this->getRAMByte(this->getR(3)));break;
		case 0x9C:this->SUBB(this->getRAMByte(this->getR(4)));break;
		case 0x9D:this->SUBB(this->getRAMByte(this->getR(5)));break;
		case 0x9E:this->SUBB(this->getRAMByte(this->getR(6)));break;
		case 0x9F:this->SUBB(this->getRAMByte(this->getR(7)));break;
			
		case 0xC4:this->SWAP();break;
			
		case 0xC5:this->setDirectByte(this->instruction[1],this->XCH(this->getDirectByteIn(this->instruction[1])));break;
		case 0xC6:
			a=this->getRAMByte(this->getR(0));
			this->setRAMByte(a,this->XCH(this->getRAMByte(a)));
			break;
		case 0xC7:
			a=this->getRAMByte(this->getR(1));
			this->setRAMByte(a,this->XCH(this->getRAMByte(a)));
			break;
		case 0xC8:this->setRAMByte(this->getR(0),this->XCH(this->getRAMByte(this->getR(0))));break;
		case 0xC9:this->setRAMByte(this->getR(1),this->XCH(this->getRAMByte(this->getR(1))));break;
		case 0xCA:this->setRAMByte(this->getR(2),this->XCH(this->getRAMByte(this->getR(2))));break;
		case 0xCB:this->setRAMByte(this->getR(3),this->XCH(this->getRAMByte(this->getR(3))));break;
		case 0xCC:this->setRAMByte(this->getR(4),this->XCH(this->getRAMByte(this->getR(4))));break;
		case 0xCD:this->setRAMByte(this->getR(5),this->XCH(this->getRAMByte(this->getR(5))));break;
		case 0xCE:this->setRAMByte(this->getR(6),this->XCH(this->getRAMByte(this->getR(6))));break;
		case 0xCF:this->setRAMByte(this->getR(7),this->XCH(this->getRAMByte(this->getR(7))));break;
			
		case 0xD6:
			a=this->getRAMByte(this->getR(0));
			this->setRAMByte(a,this->XCHD(this->getRAMByte(a)));
			break;
		case 0xD7:
			a=this->getRAMByte(this->getR(1));
			this->setRAMByte(a,this->XCHD(this->getRAMByte(a)));
			break;
			
		case 0x62:this->setDirectByte(this->instruction[1],this->XRL(this->getDirectByteOut(this->instruction[1]),this->getSFRByteIn(this->ACC)));break;
		case 0x63:this->setDirectByte(this->instruction[1],this->XRL(this->getDirectByteOut(this->instruction[1]),this->instruction[2]));break;
		case 0x64:this->setSFRByte(this->ACC,this->XRL(this->getSFRByteIn(this->ACC),this->instruction[1]));break;
		case 0x65:this->setSFRByte(this->ACC,this->XRL(this->getSFRByteIn(this->ACC),this->getDirectByteIn(this->instruction[1])));break;
		case 0x66:this->setSFRByte(this->ACC,this->XRL(this->getSFRByteIn(this->ACC),this->getRAMByte(this->getRAMByte(this->getR(0)))));break;
		case 0x67:this->setSFRByte(this->ACC,this->XRL(this->getSFRByteIn(this->ACC),this->getRAMByte(this->getRAMByte(this->getR(1)))));break;
		case 0x68:this->setSFRByte(this->ACC,this->XRL(this->getSFRByteIn(this->ACC),this->getRAMByte(this->getR(0))));break;
		case 0x69:this->setSFRByte(this->ACC,this->XRL(this->getSFRByteIn(this->ACC),this->getRAMByte(this->getR(1))));break;
		case 0x6A:this->setSFRByte(this->ACC,this->XRL(this->getSFRByteIn(this->ACC),this->getRAMByte(this->getR(2))));break;
		case 0x6B:this->setSFRByte(this->ACC,this->XRL(this->getSFRByteIn(this->ACC),this->getRAMByte(this->getR(3))));break;
		case 0x6C:this->setSFRByte(this->ACC,this->XRL(this->getSFRByteIn(this->ACC),this->getRAMByte(this->getR(4))));break;
		case 0x6D:this->setSFRByte(this->ACC,this->XRL(this->getSFRByteIn(this->ACC),this->getRAMByte(this->getR(5))));break;
		case 0x6E:this->setSFRByte(this->ACC,this->XRL(this->getSFRByteIn(this->ACC),this->getRAMByte(this->getR(6))));break;
		case 0x6F:this->setSFRByte(this->ACC,this->XRL(this->getSFRByteIn(this->ACC),this->getRAMByte(this->getR(7))));break;
	}
}
void m80C32::ACALL(unsigned short addr11){
	//printf("ACALL\n");
	unsigned char sp=this->getSFRByteIn(this->SP)+1;
	this->setRAMByte(sp,(unsigned char)(this->PC&0xFF));
	sp++;
	this->setRAMByte(sp,(unsigned char)(this->PC>>8));
	this->setSFRByte(this->SP,sp);
	this->PC&=0xF800;
	this->PC|=addr11;
}
void m80C32::ADD_A(unsigned char d){
	//printf("ADD\n");
	unsigned short a=(unsigned short)this->getSFRByteIn(this->ACC);
	unsigned short v=(unsigned short)d;
	unsigned short r=(a&0x0F)+(v&0x0F);
	this->setBitIn(this->AC,r>0x0F);
	r+=(a&0x70)+(v&0x70);
	bool o=r>0x7F;
	r+=(a&0x80)+(v&0x80);
	bool c=r>0xFF;
	this->setBitIn(this->CY,c);
	this->setBitIn(this->OV,c!=o);
	this->setSFRByte(this->ACC,(unsigned char)r);
}
void m80C32::ADDC_A(unsigned char d){
	//printf("ADDC\n");
	unsigned short a=(unsigned short)this->getSFRByteIn(this->ACC);
	unsigned short v=(unsigned short)d;
	unsigned short r=(a&0x0F)+(v&0x0F);
	if (this->getBitIn(this->CY)) r++;
	this->setBitIn(this->AC,r>0x0F);
	r+=(a&0x70)+(v&0x70);
	bool o=r>0x7F;
	r+=(a&0x80)+(v&0x80);
	bool c=r>0xFF;
	this->setBitIn(this->CY,c);
	this->setBitIn(this->OV,c!=o);
	this->setSFRByte(this->ACC,(unsigned char)r);
}
void m80C32::AJMP(unsigned short addr11){
	//printf("AJMP\n");
	this->PC&=0xF800;
	this->PC|=addr11;
}
unsigned char m80C32::ANL(unsigned char d1,unsigned char d2){
	//printf("ANL\n");
	return d1&d2;
}
void m80C32::ANL_C(bool b){
	//printf("ANL C\n");
	if (!b) this->setBitIn(this->CY,false);
}
void m80C32::CJNE(unsigned char d1,unsigned char d2,signed char rel){
	//printf("CJNE\n");
	this->setBitIn(this->CY,d1<d2);
	if (d1!=d2){
		this->PC+=rel;
	}
}
void m80C32::CLR_A(){
	//printf("CLRa\n");
	this->setSFRByte(this->ACC,0);
}
unsigned char m80C32::CLR_bit(unsigned char d,unsigned char m){
	//printf("CLRb\n");
	return d&(~m);
}
void m80C32::CPL_A(){
	//printf("CPLa\n");
	this->setSFRByte(this->ACC,~(this->getSFRByteIn(this->ACC)));
}
unsigned char m80C32::CPL_bit(unsigned char d,unsigned char m){
	//printf("CPLb\n");
	return d^m;
}
void m80C32::DA(){
	//printf("DA\n");
	unsigned short a=(unsigned short)this->getSFRByteIn(this->ACC);
	if (this->getBitIn(this->AC)||(a&0x0F)>9){
		a+=0x06;
	}
	if (this->getBitIn(this->CY)||(a>0x99)){
		this->setBitIn(this->CY,true);
		a+=0x60;
	}
	this->setSFRByte(this->ACC,(unsigned char)a);
}
unsigned char m80C32::DEC(unsigned char d){
	//printf("DEC\n");
	return d-1;
}
void m80C32::DIV(){
	//printf("DIV\n");
	unsigned char a=this->getSFRByteIn(this->ACC);
	unsigned char b=this->getSFRByteIn(this->B);
	this->setBitIn(this->CY,false);
	if (b==0x00){
		this->setBitIn(this->OV,true);
	}
	else{
		this->setBitIn(this->OV,false);
		this->setSFRByte(this->ACC,a/b);
		this->setSFRByte(this->B,a%b);
	}
}
unsigned char m80C32::DJNZ(unsigned char d,signed char rel){
	//printf("DJNZ\n");
	d--;
	if (d!=0){
		this->PC+=rel;
	}
	return d;
}
unsigned char m80C32::INC(unsigned char d){
	//printf("INC\n");
	return d+1;
}
void m80C32::INC_DPTR(){
	//printf("INCdptr\n");
	unsigned short dptr=((unsigned short)this->getSFRByteIn(this->DPL))|(((unsigned short)this->getSFRByteIn(this->DPH))<<8);
	dptr++;
	this->setSFRByte(this->DPL,(unsigned char)(dptr&0xFF));
	this->setSFRByte(this->DPH,(unsigned char)(dptr>>8));
}
void m80C32::JB(unsigned char d,unsigned char m,signed char rel){
	//printf("JB\n");
	if ((bool)(d&m)) this->PC+=rel;
}
unsigned char m80C32::JBC(unsigned char d,unsigned char m,signed char rel){
	//printf("JBC\n");
	if ((bool)(d&m)){
		this->PC+=rel;
		return d&(~m);
	}
	else{
		return 0xFF;
	}
}
void m80C32::JC(signed char rel){
	//printf("JC\n");
	if (getBitIn(this->CY))this->PC+=rel;
}
void m80C32::JMP(){
	//printf("JMP\n");
	unsigned short dptr=((unsigned short)this->getSFRByteIn(this->DPL))|(((unsigned short)this->getSFRByteIn(this->DPH))<<8);
	dptr+=(unsigned short)this->getSFRByteIn(this->ACC);
	this->PC=dptr;
}
void m80C32::JNB(unsigned char d,unsigned char m,signed char rel){
	//printf("JNB\n");
	if (!(bool)(d&m)) this->PC+=rel;
}
void m80C32::JNC(signed char rel){
	//printf("JNC\n");
	if (!getBitIn(this->CY))this->PC+=rel;
}
void m80C32::JNZ(signed char rel){
	//printf("JNZ\n");
	if (getSFRByteIn(this->ACC)!=0)this->PC+=rel;
}
void m80C32::JZ(signed char rel){
	//printf("JZ\n");
	if (getSFRByteIn(this->ACC)==0)this->PC+=rel;
}
void m80C32::LCALL(unsigned short addr16){
	//printf("LCALL\n");
	unsigned char sp=this->getSFRByteIn(this->SP)+1;
	this->setRAMByte(sp,(unsigned char)(this->PC&0xFF));
	sp++;
	this->setRAMByte(sp,(unsigned char)(this->PC>>8));
	this->setSFRByte(this->SP,sp);
	this->PC=addr16;
}
void m80C32::LJMP(unsigned short addr16){
	//printf("LJMP\n");
	this->PC=addr16;
}
//MOV
//MOV bit
//MOV DPTR
void m80C32::MOVC(unsigned short ptr){
	//printf("MOVC\n");
	ptr+=this->getSFRByteIn(this->ACC);
	this->sendP0((unsigned char)(ptr&0xFF));
	this->sendP2((unsigned char)(ptr>>8));
	this->sendALE(true);
	this->sendALE(false);
	this->sendnPSEN(false);
	this->setSFRByte(this->ACC,this->getSFRByteIn(this->P0));
	this->sendnPSEN(true);
	this->sendP0(this->PX_out[0]);
	this->sendP2(this->PX_out[2]);
}
void m80C32::MOVX_I(unsigned char a){//8 bit address
	//printf("MOVXin 8bit\n");
	this->sendP0(a);
	this->sendALE(true);
	this->sendALE(false);
	this->sendP3((this->PX_out[3]&0x3F)|0x40);
	this->setSFRByte(this->ACC,this->getSFRByteIn(this->P0));
	this->sendP3(this->PX_out[3]);
	this->sendP0(this->PX_out[0]);
	//printf("in ic 8bits %02X %02X\n",a,this->getSFRByteIn(this->ACC));
}
void m80C32::MOVX_I(unsigned short a){//16 bit address
	//printf("MOVXin 16bit\n");
	this->sendP0((unsigned char)(a&0xFF));
	this->sendP2((unsigned char)(a>>8));
	this->sendALE(true);
	this->sendALE(false);
	this->sendP3((this->PX_out[3]&0x3F)|0x40);
	this->setSFRByte(this->ACC,this->getSFRByteIn(this->P0));
	this->sendP3(this->PX_out[3]);
	this->sendP0(this->PX_out[0]);
	this->sendP2(this->PX_out[2]);
	//printf("in ic 16bits %02X %02X\n",a,this->getSFRByteIn(this->ACC));
}
void m80C32::MOVX_O(unsigned char a){//8 bit address
	//printf("MOVXout 8bit\n");
	//printf("out ic 8bits %02X %02X\n",a,this->getSFRByteIn(this->ACC));
	this->sendP0(a);
	this->sendALE(true);
	this->sendALE(false);
	this->sendP0(this->getSFRByteIn(this->ACC));
	this->sendP3((this->PX_out[3]&0x3F)|0x80);
	this->sendP3(this->PX_out[3]);
	this->sendP0(this->PX_out[0]);
}
void m80C32::MOVX_O(unsigned short a){//16 bit address
	//printf("MOVXout 16bit\n");
	//printf("out ic 16bits %04X %02X\n",a,this->getSFRByteIn(this->ACC));
	this->sendP0((unsigned char)(a&0xFF));
	this->sendP2((unsigned char)(a>>8));
	this->sendALE(true);
	this->sendALE(false);
	this->sendP0(this->getSFRByteIn(this->ACC));
	this->sendP3((this->PX_out[3]&0x3F)|0x80);
	this->sendP3(this->PX_out[3]);
	this->sendP0(this->PX_out[0]);
	this->sendP2(this->PX_out[2]);
}
void m80C32::MUL(){
	//printf("MUL\n");
	unsigned short ab=((unsigned short)this->getSFRByteIn(this->ACC))*((unsigned short)this->getSFRByteIn(this->B));
	this->setSFRByte(this->ACC,(unsigned char)(ab&0xFF));
	this->setSFRByte(this->B,(unsigned char)(ab>>8));
	this->setBitIn(this->CY,false);
	this->setBitIn(this->OV,ab>0xFF);
}
unsigned char m80C32::ORL(unsigned char d1,unsigned char d2){
	//printf("ORL\n");
	return d1|d2;
}
void m80C32::ORL_C(bool b){
	//printf("ORLcy\n");
	if (b) this->setBitIn(this->CY,true);
}
unsigned char m80C32::POP(){
	//printf("POP\n");
	unsigned char sp=this->getSFRByteIn(this->SP);
	unsigned char d=this->getRAMByte(sp);
	sp--;
	this->setSFRByte(this->SP,sp);
	return d;
}
void m80C32::PUSH(unsigned char d){
	//printf("PUSH\n");
	unsigned char sp=this->getSFRByteIn(this->SP);
	sp++;
	this->setRAMByte(sp,d);
	this->setSFRByte(this->SP,sp);
}
void m80C32::RET(){
	//printf("RET\n");
	unsigned char sp=this->getSFRByteIn(this->SP);
	unsigned short ptr=(unsigned short)this->getRAMByte(sp);
	sp--;
	ptr=(ptr<<8)|(unsigned short)this->getRAMByte(sp);
	sp--;
	this->setSFRByte(this->SP,sp);
	this->PC=ptr;
}
void m80C32::RETI(){
	//printf("RETI\n");
	//////////////////////////////////////////////////////////////////////////////////////
	this->decreaseInterruptLevel();
	unsigned char sp=this->getSFRByteIn(this->SP);
	unsigned short ptr=(unsigned short)this->getRAMByte(sp);
	sp--;
	ptr=(ptr<<8)|(unsigned short)this->getRAMByte(sp);
	sp--;
	this->setSFRByte(this->SP,sp);
	this->PC=ptr;
}
void m80C32::RL(){
	//printf("RL\n");
	unsigned char a=this->getSFRByteIn(this->ACC);
	a=(a<<1)|(a>>7);
	this->setSFRByte(this->ACC,a);
}
void m80C32::RLC(){
	//printf("RLC\n");
	unsigned char a=this->getSFRByteIn(this->ACC);
	bool b=this->getBitIn(this->CY);
	this->setBitIn(this->CY,(bool)(a>>7));
	a=(a<<1)|(b?0x01:0x00);
	this->setSFRByte(this->ACC,a);
}
void m80C32::RR(){
	//printf("RR\n");
	unsigned char a=this->getSFRByteIn(this->ACC);
	a=(a>>1)|(a<<7);
	this->setSFRByte(this->ACC,a);
}
void m80C32::RRC(){
	//printf("RRC\n");
	unsigned char a=this->getSFRByteIn(this->ACC);
	bool b=this->getBitIn(this->CY);
	this->setBitIn(this->CY,(bool)(a&0x01));
	a=(a>>1)|(b?0x80:0x00);
	this->setSFRByte(this->ACC,a);
}
//SETB
void m80C32::SJMP(signed char rel){
	//printf("SJMP\n");
	this->PC+=rel;
}
void m80C32::SUBB(unsigned char d){////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//printf("SUBB\n");
	unsigned short a=(unsigned short)this->getBitIn(this->ACC);
	unsigned short v=(unsigned short)d;
	bool c=this->getBitIn(this->CY);
	unsigned short r=(a&0x0F)-(v&0x0F)-(c?1:0);
	this->setBitIn(this->AC,r>0x0F);
	r+=(a&0x70)-(v&0x70);
	bool o=r>0x7F;
	r+=(a&0x80)-(v&0x80);
	c=r>0xFF;
	this->setBitIn(this->CY,c);
	this->setBitIn(this->OV,c!=o);
	this->setSFRByte(this->ACC,(unsigned char)r);
}
void m80C32::SWAP(){
	//printf("SWAP\n");
	unsigned char a=this->getSFRByteIn(this->ACC);
	a=(a>>4)|(a<<4);
	this->setSFRByte(this->ACC,a);
}
unsigned char m80C32::XCH(unsigned char d){
	//printf("XCH\n");
	unsigned char d2=this->getSFRByteIn(this->ACC);
	this->setSFRByte(this->ACC,d);
	return d2;
}
unsigned char m80C32::XCHD(unsigned char d){
	//printf("XCHD\n");
	unsigned char d2=this->getSFRByteIn(this->ACC);
	this->setSFRByte(this->ACC,(d2&0xF0)|(d&0x0F));//getCharIn???
	return (d&0xF0)|(d2&0x0F);
}
unsigned char m80C32::XRL(unsigned char d1,unsigned char d2){
	//printf("XRL\n");
	return d1^d2;
}