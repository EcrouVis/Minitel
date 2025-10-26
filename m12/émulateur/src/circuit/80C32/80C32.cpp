#include <limits.h>
#define NIBBLE_MAX 15
#include "circuit/80c32.h"
#include <cstdio>

m80C32::m80C32(){
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
bool m80C32::getBitIn(unsigned char address){
	unsigned char bit;
	this->bitaddress2address(&address,&bit);
	if ((bool)(address&0x80)){
		return (bool)((this->getSFRByteIn(address)>>bit)&0x01);
	}
	else{
		return (bool)((this->getRAMByte(address)>>bit)&0x01);
	}
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
unsigned char m80C32::getRAMByte(unsigned char address){
	this->last_memory_operation.store(address,std::memory_order_relaxed);
	return this->iRAM[address].load(std::memory_order_relaxed);
}
unsigned char m80C32::getSFRByteIn(unsigned char address){
	return this->SFR[address&0x7F];
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
		default:
			break;
	}
	return this->SFR[address&0x7F];
}
//change state + callback for PX port change
void m80C32::setRAMByte(unsigned char address, unsigned char d){
	this->last_memory_operation.store(((unsigned int)address)+IRAM_SIZE,std::memory_order_relaxed);
	this->iRAM[address].store(d,std::memory_order_relaxed);
}
void m80C32::setSFRByte(unsigned char address, unsigned char d){
	switch (address){
		case this->ACC:
			this->SFR[address&0x7F]=d;
			this->setACCParity();
			break;
		case this->IE:
		case this->IP:
			this->SFR[address&0x7F]=d;
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
			this->SFR[address&0x7F]=d;
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
			this->SFR[address&0x7F]=d;
			break;
		default:
			break;
	}
}

unsigned char m80C32::getR(unsigned char r){
	return (this->getSFRByteIn(this->PSW)&0x18)|r;
}

/*void m80C32::PXChange(unsigned char address, unsigned char d){
	unsigned char X=(address>>4)&0x03;
	this->PX_out[X]=d;
	switch (X){
		case 0:
			//(*this->sendP0)(d);
			this->sendP0(d);
			break;
		case 1:
			//(*this->sendP1)(d);
			this->sendP1(d);
			break;
		case 2:
			//(*this->sendP2)(d);
			this->sendP2(d);
			break;
		case 3:
			//(*this->sendP3)(d);
			this->sendP3(d);
			break;
	}
}*/

/*
=============== IO ===============
*/
void m80C32::CLKTickIn(){
	this->fixedSerialClockTick();
	//printf("0 %02X / 1 %02X / 2 %02X / 3 %02X\n",PX_out[0],PX_out[1],PX_out[2],PX_out[3]);
	
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
	this->SFR[x&0x7F]=d;//don't trigger infinite loop + don't write to TX_out
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
	
}
void m80C32::PowerDown(){
	/*(*this->sendALE)(false);
	(*this->sendnPSEN)(false);*/
	this->sendALE(true);
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
		
		interrupt_signal&=this->getSFRByteIn(this->IE)&0x38;
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
	unsigned char d1;
	unsigned char d2;
	unsigned char d;
	unsigned char a;
	unsigned short ptrs;
	unsigned char ptrc;
	switch (this->instruction[0]){
		default:
		case 0:
		case 0xA5://nop / reserved
			break;
		
		case 0x11:
		case 0x31:
		case 0x51:
		case 0x71:
		case 0x91:
		case 0xB1:
		case 0xD1:
		case 0xF1://acall
			this->ACALL();
			break;
			
		case 0x01:
		case 0x21:
		case 0x41:
		case 0x61:
		case 0x81:
		case 0xA1:
		case 0xC1:
		case 0xE1://ajmp
			this->AJMP();
			break;
			
		case 0xB4:
			d1=this->getSFRByteIn(this->ACC);
			d2=this->instruction[1];
			goto i_cjne;
		case 0xB5:
			d1=this->getSFRByteIn(this->ACC);
			d2=this->instruction[1];
			if ((bool)(d2&0x80)) d2=this->getSFRByteIn(d2);
			else d2=this->getRAMByte(d2);
			goto i_cjne;
		case 0xB6:
			d1=this->getRAMByte(this->getRAMByte(this->getR(0)));
			d2=this->instruction[1];
			goto i_cjne;
		case 0xB7:
			d1=this->getRAMByte(this->getRAMByte(this->getR(1)));
			d2=this->instruction[1];
			goto i_cjne;
		case 0xB8:
		case 0xB9:
		case 0xBA:
		case 0xBB:
		case 0xBC:
		case 0xBD:
		case 0xBE:
		case 0xBF://cjne
			d1=this->getRAMByte(this->getR(this->instruction[0]&0x07));
			d2=this->instruction[1];
			i_cjne:
			this->CJNE(d1,d2);
			break;
		
		case 0x24:
			d=this->instruction[1];
			goto i_add;
		case 0x25:
			d=this->instruction[1];
			if ((bool)(d&0x80)) d=this->getSFRByteIn(d);
			else d=this->getRAMByte(d);
			goto i_add;
		case 0x26:
			d=this->getRAMByte(this->getRAMByte(this->getR(0)));
			goto i_add;
		case 0x27:
			d=this->getRAMByte(this->getRAMByte(this->getR(1)));
			goto i_add;
		case 0x28:
		case 0x29:
		case 0x2A:
		case 0x2B:
		case 0x2C:
		case 0x2D:
		case 0x2E:
		case 0x2F://add
			d=this->getRAMByte(this->getR(this->instruction[0]&0x07));
			i_add:
			this->ADD(d);
			break;
			
		case 0x34:
			d=this->instruction[1];
			goto i_addc;
		case 0x35:
			d=this->instruction[1];
			if ((bool)(d&0x80)) d=this->getSFRByteIn(d);
			else d=this->getRAMByte(d);
			goto i_addc;
		case 0x36:
			d=this->getRAMByte(this->getRAMByte(this->getR(0)));
			goto i_addc;
		case 0x37:
			d=this->getRAMByte(this->getRAMByte(this->getR(1)));
			goto i_addc;
		case 0x38:
		case 0x39:
		case 0x3A:
		case 0x3B:
		case 0x3C:
		case 0x3D:
		case 0x3E:
		case 0x3F://addc
			d=this->getRAMByte(this->getR(this->instruction[0]&0x07));
			i_addc:
			this->ADDC(d);
			break;
			
		case 0x52:
			a=this->instruction[1];
			d=this->getSFRByteIn(this->ACC);
			goto i_anl;
		case 0x53:
			a=this->instruction[1];
			d=this->instruction[2];
			goto i_anl;
		case 0x54:
			a=this->ACC;
			d=this->instruction[1];
			goto i_anl;
		case 0x55:
			a=this->ACC;
			d=this->instruction[1];
			if ((bool)(d&0x80)) d=this->getSFRByteIn(d);
			else d=this->getRAMByte(d);
			goto i_anl;
		case 0x56:
			a=this->ACC;
			d=this->getRAMByte(this->getRAMByte(this->getR(0)));
			goto i_anl;
		case 0x57:
			a=this->ACC;
			d=this->getRAMByte(this->getRAMByte(this->getR(1)));
			goto i_anl;
		case 0x58:
		case 0x59:
		case 0x5A:
		case 0x5B:
		case 0x5C:
		case 0x5D:
		case 0x5E:
		case 0x5F://anl char
			a=this->ACC;
			d=this->getRAMByte(this->getR(this->instruction[0]&0x07));
			i_anl:
			this->ANL(a,d);
			break;
		
		case 0x82:
			this->ANLcy(this->getBitIn(this->instruction[1]));
			break;
		case 0xB0://anl c
			this->ANLcy(!this->getBitIn(this->instruction[1]));
			break;
			
		case 0xE4://clr a
			this->CLRa();
			break;
		
		case 0xC2:
			this->CLRb(this->instruction[1]);
			break;
		case 0xC3://clr bit
			this->CLRb(this->CY);
			break;
			
		case 0xF4://cpl a
			this->CPLa();
			break;
		
		case 0xB2:
			this->CPLb(this->instruction[1]);
			break;
		case 0xB3://cpl bit
			this->CPLb(this->CY);
			break;
		
		case 0xD4://da
			this->DA();
			break;
			
		case 0x14:
			a=this->ACC;
			goto i_decd;
		case 0x15:
			a=this->instruction[1];
			i_decd:
			this->DECd(a);//!!!!!
			break;
		case 0x16:
			a=this->getRAMByte(this->getR(0));
			goto i_dec;
		case 0x17:
			a=this->getRAMByte(this->getR(1));
			goto i_dec;
		case 0x18:
		case 0x19:
		case 0x1A:
		case 0x1B:
		case 0x1C:
		case 0x1D:
		case 0x1E:
		case 0x1F://dec
			a=this->getR(this->instruction[0]&0x07);
			i_dec:
			this->DEC(a);
			break;
			
		case 0x84://div
			this->DIV();
			break;
		
		case 0xD5:
			a=this->instruction[1];
			this->DJNZd(a);
			break;
		case 0xD8:
		case 0xD9:
		case 0xDA:
		case 0xDB:
		case 0xDC:
		case 0xDD:
		case 0xDE:
		case 0xDF://djnz
			a=this->getR(this->instruction[0]&0x07);
			this->DJNZ(a);
			break;
		
		case 0x04:
			a=this->ACC;
			goto i_incd;
		case 0x05:
			a=this->instruction[1];
			i_incd:
			this->INCd(a);
			break;
		case 0x06:
			a=this->getRAMByte(this->getR(0));
			goto i_inc;
		case 0x07:
			a=this->getRAMByte(this->getR(1));
			goto i_inc;
		case 0x08:
		case 0x09:
		case 0x0A:
		case 0x0B:
		case 0x0C:
		case 0x0D:
		case 0x0E:
		case 0x0F://inc
			a=this->getR(this->instruction[0]&0x07);
			i_inc:
			this->INC(a);
			break;
		
		case 0xA3://inc dptr
			this->INCdptr();
			break;
			
		case 0x20://jb
			this->JB();
			break;
			
		case 0x10://jbc
			this->JBC();
			break;
			
		case 0x40://jc
			this->JC();
			break;
			
		case 0x73://jmp
			this->JMP();
			break;
		
		case 0x30://jnb
			this->JNB();
			break;
			
		case 0x50://jnc
			this->JNC();
			break;
			
		case 0x70://jnz
			this->JNZ();
			break;
			
		case 0x60://jz
			this->JZ();
			break;
			
		case 0x12://lcall
			this->LCALL();
			break;
			
		case 0x02://ljmp
			this->LJMP();
			break;
			
		case 0x74:
			a=this->ACC;
			d=this->instruction[1];
			goto i_movd;
		case 0x75:
			a=this->instruction[1];
			d=this->instruction[2];
			goto i_movd;
		case 0x76:
			a=this->getRAMByte(this->getR(0));
			d=this->instruction[1];
			goto i_mov;
		case 0x77:
			a=this->getRAMByte(this->getR(1));
			d=this->instruction[1];
			goto i_mov;
		case 0x78:
		case 0x79:
		case 0x7A:
		case 0x7B:
		case 0x7C:
		case 0x7D:
		case 0x7E:
		case 0x7F:
			a=this->getR(this->instruction[0]&0x07);
			d=this->instruction[1];
			goto i_mov;
		case 0x85:
			a=this->instruction[2];//!!!!!!!!!!!!!!!!
			d=this->instruction[1];
			if ((bool)(d&0x80)) d=this->getSFRByteIn(d);
			else d=this->getRAMByte(d);
			goto i_movd;
		case 0x86:
			a=this->instruction[1];
			d=this->getRAMByte(this->getRAMByte(this->getR(0)));
			goto i_movd;
		case 0x87:
			a=this->instruction[1];
			d=this->getRAMByte(this->getRAMByte(this->getR(1)));
			goto i_movd;
		case 0x88:
		case 0x89:
		case 0x8A:
		case 0x8B:
		case 0x8C:
		case 0x8D:
		case 0x8E:
		case 0x8F:
			a=this->instruction[1];
			d=this->getRAMByte(this->getR(this->instruction[0]&0x07));
			goto i_movd;
		case 0xA6:
			a=this->getRAMByte(this->getR(0));
			d=this->instruction[1];
			if ((bool)(d&0x80)) d=this->getSFRByteIn(d);
			else d=this->getRAMByte(d);
			goto i_mov;
		case 0xA7:
			a=this->getRAMByte(this->getR(1));
			d=this->instruction[1];
			if ((bool)(d&0x80)) d=this->getSFRByteIn(d);
			else d=this->getRAMByte(d);
			goto i_mov;
		case 0xA8:
		case 0xA9:
		case 0xAA:
		case 0xAB:
		case 0xAC:
		case 0xAD:
		case 0xAE:
		case 0xAF:
			a=this->getR(this->instruction[0]&0x07);
			d=this->instruction[1];
			if ((bool)(d&0x80)) d=this->getSFRByteIn(d);
			else d=this->getRAMByte(d);
			goto i_mov;
		case 0xE5:
			a=this->ACC;
			d=this->instruction[1];
			if ((bool)(d&0x80)) d=this->getSFRByteIn(d);
			else d=this->getRAMByte(d);
			goto i_movd;
		case 0xE6:
			a=this->ACC;
			d=this->getRAMByte(this->getRAMByte(this->getR(0)));
			goto i_movd;
		case 0xE7:
			a=this->ACC;
			d=this->getRAMByte(this->getRAMByte(this->getR(1)));
			goto i_movd;
		case 0xE8:
		case 0xE9:
		case 0xEA:
		case 0xEB:
		case 0xEC:
		case 0xED:
		case 0xEE:
		case 0xEF:
			a=this->ACC;
			d=this->getRAMByte(this->getR(this->instruction[0]&0x07));
			goto i_movd;
		case 0xF5:
			a=this->instruction[1];
			d=this->getSFRByteIn(this->ACC);
			i_movd:
			this->MOVd(a,d);
			break;
		case 0xF6:
			a=this->getRAMByte(this->getR(0));
			d=this->getSFRByteIn(this->ACC);
			goto i_mov;
		case 0xF7:
			a=this->getRAMByte(this->getR(1));
			d=this->getSFRByteIn(this->ACC);
			goto i_mov;
		case 0xF8:
		case 0xF9:
		case 0xFA:
		case 0xFB:
		case 0xFC:
		case 0xFD:
		case 0xFE:
		case 0xFF://mov
			a=this->getR(this->instruction[0]&0x07);
			d=this->getSFRByteIn(this->ACC);
			i_mov:
			this->MOV(a,d);
			break;
		
		case 0x90://mov dptr
			this->MOVdptr();
			break;
		
		case 0x92:
			this->MOVb(this->instruction[1],this->CY);
			break;
		case 0xA2://mov bit
			this->MOVb(this->CY,this->instruction[1]);
			break;
			
		case 0x83:
			this->MOVC(this->PC);
			break;
		case 0x93://movc
			ptrs=(((unsigned short)this->getSFRByteIn(this->DPH))<<8)|((unsigned short)this->getSFRByteIn(this->DPL));
			this->MOVC(ptrs);
			break;
			
		case 0xE0:
			ptrs=(((unsigned short)this->getSFRByteIn(this->DPH))<<8)|((unsigned short)this->getSFRByteIn(this->DPL));
			this->MOVXin(ptrs);
			break;
		case 0xE2:
			ptrc=this->getRAMByte(this->getR(0));
			this->MOVXin(ptrc);
			break;
		case 0xE3:
			ptrc=this->getRAMByte(this->getR(1));
			this->MOVXin(ptrc);
			break;
		case 0xF0:
			ptrs=(((unsigned short)this->getSFRByteIn(this->DPH))<<8)|((unsigned short)this->getSFRByteIn(this->DPL));
			this->MOVXout(ptrs);
			break;
		case 0xF2:
			ptrc=this->getRAMByte(this->getR(0));
			this->MOVXout(ptrc);
			break;
		case 0xF3://movx
			ptrc=this->getRAMByte(this->getR(1));
			this->MOVXout(ptrc);
			break;
			
		case 0xA4:
			this->MUL();
			break;
			
		case 0x42:
			a=this->instruction[1];
			d=this->getSFRByteIn(this->ACC);
			goto i_orl;
		case 0x43:
			a=this->instruction[1];
			d=this->instruction[2];
			goto i_orl;
		case 0x44:
			a=this->ACC;
			d=this->instruction[1];
			goto i_orl;
		case 0x45:
			a=this->ACC;
			d=this->instruction[1];
			if ((bool)(d&0x80)) d=this->getSFRByteIn(d);
			else d=this->getRAMByte(d);
			goto i_orl;
		case 0x46:
			a=this->ACC;
			d=this->getRAMByte(this->getRAMByte(this->getR(0)));
			goto i_orl;
		case 0x47:
			a=this->ACC;
			d=this->getRAMByte(this->getRAMByte(this->getR(1)));
			goto i_orl;
		case 0x48:
		case 0x49:
		case 0x4A:
		case 0x4B:
		case 0x4C:
		case 0x4D:
		case 0x4E:
		case 0x4F://orl
			a=this->ACC;
			d=this->getRAMByte(this->getR(this->instruction[0]&0x07));
			i_orl:
			this->ORL(a,d);
			break;
			
		case 0x72:
			this->ORLcy(this->getBitIn(this->instruction[1]));
			break;
		case 0xA0://orl cy
			this->ORLcy(!this->getBitIn(this->instruction[1]));
			break;
			
		case 0xD0://pop
			this->POP();
			break;
			
		case 0xC0://push
			this->PUSH();
			break;
		
		case 0x22://ret
			this->RET();
			break;
			
		case 0x32://reti
			this->RETI();
			break;
			
		case 0x23://rl
			this->RL();
			break;
			
		case 0x33://rlc
			this->RLC();
			break;
			
		case 0x03://rr
			this->RR();
			break;
			
		case 0x13://rrc
			this->RRC();
			break;
			
		case 0xD2:
			this->SETB(this->instruction[1]);
			break;
		case 0xD3://setb
			this->SETB(this->CY);
			break;
			
		case 0x80://sjmp
			this->SJMP();
			break;
			
		case 0x94:
			d=this->instruction[1];
			goto i_subb;
		case 0x95:
			d=this->instruction[1];
			if ((bool)(d&0x80)) d=this->getSFRByteIn(d);
			else d=this->getRAMByte(d);
			goto i_subb;
		case 0x96:
			d=this->getRAMByte(this->getRAMByte(this->getR(0)));
			goto i_subb;
		case 0x97:
			d=this->getRAMByte(this->getRAMByte(this->getR(1)));
			goto i_subb;
		case 0x98:
		case 0x99:
		case 0x9A:
		case 0x9B:
		case 0x9C:
		case 0x9D:
		case 0x9E:
		case 0x9F://subb
			d=this->getRAMByte(this->getR(this->instruction[0]&0x07));
			i_subb:
			this->SUBB(d);
			break;
			
		case 0xC4://swap
			this->SWAP();
			break;
			
		case 0xC5:
			a=this->instruction[1];
			this->XCHd(a);
			break;
		case 0xC6:
			a=this->getRAMByte(this->getR(0));
			goto i_xch;
		case 0xC7:
			a=this->getRAMByte(this->getR(1));
			goto i_xch;
		case 0xC8:
		case 0xC9:
		case 0xCA:
		case 0xCB:
		case 0xCC:
		case 0xCD:
		case 0xCE:
		case 0xCF://xch
			a=this->getR(this->instruction[0]&0x07);
			i_xch:
			this->XCH(a);
			break;
			
		case 0xD6:
			this->XCHD(this->getRAMByte(this->getR(0)));
			break;
		case 0xD7://xchd
			this->XCHD(this->getRAMByte(this->getR(1)));
			break;
			
		case 0x62:
			a=this->instruction[1];
			d=this->getSFRByteIn(this->ACC);
			goto i_xrl;
		case 0x63:
			a=this->instruction[1];
			d=this->instruction[2];
			goto i_xrl;
		case 0x64:
			a=this->ACC;
			d=this->instruction[1];
			goto i_xrl;
		case 0x65:
			a=this->ACC;
			d=this->instruction[1];
			if ((bool)(d&0x80)) d=this->getSFRByteIn(d);
			else d=this->getRAMByte(d);
			goto i_xrl;
		case 0x66:
			a=this->ACC;
			d=this->getRAMByte(this->getRAMByte(this->getR(0)));
			goto i_xrl;
		case 0x67:
			a=this->ACC;
			d=this->getRAMByte(this->getRAMByte(this->getR(1)));
			goto i_xrl;
		case 0x68:
		case 0x69:
		case 0x6A:
		case 0x6B:
		case 0x6C:
		case 0x6D:
		case 0x6E:
		case 0x6F://xrl
			a=this->ACC;
			d=this->getRAMByte(this->getR(this->instruction[0]&0x07));
			i_xrl:
			this->XRL(a,d);
			break;
	}
}
void m80C32::ACALL(){
	//printf("ACALL\n");
	unsigned char sp=this->getSFRByteIn(this->SP)+1;
	this->setRAMByte(sp,(unsigned char)(this->PC&0xFF));
	sp++;
	this->setRAMByte(sp,(unsigned char)(this->PC>>8));
	this->setSFRByte(this->SP,sp);
	this->AJMP();
}
void m80C32::AJMP(){
	//printf("AJMP\n");
	this->PC=(this->PC&0xF800)|(((unsigned short)(this->instruction[0]>>5))<<8)|((unsigned short)this->instruction[1]);
}
void m80C32::ADD(unsigned char d){
	//printf("ADD\n");
	unsigned char a=this->getSFRByteIn(this->ACC);
	this->setBitIn(this->CY,(a>UCHAR_MAX-d));
	this->setBitIn(this->AC,((a&0x0F)>(NIBBLE_MAX-(d&0x0F))));
	if (((signed char)d)>0){
		this->setBitIn(this->OV,(((signed char)a)>SCHAR_MAX-((signed char)d)));
	}
	else{
		this->setBitIn(this->OV,(((signed char)a)<SCHAR_MIN-((signed char)d)));
	}
	this->setSFRByte(this->ACC,a+d);
}
void m80C32::ADDC(unsigned char d){
	//printf("ADDC\n");
	if (this->getBitIn(this->CY)) d++;
	this->ADD(d);
}
void m80C32::ANL(unsigned char a,unsigned char d){
	//printf("ANL\n");
	if ((bool)(a&0x80)) this->setSFRByte(a,this->getSFRByteOut(a)&d);
	else this->setRAMByte(a,this->getRAMByte(a)&d);
}
void m80C32::ANLcy(bool b){
	//printf("ANLcy\n");
	if (!b) this->setBitIn(this->CY,false);
}
void m80C32::CJNE(unsigned char d1,unsigned char d2){
	//printf("CJNE\n");
	this->setBitIn(this->CY,d1<d2);
	if (d1!=d2){
		this->PC+=(unsigned short)(signed char)this->instruction[2];
	}
}
void m80C32::CLRa(){
	//printf("CLRa\n");
	this->setSFRByte(this->ACC,0);
}
void m80C32::CLRb(unsigned char a){
	//printf("CLRb\n");
	if ((bool)(a&0x80)){
		unsigned char bit;
		this->bitaddress2address(&a,&bit);
		unsigned char v=this->getSFRByteOut(a);//!!!
		v&=~(1<<bit);
		this->setSFRByte(a,v);
	}
	else{
		this->setBitIn(a,0);
	}
}
void m80C32::CPLa(){
	//printf("CPLa\n");
	this->setSFRByte(this->ACC,this->getSFRByteIn(this->ACC)^0xFF);
}
void m80C32::CPLb(unsigned char a){
	//printf("CPLb\n");
	if ((bool)(a&0x80)){
		unsigned char bit;
		this->bitaddress2address(&a,&bit);
		unsigned char v=this->getSFRByteOut(a);//!!!
		v^=(1<<bit);
		this->setSFRByte(a,v);
	}
	else{
		this->setBitIn(a,!this->getBitIn(a));
	}
}
void m80C32::DA(){
	//printf("DA\n");
	unsigned char a=this->getSFRByteIn(this->ACC);
	if (this->getBitIn(this->AC)||(a&0x0F)>9){
		this->setBitIn(this->CY,(a>UCHAR_MAX-0x06)||this->getBitIn(this->CY));
		a+=0x06;
	}
	if (this->getBitIn(this->CY)||a>0x9F){
		this->setBitIn(this->CY,(a>UCHAR_MAX-0x60)||this->getBitIn(this->CY));
		a+=0x60;
	}
	this->setSFRByte(this->ACC,a);
}
void m80C32::DEC(unsigned char a){
	//printf("DEC\n");
	unsigned char v=this->getRAMByte(a)-1;
	this->setRAMByte(a,v);
}
void m80C32::DECd(unsigned char a){
	//printf("DECd\n");
	if ((bool)(a&0x80)){
		unsigned char v=this->getSFRByteOut(a)-1;
		this->setSFRByte(a,v);
	}
	else this->DEC(a);
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
void m80C32::DJNZ(unsigned char a){
	//printf("DJNZ\n");
	unsigned char v=this->getRAMByte(a)-1;
	if (v!=0){
		this->PC+=(unsigned short)(signed char)this->instruction[1];
	}
	this->setRAMByte(a,v);
}
void m80C32::DJNZd(unsigned char a){
	//printf("DJNZd\n");
	if ((bool)(a&0x80)){
		unsigned char v=this->getSFRByteOut(a)-1;
		if (v!=0){
			this->PC+=(unsigned short)(signed char)this->instruction[1];
		}
		this->setSFRByte(a,v);
	}
	else DJNZ(a);
}
void m80C32::INC(unsigned char a){
	//printf("INC\n");
	this->setRAMByte(a,this->getRAMByte(a)+1);
}
void m80C32::INCd(unsigned char a){
	//printf("INCd\n");
	if ((bool)(a&0x80)) this->setSFRByte(a,this->getSFRByteOut(a)+1);
	else INC(a);
}
void m80C32::INCdptr(){
	//printf("INCdptr\n");
	unsigned short dptr=((unsigned short)this->getSFRByteIn(this->DPL))|(((unsigned short)this->getSFRByteIn(this->DPH))<<8);
	dptr++;
	this->setSFRByte(this->DPL,(unsigned char)(dptr&0xFF));
	this->setSFRByte(this->DPH,(unsigned char)(dptr>>8));
}
void m80C32::JB(){
	//printf("JB\n");
	if (getBitIn(this->instruction[1]))this->PC+=(unsigned short)(signed char)this->instruction[2];
}
void m80C32::JBC(){
	//printf("JBC\n");
	unsigned char bit;
	unsigned char address=this->instruction[1];
	this->bitaddress2address(&address,&bit);
	unsigned char mask=1<<bit;
	unsigned char d;
	if ((bool)(address&0x80)){
		d=this->getSFRByteOut(address);
		this->setSFRByte(address,d&(~mask));
	}
	else{
		d=this->getRAMByte(address);
		this->setRAMByte(address,d&(~mask));
	}
	if ((d&mask)!=0){
		this->PC+=(unsigned short)(signed char)this->instruction[2];
	}
}
void m80C32::JC(){
	//printf("JC\n");
	if (getBitIn(this->CY))this->PC+=(unsigned short)(signed char)this->instruction[1];
}
void m80C32::JMP(){
	//printf("JMP\n");
	unsigned short dptr=((unsigned short)this->getSFRByteIn(this->DPL))|(((unsigned short)this->getSFRByteIn(this->DPH))<<8);
	dptr+=(unsigned short)this->getSFRByteIn(this->ACC);
	this->PC=dptr;
}
void m80C32::JNB(){
	//printf("JNB\n");
	if (!getBitIn(this->instruction[1]))this->PC+=(unsigned short)(signed char)this->instruction[2];
}
void m80C32::JNC(){
	//printf("JNC\n");
	if (!getBitIn(this->CY))this->PC+=(unsigned short)(signed char)this->instruction[1];
}
void m80C32::JNZ(){
	//printf("JNZ\n");
	if (getSFRByteIn(this->ACC)!=0)this->PC+=(unsigned short)(signed char)this->instruction[1];
}
void m80C32::JZ(){
	//printf("JZ\n");
	if (getSFRByteIn(this->ACC)==0)this->PC+=(unsigned short)(signed char)this->instruction[1];
}
void m80C32::LCALL(){
	//printf("LCALL\n");
	unsigned char sp=this->getSFRByteIn(this->SP)+1;
	this->setRAMByte(sp,(unsigned char)(this->PC&0xFF));
	sp++;
	this->setRAMByte(sp,(unsigned char)(this->PC>>8));
	this->setSFRByte(this->SP,sp);
	this->LJMP();
}
void m80C32::LJMP(){
	//printf("LJMP\n");
	this->PC=(((unsigned short)this->instruction[1])<<8)|((unsigned short)this->instruction[2]);
}
void m80C32::MOV(unsigned char a,unsigned char d){
	//printf("MOV\n");
	this->setRAMByte(a,d);
}
void m80C32::MOVd(unsigned char a,unsigned char d){
	//printf("MOVd\n");
	if ((bool)(a&0x80)) this->setSFRByte(a,d);
	else MOV(a,d);
}
void m80C32::MOVdptr(){
	//printf("MOVdptr\n");
	this->setSFRByte(this->DPH,this->instruction[1]);
	this->setSFRByte(this->DPL,this->instruction[2]);
}
void m80C32::MOVb(unsigned char ba1,unsigned char ba2){
	//printf("MOVb\n");
	unsigned char bit;
	this->bitaddress2address(&ba1,&bit);
	if ((bool)(ba1&0x80)){
		unsigned char d=this->getSFRByteOut(ba1);
		unsigned char mask=1<<bit;
		d&=~mask;
		d|=this->getBitIn(ba2)?mask:0x00;
		this->setSFRByte(ba1,d);
	}
	else{
		unsigned char d=this->getRAMByte(ba1);
		unsigned char mask=1<<bit;
		d&=~mask;
		d|=this->getBitIn(ba2)?mask:0x00;
		this->setRAMByte(ba1,d);
	}
}
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
void m80C32::MOVXin(unsigned char a){//8 bit address
	//printf("MOVXin 8bit\n");
	this->sendP3((this->PX_out[3]&0x3F)|0xC0);
	this->sendP0(a);
	this->sendALE(true);
	this->sendALE(false);
	this->sendP3((this->PX_out[3]&0x3F)|0x40);
	this->setSFRByte(this->ACC,this->getSFRByteIn(this->P0));
	this->sendP3((this->PX_out[3]&0x3F)|0xC0);
	this->sendP3(this->PX_out[3]);
	this->sendP0(this->PX_out[0]);
}
void m80C32::MOVXin(unsigned short a){//16 bit address
	//printf("MOVXin 16bit\n");
	this->sendP3((this->PX_out[3]&0x3F)|0xC0);
	this->sendP0((unsigned char)(a&0xFF));
	this->sendP2((unsigned char)(a>>8));
	this->sendALE(true);
	this->sendALE(false);
	this->sendP3((this->PX_out[3]&0x3F)|0x40);
	this->setSFRByte(this->ACC,this->getSFRByteIn(this->P0));
	this->sendP3((this->PX_out[3]&0x3F)|0xC0);
	this->sendP3(this->PX_out[3]);
	this->sendP0(this->PX_out[0]);
	this->sendP2(this->PX_out[2]);
}
void m80C32::MOVXout(unsigned char a){//8 bit address
	//printf("MOVXout 8bit\n");
	this->sendP3((this->PX_out[3]&0x3F)|0xC0);
	this->sendP0(a);
	this->sendALE(true);
	this->sendALE(false);
	this->sendP3((this->PX_out[3]&0x3F)|0x80);
	this->sendP0(this->getSFRByteIn(this->ACC));
	this->sendP3((this->PX_out[3]&0x3F)|0xC0);
	this->sendP3(this->PX_out[3]);
	this->sendP0(this->PX_out[0]);
}
void m80C32::MOVXout(unsigned short a){//16 bit address
	//printf("MOVXout 16bit\n");
	this->sendP3((this->PX_out[3]&0x3F)|0xC0);
	this->sendP0((unsigned char)(a&0xFF));
	this->sendALE(true);
	this->sendALE(false);
	this->sendP2((unsigned char)(a>>8));
	this->sendP3((this->PX_out[3]&0x3F)|0x80);
	this->sendP0(this->getSFRByteIn(this->ACC));
	this->sendP3((this->PX_out[3]&0x3F)|0xC0);
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
void m80C32::ORL(unsigned char a,unsigned char d){
	//printf("ORL\n");
	if ((bool)(a&0x80)) this->setSFRByte(a,this->getSFRByteOut(a)|d);
	else this->setRAMByte(a,this->getRAMByte(a)|d);
}
void m80C32::ORLcy(bool b){
	printf("ORLcy\n");
	if (b) this->setBitIn(this->CY,true);
}
void m80C32::POP(){
	//printf("POP\n");
	unsigned char sp=this->getSFRByteIn(this->SP);
	if ((bool)(this->instruction[1]&0x80)) this->setSFRByte(this->instruction[1],this->getRAMByte(sp));
	else this->setRAMByte(this->instruction[1],this->getRAMByte(sp));
	sp--;
	this->setSFRByte(this->SP,sp);
}
void m80C32::PUSH(){
	//printf("PUSH\n");
	unsigned char sp=this->getSFRByteIn(this->SP);
	sp++;
	this->setRAMByte(sp,((bool)(this->instruction[1]&0x80))?this->getSFRByteIn(this->instruction[1]):this->getRAMByte(this->instruction[1]));
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
	this->RET();
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
void m80C32::SETB(unsigned char ba){
	//printf("SETB\n");
	if ((bool)(ba&0x80)){
		unsigned char bit;
		this->bitaddress2address(&ba,&bit);
		unsigned char d=this->getSFRByteOut(ba);
		d|=1<<bit;
		this->setSFRByte(ba,d);
	}
	else this->setBitIn(ba,true);
}
void m80C32::SJMP(){
	//printf("SJMP\n");
	this->PC+=(unsigned short)(signed char)this->instruction[1];
}
void m80C32::SUBB(unsigned char d){
	//printf("SUBB\n");
	if (this->getBitIn(this->CY)) d++;
	unsigned char a=this->getBitIn(this->ACC);
	this->setBitIn(this->CY,(a>UCHAR_MAX+d));
	this->setBitIn(this->AC,((a&0x0F)>(NIBBLE_MAX+(d&0x0F))));
	if (((signed char)d)<0){
		this->setBitIn(this->OV,(((signed char)a)>SCHAR_MAX+((signed char)d)));
	}
	else{
		this->setBitIn(this->OV,(((signed char)a)<SCHAR_MIN+((signed char)d)));
	}
	this->setSFRByte(this->ACC,a-d);
}
void m80C32::SWAP(){
	//printf("SWAP\n");
	unsigned char a=this->getSFRByteIn(this->ACC);
	a=(a>>4)|(a<<4);
	this->setSFRByte(this->ACC,a);
}
void m80C32::XCH(unsigned char a){
	//printf("XCH\n");
	unsigned char d=this->getSFRByteIn(this->ACC);
	this->setSFRByte(this->ACC,this->getRAMByte(a));//getCharIn???
	this->setRAMByte(a,d);
}
void m80C32::XCHd(unsigned char a){
	//printf("XCHd\n");
	if ((bool)(a&0x80)){
		unsigned char d=this->getSFRByteIn(this->ACC);
		this->setSFRByte(this->ACC,this->getSFRByteIn(a));//getCharIn???
		this->setSFRByte(a,d);
	}
	else XCH(a);
}
void m80C32::XCHD(unsigned char a){
	//printf("XCHD\n");
	unsigned char d1=this->getSFRByteIn(this->ACC);
	unsigned char d2=this->getRAMByte(a);
	this->setSFRByte(this->ACC,(d1&0xF0)|(d2&0x0F));//getCharIn???
	this->setRAMByte(a,(d2&0xF0)|(d1&0x0F));
}
void m80C32::XRL(unsigned char a,unsigned char d){
	//printf("XRL\n");
	if ((bool)(a&0x80)) this->setSFRByte(a,d^this->getSFRByteOut(a));
	else this->setRAMByte(a,d^this->getRAMByte(a));
}