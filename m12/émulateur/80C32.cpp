#include <limits.h>
#define NIBBLE_MAX 15

class m80C32{
	public:
		unsigned char iRAM[256];
		unsigned char PX_out[4];
		unsigned char SBUF_out;
		unsigned char SBUF_in;//buffer
		bool TEN=false
		
		void m80C32(){
			this->Reset();
			auto vc=[](unsigned char d){};
			auto vb=[](bool b){};
			this->sendP0=vc;
			this->sendP1=vc;
			this->sendP2=vc;
			this->sendP3=vc;
			this->sendnRD=vb;
			this->sendnWR=vb;
			this->sendTxD=vb;
			this->sendRxD=vb;
			this->sendALE=vb;
			this->sendnPSEN=vb;
		}
		
		void CLKTickIn();
		void ResetChangeIn(bool);
		void PXChangeIn(unsigned char,unsigned char);
		void subscribeP0(void (*f)(unsigned char)){this->sendP0=f;}
		void subscribeP1(void (*f)(unsigned char)){this->sendP1=f;}
		void subscribeP2(void (*f)(unsigned char)){this->sendP2=f;}
		void subscribeP3(void (*f)(unsigned char)){this->sendP3=f;}
		void subscribenRD(void (*f)(bool)){this->sendnRD=f;}
		void subscribenWR(void (*f)(bool)){this->sendnWR=f;}
		void subscribeTxD(void (*f)(bool)){this->sendTxD=f;}
		void subscribeRxD(void (*f)(bool)){this->sendRxD=f;}
		void subscribeALE(void (*f)(bool)){this->sendALE=f;}
		void subscribenPSEN(void (*f)(bool)){this->sendPSEN=f;}
		
		
	private:
	
		const unsigned char periodPerCycle=12;
		
		//address
		const unsigned char ACC=0xE0;
		const unsigned char B=0xF0;
		const unsigned char DPH=0x83;
		const unsigned char DPL=0x82;
		const unsigned char IE=0xA8;
		const unsigned char IP=0xB8;
		const unsigned char P0=0x80;
		const unsigned char P1=0x90;
		const unsigned char P2=0xA0;
		const unsigned char P3=0xB0;
		const unsigned char PCON=0x87;
		const unsigned char PSW=0xD0;
		const unsigned char RCAP2H=0xCB;
		const unsigned char RCAP2L=0xCA;
		const unsigned char SBUF=0x99;
		const unsigned char SCON=0x98;
		const unsigned char SP=0x81;
		const unsigned char TCON=0x88;
		const unsigned char T2CON=0xC8;
		const unsigned char TH0=0x8C;
		const unsigned char TH1=0x8D;
		const unsigned char TH2=0xCD;
		const unsigned char TL0=0x8A;
		const unsigned char TL1=0x8B;
		const unsigned char TL2=0xCC;
		const unsigned char TMOD=0x89;
		
		//bit address
		const unsigned char EA=0xAF;
		const unsigned char ET2=0xAD;
		const unsigned char ES=0xAC;
		const unsigned char ET1=0xAB;
		const unsigned char EX1=0xAA
		const unsigned char ET0=0xA9;
		const unsigned char EX0=0xA8;
		const unsigned char PT2=0xBD;
		const unsigned char PS=0xBC;
		const unsigned char PT1=0xBB;
		const unsigned char PX1=0xBA
		const unsigned char PT0=0xB9;
		const unsigned char PX0=0xB8;
		const unsigned char T2EX=0x91;
		const unsigned char T2=0x90;
		const unsigned char nRD=0xB7;
		const unsigned char nWR=0xB6;
		const unsigned char T1=0xB5;
		const unsigned char T0=0xB4;
		const unsigned char nINT1=0xB3;
		const unsigned char nINT0=0xB2;
		const unsigned char TxD=0xB1;
		const unsigned char RxD=0xB0;
		const unsigned char CY=0xD7;
		const unsigned char AC=0xD6;
		const unsigned char F0=0xD5;
		const unsigned char RS1=0xD4;
		const unsigned char RS0=0xD3;
		const unsigned char OV=0xD2;
		const unsigned char P=0xD0;
		const unsigned char SM0=0x9F;
		const unsigned char SM1=0x9E;
		const unsigned char SM2=0x9D;
		const unsigned char REN=0x9C;
		const unsigned char TB8=0x9B;
		const unsigned char RB8=0x9A;
		const unsigned char TI=0x99;
		const unsigned char RI=0x98;
		const unsigned char TF1=0x8F;
		const unsigned char TR1=0x8E;
		const unsigned char TF0=0x8D;
		const unsigned char TR0=0x8C;
		const unsigned char IE1=0x8B;
		const unsigned char IT1=0x8A;
		const unsigned char IE0=0x89;
		const unsigned char IT0=0x88;
		const unsigned char TF2=0xCF;
		const unsigned char EXF2=0xCE;
		const unsigned char RCLK=0xCD;
		const unsigned char TCLK=0xCC;
		const unsigned char EXEN2=0xCB;
		const unsigned char TR2=0xCA;
		const unsigned char C_nT2=0xC9;
		const unsigned char CP_nRL2=0xC8;
		
		//num bit
		//PCON
		const unsigned char SMOD=7;
		const unsigned char GF1=3;
		const unsigned char GF0=2;
		const unsigned char PD=1;
		const unsigned char IDL=0;
		//TMOD
		const unsigned char GATE_1=7;
		const unsigned char C_T_1=6;
		const unsigned char M1_1=5;
		const unsigned char M0_1=4;
		const unsigned char GATE_0=3;
		const unsigned char C_T_0=2;
		const unsigned char M1_0=1;
		const unsigned char M0_0=0;
	
		const unsigned char i_length[256]={
			1,2,3,1,1,2,1,1,1,1,1,1,1,1,1,1,
			3,2,3,1,1,2,1,1,1,1,1,1,1,1,1,1,
            3,2,1,1,2,2,1,1,1,1,1,1,1,1,1,1,
            3,2,1,1,2,2,1,1,1,1,1,1,1,1,1,1,
            2,2,2,3,2,2,1,1,1,1,1,1,1,1,1,1,
            2,2,2,3,2,2,1,1,1,1,1,1,1,1,1,1,
            2,2,2,3,2,2,1,1,1,1,1,1,1,1,1,1,
            2,2,2,1,2,3,2,2,2,2,2,2,2,2,2,2,
            2,2,2,1,1,3,2,2,2,2,2,2,2,2,2,2,
            3,2,2,1,2,2,1,1,1,1,1,1,1,1,1,1,
            2,2,2,1,1,1,2,2,2,2,2,2,2,2,2,2,
            2,2,2,1,3,3,3,3,3,3,3,3,3,3,3,3,
            2,2,2,1,1,2,1,1,1,1,1,1,1,1,1,1,
            2,2,2,1,1,3,1,1,2,2,2,2,2,2,2,2,
            1,2,1,1,1,2,1,1,1,1,1,1,1,1,1,1,
            1,2,1,1,1,2,1,1,1,1,1,1,1,1,1,1
			};
		const unsigned char i_cycle[256]={
			1,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,
			2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,
			2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,
			2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,
			2,2,1,2,1,1,1,1,1,1,1,1,1,1,1,1,
			2,2,1,2,1,1,1,1,1,1,1,1,1,1,1,1,
			2,2,1,2,1,1,1,1,1,1,1,1,1,1,1,1,
			2,2,2,2,1,2,1,1,1,1,1,1,1,1,1,1,
			2,2,2,2,4,2,2,2,2,2,2,2,2,2,2,2,
			2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,
			2,2,1,2,4,1,2,2,2,2,2,2,2,2,2,2,
			2,2,1,1,2,2,2,2,2,2,2,2,2,2,2,2,
			2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
			2,2,1,1,1,2,1,1,2,2,2,2,2,2,2,2,
			2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,
			2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1
			};
		
		void bitaddress2address(unsigned char* address, unsigned char* bit){
			*bit=*address&0x07;
			if (address>=0x80){
				*address=*address&0xF8;
			}
			else{
				*address=0x20+(*address>>3);
			}
		}
		
		bool getBitIn(unsigned char address){
			unsigned char bit;
			this->bitaddress2address(&address,&bit);
			return (bool)((this->getCharIn(address)>>bit)&0x01);
		}
		//change state + callback for PX port change
		void SetBitIn(unsigned char address, bool b){
			unsigned char bit;
			this->bitaddress2address(&address,&bit);
			unsigned char mask=1<<bit;
			unsigned char v=this->getCharIn(address);
			v&=~mask;
			v|=b?mask:0x00;
			SetChar(address,v);
		}
		
		unsigned char getCharIn(unsigned char address){
			return this->iRAM[address];
		}
		//for read-modify-write instruction
		unsigned char getCharOut(unsigned char address){
			switch (address){
				case this->P0:
				case this->P1:
				case this->P2:
				case this->P3:
					unsigned char X=(address>>8)&0x03;
					return this->PX_out[X];
				default:
			}
			return this->iRAM[address];
		}
		//change state + callback for PX port change
		void SetChar(unsigned char address, unsigned char d){
			switch (address){
				case this->P0:
				case this->P1:
				case this->P2:
				case this->P3:
					if (this->getCharIn(address)^d!=0){
						this->PXChange(a,d);
					}
					break;
					
				case this->IE:
				case this->IP:
					this->iRAM[address]=d;
					this->interrupt_change=true;
					break;
					
				case this->ACC:
					this->iRAM[address]=d;
					this->setACCParity();
					break;
					
				case this->SBUF:
					this->SBUF_out=d;
					this->TEN=true;
					break;
					
				case this->PCON:
					this->iRAM[address]=d;
					this->PCONChange();
					break;
				
				default:
					this->iRAM[address]=d;
			}
		}
		
		unsigned char getR(unsigned char r){
			return (this->getCharIn(this->PSW)&0x18)|r;
		}
		
		void PXChange(unsigned char address, unsigned char d){
			unsigned char X=(address>>8)&0x03;
			this->PX_out[X]=d;
			switch (x){
				case 0:
					(*this->sendP0)(d);
				case 1:
					(*this->sendP1)(d);
				case 2:
					(*this->sendP2)(d);
				case 3:
					(*this->sendP3)(d);
			}
		}
		void SBUFChange(unsigned char d){
			this->SBUF_out=d;
			this->TEN=true;
		}
		
		unsigned char period=0;
			
		unsigned short PC=0;
		
		unsigned char instruction[3]={0,0,0};
		unsigned char i_cycle_n=0xFF;
		unsigned char i_part_n;
		
		void ACALL();
		void AJMP();
		void ADD(unsigned char);
		void ADDC(unsigned char);
		void ANL(unsigned char,unsigned char)
		void ANLcy(bool);
		void CJNE(unsigned char,unsigned char);
		void CLRa();
		void CLRb(unsigned char);
		void CPLa();
		void CPLb(unsigned char);
		void DA();
		void DEC(unsigned char);
		void DIV();
		void DJNZ(unsigned char);
		void INC(unsigned char);
		void INCdptr();
		void JB();
		void JBC();
		void JC();
		void JMP();
		void JNB();
		void JNC();
		void JNZ();
		void JZ();
		void LCALL();
		void LJMP();
		void MOV(unsigned char,unsigned char);
		void MOVdptr();
		void MOVb(unsigned char,unsigned char);
		void MOVC(unsigned short);
		void MOVXin(unsigned char);
		void MOVXin(unsigned short);
		void MOVXout(unsigned char);
		void MOVXout(unsigned short);
		void MUL();
		void ORL(unsigned char,unsigned char);
		void ORLcy(bool);
		void POP();
		void PUSH();
		void RET();
		void RETI();
		void RL();
		void RLC();
		void RR();
		void RRC();
		void SETB(unsigned char);
		void SJMP();
		void SUBB(unsigned char);
		void SWAP();
		void XCH(unsigned char);
		void XCHD(unsigned char);
		void XRL(unsigned char,unsigned char);
		
		void nextCycleALU();
		void execInstruction();
		void setACCParity();
		
		
		unsigned char interrupt_level=0;
		bool interrupt_change=false;
		
		void checkInterrupts();
		void decreaseInterruptLevel();
		
		void ResetCountdown();
		void Reset();
		void Idle();
		void PowerDown();
		void PCONChange();
		
		bool reset_level=true;
		unsigned char reset_count=0;
		
		void (*sendP0)(unsigned char);
		void (*sendP1)(unsigned char);
		void (*sendP2)(unsigned char);
		void (*sendP3)(unsigned char);
		void (*sendnRD)(bool);
		void (*sendnWR)(bool);
		void (*sendTxD)(bool);
		void (*sendRxD)(bool);
		void (*sendALE)(bool);
		void (*sendnPSEN)(bool);
		void checkPortChangeConsequences(unsigned char,unsigned char);
		
		void T2EXFall();
		void T2Tick();
		void nINT0Fall();
		void nINT1Fall();
		void T0Tick();
		void T1Tick();
		void T2SerialClockTick();
		void T1SerialClockTick();
		void fixedSerialClockTick();
		void RXClockTickX4();
		void TXClockTickX4();
		void updateRX();
		void updateTX();
		
		unsigned char TX_bit=0;
		unsigned char RX_bit=0;
		bool RX_state=true;
}

/*
=============== IO ===============
*/
void m80C32::CLKTickIn(){
	this->fixedSerialClockTick();
	
	
	this->period++;
	if (this->period&0x01==1) return;// f/2->state time
	
	unsigned char t2con=this->getCharIn(this->T2CON);
	unsigned char t2con_mask1=1<<(this->C_nT2&0x07);
	unsigned char t2con_mask2=(1<<(this->C_nT2&0x07))|(1<<(this->RCLK&0x07))|(1<<(this->TCLK&0x07));
	if (t2con&t2con_mask1==0&&t2con&t2con_mask2!=0) this->T2Tick();
	
	if (this->period<this->periodPerCycle) return;
	this->period=0;
	
	if (t2con&t2con_mask2==0) this->T2Tick();
	if (!this->getBitIn(this->C_T_0)) this->T0Tick();
	if (!this->getBitIn(this->C_T_1)) this->T1Tick();
	
	this->ResetCountdown();
	unsigned char pd_mask=1<<this->PD;
	unsigned char idl_mask=1<<this->IDL;
	unsigned char power_mode=this->getCharIn(this->PCON)&(pd_mask|idl_mask);
	if (power_mode&pd_mask==0){
		if (power_mode&idl_mask==0){
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
	unsigned char v=this->iRAM[x];
	this->iRAM[x]=d;//don't trigger checkWriteConsequences!!!
	this->checkPortChangeConsequences(x,v^d);
	
}
void m80C32::checkPortChangeConsequences(unsigned char a,unsigned char mask){
	if (a==this->P3){
		mask=mask&(~this->getCharIn(a));//port fall mask
		//nINT0
		//nINT1
		if ((mask>>(this->nINT0&0x07))&0x01!=0){
			this->nINT0Fall();
		}
		if ((mask>>(this->nINT1&0x07))&0x01!=0){
			this->nINT1Fall();
		}
		//T0
		//T1
		if ((mask>>(this->T0&0x07))&0x01!=0){
			if (this->getBitIn(this->C_T_0)){
				this->T0Tick();
			}
		}
		if ((mask>>(this->T1&0x07))&0x01!=0){
			if (this->getBitIn(this->C_T_1)){
				this->T1Tick();
			}
		}
	}
	else if (a==this->P1){
		mask=mask&(~this->getCharIn(a));//port fall mask
		if ((mask>>(this->T2EX&0x07))&0x01!=0){
			this->T2EXFall();
		}
		if ((mask>>(this->T2&0x07))&0x01!=0){
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
	this->setChar(this->ACC,0x00);
	this->setChar(this->B,0x00);
	this->setChar(this->DPH,0x00);
	this->setChar(this->DPL,0x00);
	this->SetBitIn(this->EA,false);
	this->SetBitIn(this->ET2,false);
	this->SetBitIn(this->ES,false);
	this->SetBitIn(this->ET1,false);
	this->SetBitIn(this->EX1,false);
	this->SetBitIn(this->ET0,false);
	this->SetBitIn(this->EX0,false);
	this->SetBitIn(this->PT2,false);
	this->SetBitIn(this->PS,false);
	this->SetBitIn(this->PT1,false);
	this->SetBitIn(this->PX1,false);
	this->SetBitIn(this->PT0,false);
	this->SetBitIn(this->PX0,false);
	this->setChar(this->P0,0xFF);
	this->setChar(this->P1,0xFF);
	this->setChar(this->P2,0xFF);
	this->setChar(this->P3,0xFF);
	this->SetBitIn(this->SMOD,false);
	this->setChar(this->PSW,0x00);
	this->setChar(this->RCAP2H,0x00);
	this->setChar(this->RCAP2L,0x00);
	this->setChar(this->SCON,0x00);
	this->setChar(this->SP,0x07);
	this->setChar(this->TCON,0x00);
	this->setChar(this->T2CON,0x00);
	this->setChar(this->TH0,0x00);
	this->setChar(this->TH1,0x00);
	this->setChar(this->TH2,0x00);
	this->setChar(this->TL0,0x00);
	this->setChar(this->TL1,0x00);
	this->setChar(this->TL2,0x00);
	this->setChar(this->TMOD,0x00);
	
	//reset power state
	this->setChar(this->PCON,this->getCharIn(this->PCON)&(~((1<<this->PD)|(1<<this->IDL))));
	
	//reset pins
	(*this->sendALE)(true);
	(*this->sendnPSEN)(true);
	
	//reset ALU
	this->i_cycle_n=0xFF;
	
	//reset interrupts
	this->interrupt_level=0;
	this->interrupt_change=false;
}
void m80C32::Idle(){
	
}
void m80C32::PowerDown(){
	(*this->sendALE)(false);
	(*this->sendnPSEN)(false);
}
void 80C32::PCONChange(){
	if ((1<<this->PD)!=0){
		this->PowerDown();
	}
	else if ((1<<this->IDL)!=0){
		this->Idle();
	}
}

/*
=============== SERIAL + TIMERS/COUNTERS ===============
*/
void m80C32::T2EXFall(){
	if (this->getBitIn(this->EXEN2)){
		this->SetBitIn(this->EXF2,true);
		
		bool t2rx=this->getBitIn(this->RCLK);
		bool t2tx=this->getBitIn(this->TCLK);
		if ((!t2rx)&&(!t2tx)){
			if (this->getBitIn(this->CP_nRL2)){//capture mode
				this->setChar(this->RCAP2L,this->getCharIn(this->TL2));
				this->setChar(this->RCAP2H,this->getCharIn(this->TH2));
			}
			else{//auto reload mode
				this->setChar(this->TL2,this->getCharIn(this->RCAP2L));
				this->setChar(this->TH2,this->getCharIn(this->RCAP2H));
			}	
		}
	}
}
void m80C32::T2Tick(){
	if (this->getBitIn(this->TR2)){
		unsigned short t2=(unsigned short)this->getCharIn(this->TL2);
		t2|=((unsigned short)this->getCharIn(this->TH2))<<8;
		t2++;
		if (t2==0){
			bool t2rx=this->getBitIn(this->RCLK);
			bool t2tx=this->getBitIn(this->TCLK);
			if (t2rx||t2tx){//baudrate generator mode
				this->setChar(this->TL2,this->getCharIn(this->RCAP2L));
				this->setChar(this->TH2,this->getCharIn(this->RCAP2H));
				this->T2SerialClockTick();
			}
			else{
				this->SetBitIn(this->TF2,true);
				if (!this->getBitIn(this->CP_nRL2)){//auto reload mode
					this->setChar(this->TL2,this->getCharIn(this->RCAP2L));
					this->setChar(this->TH2,this->getCharIn(this->RCAP2H));	
				}
			}
		}
	}
}
void m80C32::nINT0Fall(){
	this->SetBitIn(this->IE0,true);
}
void m80C32::nINT1Fall(){
	this->SetBitIn(this->IE1,true);
}
void m80C32::T0Tick(){
	if (this->getBitIn(this->TR0)&&((!this->getBitIn(this->GATE_0))||this->getBitIn(this->nINT0))){
		bool m0=this.getBitIn(this->M0_0);
		bool m1=this.getBitIn(this->M1_0);
		if (m0){
			if (m1){//mode 3: split -> 8 bit TL0
				unsigned char tl0=this->getCharIn(this->TL0);
				tl0++;
				if (tl0==0) this->SetBitIn(this->TF0,true);
				this->setChar(this->TL0,tl0);
			}
			else{//mode 1: 16 bit
				unsigned short t0=(unsigned short)this->getCharIn(this->TL0);
				t0|=((unsigned short)this->getCharIn(this->TH0))<<8;
				t0++;
				if (t0==0) this->SetBitIn(this->TF0,true);
				this->setChar(this->TH0,(unsigned char)(t0>>8));
				this->setChar(this->TL0,(unsigned char)t0);
			}
		}
		else{
			if (m1){//mode 2: 8 bit auto reload
				unsigned char tl0=this->getCharIn(this->TL0);
				tl0++;
				if (tl0==0){
					tl0=this->getCharIn(this->TH0);
					this->SetBitIn(this->TF0,true);
				}
				this->setChar(this->TL0,tl0);
			}
			else{//mode 0: 13 bit
				unsigned char tl0=this->getCharIn(this->TL0);
				unsigned short t0=(unsigned short)(tl0<<3);
				t0|=((unsigned short)this->getCharIn(this->TH0))<<8;
				t0+=1<<3;
				if (t0<(1<<3)) this->SetBitIn(this->TF0,true);
				this->setChar(this->TH0,(unsigned char)(t0>>8));
				this->setChar(this->TL0,(((unsigned char)(t0>>3))&0x1F)|(tl0&0xE0));
			}
		}
	}
}
void m80C32::T1Tick(){
	bool split=this.getBitIn(this->M0_0)&&this.getBitIn(this->M1_0);
	bool inc=this->getBitIn(this->TR1)&&((!this->getBitIn(this->GATE_1))||this->getBitIn(this->nINT1));
	if (split&&inc){//8 bit TH0
		unsigned char th0=this->getCharIn(this->TH0);
		th0++;
		if (th0==0) this->SetBitIn(this->TF1,true);
		this->setChar(this->TH0,th0);
	}
	if (split||inc){
		bool m0=this.getBitIn(this->M0_1);
		bool m1=this.getBitIn(this->M1_1);
		if (m0){
			if (!m1){//mode 1: 16 bit
				unsigned short t1=(unsigned short)this->getCharIn(this->TL1);
				t1|=((unsigned short)this->getCharIn(this->TH1))<<8;
				t1++;
				if (t1==0){
					if (!split) this->SetBitIn(this->TF1,true);
					this->T1SerialClockTick();
				}
				this->setChar(this->TH1,(unsigned char)(t1>>8));
				this->setChar(this->TL1,(unsigned char)t1);
			}
		}
		else{
			if (m1){//mode 2: 8 bit auto reload
				unsigned char tl1=this->getCharIn(this->TL1);
				tl1++;
				if (tl1==0){
					tl1=this->getCharIn(this->TH1);
					if (!split) this->SetBitIn(this->TF1,true);
					this->T1SerialClockTick();
				}
				this->setChar(this->TL1,tl1);
			}
			else{//mode 0: 13 bit
				unsigned char tl1=this->getCharIn(this->TL1);
				unsigned short t1=(unsigned short)(tl1<<3);
				t1|=((unsigned short)this->getCharIn(this->TH1))<<8;
				t1+=1<<3;
				if (t1<(1<<3)){
					if (!split) this->SetBitIn(this->TF1,true);
					this->T1SerialClockTick();
				}
				this->setChar(this->TH1,(unsigned char)(t1>>8));
				this->setChar(this->TL1,(((unsigned char)(t1>>3))&0x1F)|(tl1&0xE0));
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
	bool smod=(bool)((this->getCharIn(this->PCON)>>this->SMOD)&0x01);
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
		bool smod=(bool)((this->getCharIn(this->PCON)>>this->SMOD)&0x01);
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
		switch (this->RX_bit){//RX_bit: 0bBBBBBBSS B->bit S->sub bit
			case 0:
				bool rx_state=this->getBitIn(this->RxD);
				if (this->RX_state&&(!rx_state))this->RX_bit++;//falling edge
				this->RX_state=rx_state;
				break;
			default:
				this->RX_bit++;
				if (this->RX_bit&0x03!=2) break;
				unsigned char sbuf=this->SBUF_in;
				sbuf=sbuf>>1;
				sbuf|=this->getBitIn(this->RxD)?0x80:0x00;
				this->SBUF_in=sbuf;
				break;
			case 38://stop bit/RB8 (n_bit+1)*4-3: 10*4-2
				bool rb8=this->getBitIn(this->RxD);
				this->setChar(this->SBUF,this->SBUF_in);
				this->SetBitIn(this->RB8,rb8);
				rb8=rb8||(!this->getBitIn(this->SM2));
				this->SetBitIn(this->RI,rb8);
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
				this->setChar(this->SBUF,this->SBUF_in);
				this->SetBitIn(this->RI,true);
				this->RX_bit=0;
			}
		}
		this->RX_state=rx_state;
	}
}
void m80C32::updateTX(){
	if ((!this->TEN)) return;
	if (this->getBitIn(this->SM0)){//mode 2 3
		if (this->TX_bit&0x03!=0) return;
		unsigned char n_bit=11;
		if (this->TX_bit>=n_bit*4) this->TX_bit=0;
		switch (this->TX_bit>>2){
			case 0:
				(*this->sendTxD)(false);
				break;
			case 10:
				(*this->sendTxD)((bool)(this->PX_out[3]&0x02));
				break;
			case 9:
				(*this->sendTxD)(this->getBitIn(this->TB8)&&((bool)(this->PX_out[3]&0x02)));
				break;
			default:
				(*this->sendTxD)(((bool)(this->SBUF_out&0x01))&&((bool)(this->PX_out[3]&0x02)));
				this->SBUF_out=this->SBUF_out>>1;
		}
	else if (this->getBitIn(this->SM1)){//mode 1
		if (this->TX_bit&0x03!=0) return;
		unsigned char n_bit=10;
		if (this->TX_bit>=n_bit*4) this->TX_bit=0;
		switch (this->TX_bit>>2){
			case 0:
				(*this->sendTxD)(false);
				break;
			case 9:
				(*this->sendTxD)((bool)(this->PX_out[3]&0x02));
				break;
			default:
				(*this->sendTxD)(((bool)(this->SBUF_out&0x01))&&((bool)(this->PX_out[3]&0x02)));
				this->SBUF_out=this->SBUF_out>>1;
		}
	}
	else{//mode 0
		unsigned char n_bit=8;
		if (this->TX_bit>=n_bit*4) this->TX_bit=0;
		if (this->TX_bit&0x03==0){
			(*this->sendRxD)(((bool)(this->SBUF_out&0x01))&&((bool)(this->PX_out[3]&0x01)));
			this->SetBitIn(this->RxD,(bool)(this->SBUF_out&0x01));
			this->SBUF_out=this->SBUF_out>>1;
			(*this->sendTxD)((bool)(this->PX_out[3]&0x02));
		}
		else if (this->TX_bit&0x03==2){
			(*this->sendTxD)(false);
		}
	}
	this->TX_bit++;
	if (this->TX_bit<n_bit*4) return;
	this->TX_bit=0;
	this->TEN=false;
	this->SetBitIn(this->TI,true);
	(*this->sendTxD)((bool)(this->PX_out[3]&0x02));
	(*this->sendRxD)((bool)(this->PX_out[3]&0x01));
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
		
		interrupt_signal&=this->getCharIn(this->IE)&0x38;
		unsigned char ip=this->getCharIn(this->IP);
		
		bool int_high=((this->interrupt_level&ip)!=0);
		if (int_high) interrupt_signal&=ip;
		
		for(int i=0;i++;i<8){
			unsigned char mask=1<<i;
			if (this->interrupt_level&mask!=0) break;
			if (interrupt_signal&mask!=0){
				this->i_cycle_n=this->i_cycle[0x12];
				this->instruction[0]=0x12;
				this->instruction[1]=0x00;
				switch (i){
					case 0:
						if (this->getBitIn(this->IT0)){
							this->SetBitIn(this->IE0,false);//clear when falling edge
						}
						this->instruction[2]=0x03;
						break;
					case 1:
						this->SetBitIn(this->TF0,false);
						this->instruction[2]=0x0B;
						break;
					case 2:
						if (this->getBitIn(this->IT1)){
							this->SetBitIn(this->IE1,false);
						}
						this->instruction[2]=0x13;
						break;
					case 3:
						this->SetBitIn(this->TF1,false);
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
				this->setChar(this->PCON,this->getCharIn(this->PCON)&(~(1<<this->IDL)));
				break;
			}
		}
	}
	this->interrupt_change=false;
}
void m80C32::decreaseInterruptLevel(){
	unsigned char il=this->interrupt_level;
	unsigned char ip=this->getCharIn(this->IP);
	if ((il&ip)!=0) il&=ip;
	for(int i=0;i++;i<8){
		if ((bool)((il>>i)&1)){
			this->interrupt_level&=~(1<<i);
			break;
		}
	}
}

/*
=============== ALU ===============
*/
void nextCycleALU(){
	if (this->i_cycle_n>=this->i_cycle[this->instruction[0]]){
		this->i_cycle_n=0;
		this->i_part_n=0;
	}
	unsigned char external_rw_action=0;
	while (external_rw_action<2||this->i_part_n<this->i_length[this->instruction[0]]){
		(*this->sendP0)((unsigned char)(this->PC&0xFF));
		(*this->sendP2)((unsigned char)(this->PC>>8));
		(*this->sendALE)(false);
		(*this->sendnPSEN)(false);
		this->instruction[this->i_part_n]=this->getCharIn(this->P0);
		(*this->sendALE)(true);
		(*this->sendnPSEN)(true);
		(*this->sendP0)(this->PX_out[0]);
		(*this->sendP2)(this->PX_out[2]);
		this->PC++;
		this->i_part_n++;
		external_rw_action++;
	}
	
	if (this->i_cycle[this->instruction[0]]-this->i_cycle_n<=1){
		this->execInstruction();
	}
	
	
	this->i_cycle_n++;
}
void m80C32::setACCParity(){
	unsigned short p=0b0110100110010110;
	unsigned char a=this->getCharIn(this->ACC);
	p=((p>>(a&0x0F))^(p>>(a>>4)))&0x01;
	this->SetBitIn(this->P,p!=0);
}
void m80C32::execInstruction(){
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
			this->ACALL()
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
			unsigned char d1=this->getCharIn(this->ACC);
			unsigned char d2=this->instruction[1];
			goto i_cjne;
		case 0xB5:
			unsigned char d1=this->getCharIn(this->ACC);
			unsigned char d2=this->getCharIn(this->instruction[1]);
			goto i_cjne;
		case 0xB6:
			unsigned char d1=this->getCharIn(this->getCharIn(this->getR(0)));
			unsigned char d2=this->instruction[1];
			goto i_cjne;
		case 0xB7:
			unsigned char d1=this->getCharIn(this->getCharIn(this->getR(1)));
			unsigned char d2=this->instruction[1];
			goto i_cjne;
		case 0xB8:
		case 0xB9:
		case 0xBA:
		case 0xBB:
		case 0xBC:
		case 0xBD:
		case 0xBE:
		case 0xBF://cjne
			unsigned char d1=this->getCharIn(this->getR(this->instruction[0]&0x07));
			unsigned char d2=this->instruction[1];
			i_cjne:
			this->CJNE(d1,d2);
			break;
		
		case 0x24:
			unsigned char d=this->instruction[1];
			goto i_add;
		case 0x25:
			unsigned char d=this->getCharIn(this->instruction[1]);
			goto i_add;
		case 0x26:
			unsigned char d=this->this->getCharIn(this->getCharIn(this->getR(0)));
			goto i_add;
		case 0x27:
			unsigned char d=this->getCharIn(this->getCharIn(this->getR(1)));
			goto i_add;
		case 0x28:
		case 0x29:
		case 0x2A:
		case 0x2B:
		case 0x2C:
		case 0x2D:
		case 0x2E:
		case 0x2F://add
			unsigned char d=this->getCharIn(this->getR(this->instruction[0]&0x07));
			i_add:
			this->ADD(d);
			
		case 0x34:
			unsigned char d=this->instruction[1];
			goto i_addc;
		case 0x35:
			unsigned char d=this->getCharIn(this->instruction[1]);
			goto i_addc;
		case 0x36:
			unsigned char d=this->getCharIn(this->getCharIn(this->getR(0)));
			goto i_addc;
		case 0x37:
			unsigned char d=this->getCharIn(this->getCharIn(this->getR(1)]));
			goto i_addc;
		case 0x38:
		case 0x39:
		case 0x3A:
		case 0x3B:
		case 0x3C:
		case 0x3D:
		case 0x3E:
		case 0x3F://addc
			unsigned char d=this->getCharIn(this->getR(this->instruction[0]&0x07));
			i_addc:
			this->ADDC(d);
			break;
			
		case 0x52:
			unsigned char a=this->instruction[1];
			unsigned char d=this->getCharIn(this->ACC);
			goto i_anl;
		case 0x53:
			unsigned char a=this->instruction[1];
			unsigned char d=this->instruction[2];
			goto i_anl;
		case 0x54:
			unsigned char a=this->ACC;
			unsigned char d=this->instruction[1];
			goto i_anl;
		case 0x55:
			unsigned char a=this->ACC;
			unsigned char d=this->getCharIn(this->instruction[1]);
			goto i_anl;
		case 0x56:
			unsigned char a=this->ACC;
			unsigned char d=this->getCharIn(this->getCharIn(this->getR(0)));
			goto i_anl;
		case 0x57:
			unsigned char a=this->ACC;
			unsigned char d=this->getCharIn(this->getCharIn(this->getR(1)));
			goto i_anl;
		case 0x58:
		case 0x59:
		case 0x5A:
		case 0x5B:
		case 0x5C:
		case 0x5D:
		case 0x5E:
		case 0x5F://anl char
			unsigned char a=this->ACC;
			unsigned char d=this->getCharIn(this->getR(this->instruction[0]&0x07));
			i_anl:
			this->ANL(a,d);
			break;
		
		case 0x82:
			this->ANLcy(this->getBitIn(this->instruction[1]));
			break
		case 0xB0://anl c
			this->ANLcy(!this->getBitIn(this->instruction[1]));
			break
			
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
			unsigned char a=this->ACC;
			goto i_dec;
		case 0x15:
			unsigned char a=this->instruction[1];
			goto i_dec;
		case 0x16:
			unsigned char a=this->getCharIn(this->getR(0));
			goto i_dec;
		case 0x17:
			unsigned char a=this->getCharIn(this->getR(1));
			goto i_dec;
		case 0x18:
		case 0x19:
		case 0x1A:
		case 0x1B:
		case 0x1C:
		case 0x1D:
		case 0x1E:
		case 0x1F://dec
			unsigned char a=this->getR(this->instruction[0]&0x07);
			i_dec:
			this->DEC(a);
			break;
			
		case 0x84://div
			this->DIV();
			break;
		
		case 0xD5:
			unsigned char a=this->instruction[1];
			goto i_djnz;
		case 0xD8:
		case 0xD9:
		case 0xDA:
		case 0xDB:
		case 0xDC:
		case 0xDD:
		case 0xDE:
		case 0xDF://djnz
			unsigned char a=this->getR(this->instruction[0]&0x07);
			i_djnz:
			this->DJNZ(a);
			break;
		
		case 0x04:
			unsigned char a=this->ACC;
			goto i_inc;
		case 0x05:
			unsigned char a=this->instruction[1];
			goto i_inc;
		case 0x06:
			unsigned char a=this->getCharIn(this->getR(0));
			goto i_inc;
		case 0x07:
			unsigned char a=this->getCharIn(this->getR(1));
			goto i_inc;
		case 0x08:
		case 0x09:
		case 0x0A:
		case 0x0B:
		case 0x0C:
		case 0x0D:
		case 0x0E:
		case 0x0F://inc
			unsigned char a=this->getR(this->instruction[0]&0x07);
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
			unsigned char a=this->ACC;
			unsigned char d=this->instruction[1];
			goto i_mov;
		case 0x75:
			unsigned char a=this->instruction[1];
			unsigned char d=this->instruction[2];
			goto i_mov;
		case 0x76:
			unsigned char a=this->getCharIn(this->getR(0));
			unsigned char d=this->instruction[1];
			goto i_mov;
		case 0x77:
			unsigned char a=this->getCharIn(this->getR(1));
			unsigned char d=this->instruction[1];
			goto i_mov;
		case 0x78:
		case 0x79:
		case 0x7A:
		case 0x7B:
		case 0x7C:
		case 0x7D:
		case 0x7E:
		case 0x7F:
			unsigned char a=this->getR(this->instruction[0]&0x07);
			unsigned char d=this->instruction[1];
			goto i_mov;
		case 0x85:
			unsigned char a=this->instruction[2];//!!!!!!!!!!!!!!!!
			unsigned char d=this->getCharIn(this->instruction[1]);
			goto i_mov;
		case 0x86:
			unsigned char a=this->instruction[1];
			unsigned char d=this->getCharIn(this->getCharIn(this->getR(0)));
			goto i_mov;
		case 0x87:
			unsigned char a=this->instruction[1];
			unsigned char d=this->getCharIn(this->getCharIn(this->getR(1)));
			goto i_mov;
		case 0x88:
		case 0x89:
		case 0x8A:
		case 0x8B:
		case 0x8C:
		case 0x8D:
		case 0x8E:
		case 0x8F:
			unsigned char a=this->instruction[1];
			unsigned char d=this->getCharIn(this->getR(this->instruction[0]&0x07));
			goto i_mov;
		case 0xA6:
			unsigned char a=this->getCharIn(this->getR(0));
			unsigned char d=this->getCharIn(this->instruction[1]);
			goto i_mov;
		case 0xA7:
			unsigned char a=this->getCharIn(this->getR(1));
			unsigned char d=this->getCharIn(this->instruction[1]);
			goto i_mov;
		case 0xA8:
		case 0xA9:
		case 0xAA:
		case 0xAB:
		case 0xAC:
		case 0xAD:
		case 0xAE:
		case 0xAF:
			unsigned char a=this->getR(this->instruction[0]&0x07);
			unsigned char d=this->getCharIn(this->instruction[1]);
			goto i_mov;
		case 0xE5:
			unsigned char a=this->ACC;
			unsigned char d=this->getCharIn(this->instruction[1]);
			goto i_mov;
		case 0xE6:
			unsigned char a=this->ACC;
			unsigned char d=this->getCharIn(this->getCharIn(this->getR(0)));
			goto i_mov;
		case 0xE7:
			unsigned char a=this->ACC;
			unsigned char d=this->getCharIn(this->getCharIn(this->getR(1)));
			goto i_mov;
		case 0xE8:
		case 0xE9:
		case 0xEA:
		case 0xEB:
		case 0xEC:
		case 0xED:
		case 0xEE:
		case 0xEF:
			unsigned char a=this->ACC;
			unsigned char d=this->getCharIn(this->getR(this->instruction[0]&0x07));
			goto i_mov;
		case 0xF5:
			unsigned char a=this->instruction[1];
			unsigned char d=this->getCharIn(this->ACC);
			goto i_mov;
		case 0xF6:
			unsigned char a=this->getCharIn(this->getR(0));
			unsigned char d=this->getCharIn(this->ACC);
			goto i_mov;
		case 0xF7:
			unsigned char a=this->getCharIn(this->getR(1));
			unsigned char d=this->getCharIn(this->ACC);
			goto i_mov;
		case 0xF8:
		case 0xF9:
		case 0xFA:
		case 0xFB:
		case 0xFC:
		case 0xFD:
		case 0xFE:
		case 0xFF://mov
			unsigned char a=this->getR(this->instruction[0]&0x07);
			unsigned char d=this->getCharIn(this->ACC);
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
			unsigned short ptr=(((unsigned short)this->getCharIn(this->DPH))<<8)|((unsigned short)this->getCharIn(this->DPL));
			this->MOVC(ptr);
			break;
			
		case 0xE0:
			unsigned short ptr=(((unsigned short)this->getCharIn(this->DPH))<<8)|((unsigned short)this->getCharIn(this->DPL));
			this->MOVXin(ptr);
			break;
		case 0xE2:
			unsigned char ptr=this->getCharIn(this->getR(0));
			this->MOVXin(ptr);
			break;
		case 0xE3:
			unsigned char ptr=this->getCharIn(this->getR(1));
			this->MOVXin(ptr);
			break;
		case 0xF0:
			unsigned short ptr=(((unsigned short)this->getCharIn(this->DPH))<<8)|((unsigned short)this->getCharIn(this->DPL));
			this->MOVXout(ptr);
			break;
		case 0xF2:
			unsigned char ptr=this->getCharIn(this->getR(0));
			this->MOVXout(ptr);
			break;
		case 0xF3://movx
			unsigned char ptr=this->getCharIn(this->getR(1));
			this->MOVXout(ptr);
			break;
			
		case 0xA4:
			this->MUL();
			break;
			
		case 0x42:
			unsigned char a=this->instruction[1];
			unsigned char d=this->getCharIn(this->ACC);
			goto i_orl;
		case 0x43:
			unsigned char a=this->instruction[1];
			unsigned char d=this->instruction[2];
			goto i_orl;
		case 0x44:
			unsigned char a=this->ACC;
			unsigned char d=this->instruction[1];
			goto i_orl;
		case 0x45:
			unsigned char a=this->ACC;
			unsigned char d=this->getCharIn(this->instruction[1]);
			goto i_orl;
		case 0x46:
			unsigned char a=this->ACC;
			unsigned char d=this->getCharIn(this->getCharIn(this->getR(0)));
			goto i_orl;
		case 0x47:
			unsigned char a=this->ACC;
			unsigned char d=this->getCharIn(this->getCharIn(this->getR(1)));
			goto i_orl;
		case 0x48:
		case 0x49:
		case 0x4A:
		case 0x4B:
		case 0x4C:
		case 0x4D:
		case 0x4E:
		case 0x4F://orl
			unsigned char a=this->ACC;
			unsigned char d=this->getCharIn(this->getR(this->instruction[0]&0x07));
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
			unsigned char d=this->instruction[1];
			goto i_subb;
		case 0x95:
			unsigned char d=this->getCharIn(this->instruction[1]);
			goto i_subb;
		case 0x96:
			unsigned char d=this->getCharIn(this->getCharIn(this->getR(0)));
			goto i_subb;
		case 0x97:
			unsigned char d=this->getCharIn(this->getCharIn(this->getR(1)));
			goto i_subb;
		case 0x98:
		case 0x99:
		case 0x9A:
		case 0x9B:
		case 0x9C:
		case 0x9D:
		case 0x9E:
		case 0x9F://subb
			unsigned char d=this->getCharIn(this->getR(this->instruction[0]&0x07));
			i_subb:
			this->SUBB(d);
			break;
			
		case 0xC4://swap
			this->SWAP();
			break;
			
		case 0xC5:
			unsigned char a=this->instruction[1];
			goto i_xch;
		case 0xC6:
			unsigned char a=this->getCharIn(this->getR(0));
			goto i_xch;
		case 0xC7:
			unsigned char a=this->getCharIn(this->getR(1));
			goto i_xch;
		case 0xC8:
		case 0xC9:
		case 0xCA:
		case 0xCB:
		case 0xCC:
		case 0xCD:
		case 0xCE:
		case 0xCF://xch
			unsigned char a=this->getR(this->instruction[0]&0x07);
			i_xch:
			this->XCH(a);
			break;
			
		case 0xD6:
			this->XCHD(this->getCharIn(this->getR(0)));
			break;
		case 0xD7://xchd
			this->XCHD(this->getCharIn(this->getR(1)));
			break;
			
		case 0x62:
			unsigned char a=this->instruction[1];
			unsigned char d=this->getCharIn(this->ACC);
			goto i_xrl;
		case 0x63:
			unsigned char a=this->instruction[1];
			unsigned char d=this->instruction[2];
			goto i_xrl;
		case 0x64:
			unsigned char a=this->ACC;
			unsigned char d=this->instruction[1];
			goto i_xrl;
		case 0x65:
			unsigned char a=this->ACC;
			unsigned char d=this->getCharIn(this->instruction[1]);
			goto i_xrl;
		case 0x66:
			unsigned char a=this->ACC;
			unsigned char d=this->getCharIn(this->getCharIn(this->getR(0)));
			goto i_xrl;
		case 0x67:
			unsigned char a=this->ACC;
			unsigned char d=this->getCharIn(this->getCharIn(this->getR(1)));
			goto i_xrl;
		case 0x68:
		case 0x69:
		case 0x6A:
		case 0x6B:
		case 0x6C:
		case 0x6D:
		case 0x6E:
		case 0x6F://xrl
			unsigned char a=this->ACC;
			unsigned char d=this->getCharIn(this->getR(this->instruction[0]&0x07));
			i_xrl:
			this->XRL(a,d);
			break;
	}
}
void m80C32::ACALL(){
	sp=this->getCharIn(this->SP)+1;
	this->setChar(sp,(unsigned char)(this->PC&0xFF));
	sp++;
	this->setChar(sp,(unsigned char)(this->PC>>8));
	this->setChar(this->SP,sp);
	this->AJMP();
}
void m80C32::AJMP(){
	this->PC=(this->PC&0xF800)|(((unsigned short)(this->instruction[0]>>5))<<8)|((unsigned short)this->instruction[1]);
}
void m80C32::ADD(unsigned char d){
	unsigned char a=this->getCharIn(this->ACC);
	this->SetBitIn(this->CY,(a>UCHAR_MAX-d));
	this->SetBitIn(this->AC,(a&0x0F>NIBBLE_MAX-(d&0x0F)));
	if (((signed char)d)>0){
		this->SetBitIn(this->OV,(((signed char)a)>SCHAR_MAX-((signed char)d)));
	}
	else{
		this->SetBitIn(this->OV,(((signed char)a)<SCHAR_MIN-((signed char)d)));
	}
	this->setChar(this->ACC,a+d);
}
void m80C32::ADDC(unsigned char d){
	if this->getBitIn(this->CY) d++;
	this->ADD(d);
}
void m80C32::ANL(unsigned char a,unsigned char d){
	this->setChar(a,this->getCharOut(a)&d);
}
void m80C32::ANLcy(bool b){
	if (!b) this->SetBitIn(this->CY,false);
}
void m80C32::CJNE(unsigned char d1,unsigned char d2){
	this->SetBitIn(this->CY,d1<d2);
	if (d1!=d2){
		this->PC+=(unsigned short)(signed char)this->instruction[2];
	}
}
void m80C32::CLRa(){
	this->setChar(this->ACC,0);
}
void m80C32::CLRb(unsigned char a){
	unsigned char bit;
	this->bitaddress2address(&a,&bit);
	unsigned char v=this->getCharOut(a);//!!!
	v&=~(1<<bit);
	this->setChar(a,v);
}
void m80C32::CPLa(){
	this->setChar(this->ACC,this->getCharIn(this->ACC)^0xFF);
}
void m80C32::CPLb(unsigned char a){
	unsigned char bit;
	this->bitaddress2address(&a,&bit);
	unsigned char d=this->getCharOut(a);
	this->setChar(a,d^(1<<bit));
}
void m80C32::DA(){
	unsigned char a=this->getCharIn(this->ACC);
	if (this->getBitIn(this->AC)||a&0x0F>9){
		this->SetBitIn(this->CY,(a>UCHAR_MAX-0x06)||this->getBitIn(this->CY));
		a+=0x06;
	}
	if (this->getBitIn(this->CY)||a>0x9F){
		this->SetBitIn(this->CY,(a>UCHAR_MAX-0x60)||this->getBitIn(this->CY));
		a+=0x60;
	}
	this->s=setChar(this->ACC,a);
}
void m80C32::DEC(unsigned char a){
	unsigned char v=this->getCharOut(a)-1;
	this->setChar(a,v);
}
void m80C32::DIV(){
	unsigned char a=this->getCharIn(this->ACC);
	unsigned char b=this->getCharIn(this->B);
	this->SetBitIn(this->CY,false);
	if (b==0x00){
		this->SetBitIn(this->OV,true);
	}
	else{
		this->SetBitIn(this->OV,false);
		this->setChar(this->ACC,a/b);
		this->setChar(this->B,a%b);
	}
}
void m80C32::DJNZ(unsigned char a){
	unsigned char v=this->getCharOut(a)-1;
	if (v!=0){
		this->PC+=(unsigned short)(signed char)this->instruction[1];
	}
	this->setChar(a,v);
}
void m80C32::INC(unsigned char a){
	this->setChar(a,this->getCharOut(a)+1);
}
void m80C32::INCdptr(){
	unsigned short dptr=((unsigned short)this->getCharIn(this->DPL))|(((unsigned short)this->getCharIn(this->DPH))<<8);
	dptr++;
	this->setChar(this->DPL,(unsigned char)(dptr&0xFF));
	this->setChar(this->DPH,(unsigned char)(dptr>>8));
}
void m80C32::JB(){
	if (getBitIn(this->instruction[1]))this->PC+=(unsigned short)(signed char)this->instruction[2];
}
void m80C32::JBC(){
	unsigned char bit;
	unsigned char address=this->instruction[1];
	this->bitaddress2address(&address,&bit);
	unsigned char d=this->getCharOut(address);
	unsigned char mask=1<<bit;
	if (d&mask!=0){
		this->setChar(address,d&(~mask));
		this->PC+=(unsigned short)(signed char)this->instruction[2];
	}
}
void m80C32::JC(){
	if (getBitIn(this->CY))this->PC+=(unsigned short)(signed char)this->instruction[1];
}
void m80C32::JMP(){
	unsigned short dptr=((unsigned short)this->getCharIn(this->DPL))|(((unsigned short)this->getCharIn(this->DPH))<<8);
	dptr+=(unsigned short)this->getCharIn(this->ACC);
	this->PC=dptr;
}
void m80C32::JNB(){
	if (!getBitIn(this->instruction[1]))this->PC+=(unsigned short)(signed char)this->instruction[2];
}
void m80C32::JNC(){
	if (!getBitIn(this->CY))this->PC+=(unsigned short)(signed char)this->instruction[1];
}
void m80C32::JNZ(){
	if (getCharIn(this->ACC)!=0)this->PC+=(unsigned short)(signed char)this->instruction[1];
}
void m80C32::JZ(){
	if (getCharIn(this->ACC)==0)this->PC+=(unsigned short)(signed char)this->instruction[1];
}
void m80C32::LCALL(){
	sp=this->getCharIn(this->SP)+1;
	this->setChar(sp,(unsigned char)(this->PC&0xFF));
	sp++;
	this->setChar(sp,(unsigned char)(this->PC>>8));
	this->setChar(this->SP,sp);
	this->LJMP();
}
void m80C32::LJMP(){
	this->PC=(((unsigned short)this->instruction[1])<<8)|((unsigned short)this->instruction[2]);
}
void m80C32::MOV(unsigned char a,unsigned char d){
	this->setChar(a,d);
}
void m80C32::MOVdptr(){
	this->setChar(this->DPH,this->instruction[1]);
	this->setChar(this->DPL,this->instruction[2]);
}
void m80C32::MOVb(unsigned char ba1,unsigned char ba2){
	unsigned char bit;
	this->bitaddress2address(&ba1,&bit);
	unsigned char d=this->getCharOut(ba1);
	unsigned char mask=1<<bit;
	d&=~mask;
	d|=this->getBitIn(ba2)?mask:0x00;
	this->setChar(ba1,d);
}
void m80C32::MOVC(unsigned short ptr){
	ptr+=this.getCharIn(this->ACC);
	(*this->sendP0)((unsigned char)(ptr&0xFF));
	(*this->sendP2)((unsigned char)(ptr>>8));
	(*this->sendALE)(false);
	(*this->sendnPSEN)(false);
	this->setChar(this->ACC,this->getCharIn(this->P0));
	(*this->sendALE)(true);
	(*this->sendnPSEN)(true);
	(*this->sendP0)(this->PX_out[0]);
	(*this->sendP2)(this->PX_out[2]);
}
void m80C32::MOVXin(unsigned char a){//8 bit address
	(*this->sendP0)(a);
	(*this->sendALE)(false);
	(*this->sendnRD)(false);
	this->setChar(this->ACC,this->getCharIn(this->P0));
	(*this->sendALE)(true);
	(*this->sendnRD)((bool)(this->PX_out[3]&0x80));
	(*this->sendP0)(this->PX_out[0]);
}
void m80C32::MOVXin(unsigned short a){//16 bit address
	(*this->sendP0)((unsigned char)(a&0xFF));
	(*this->sendP2)((unsigned char)(a>>8));
	(*this->sendALE)(false);
	(*this->sendnRD)(false);
	this->setChar(this->ACC,this->getCharIn(this->P0));
	(*this->sendALE)(true);
	(*this->sendnRD)((bool)(this->PX_out[3]&0x80));
	(*this->sendP0)(this->PX_out[0]);
	(*this->sendP2)(this->PX_out[2]);
}
void m80C32::MOVXout(unsigned char a){//8 bit address
	(*this->sendP0)(a);
	(*this->sendALE)(false);
	(*this->sendP0)(this->getCharIn(this->ACC));
	(*this->sendnWR)(false);
	(*this->sendALE)(true);
	(*this->sendnWR)((bool)(this->PX_out[3]&0x40));
	(*this->sendP0)(this->PX_out[0]);
}
void m80C32::MOVXout(unsigned short a){//16 bit address
	(*this->sendP0)((unsigned char)(a&0xFF));
	(*this->sendP2)((unsigned char)(a>>8));
	(*this->sendALE)(false);
	(*this->sendP0)(this->getCharIn(this->ACC));
	(*this->sendnWR)(false);
	(*this->sendALE)(true);
	(*this->sendnWR)((bool)(this->PX_out[3]&0x40));
	(*this->sendP0)(this->PX_out[0]);
	(*this->sendP2)(this->PX_out[2]);
}
void m80C32::MUL(){
	unsigned short ab=((unsigned short)this->getCharIn(this->ACC))*((unsigned short)this->getCharIn(this->B));
	this.setChar(this->ACC,(unsigned char)(ab&0xFF));
	this.setChar(this->B,(unsigned char)(ab>>8));
	this.SetBitIn(this->CY,false);
	this.SetBitIn(this->OV,ab>0xFF);
}
void m80C32::ORL(unsigned char a,unsigned char d){
	this->setChar(a,this->getCharOut(a)|d);
}
void m80C32::ORLcy(bool b){
	if (b) this->SetBitIn(this->CY,true);
}
void m80C32::POP(){
	unsigned char sp=this->getCharIn(this->SP);
	this->setChar(this->instruction[1],this->getCharIn(sp));
	sp--;
	this->setChar(this->SP,sp);
}
void m80C32::PUSH(){
	unsigned char sp=this->getCharIn(this->SP);
	sp++;
	this->setChar(sp,this->getCharIn(this->instruction[1]));
	this->setChar(this->SP,sp);
}
void m80C32::RET(){
	unsigned char sp=this->getCharIn(this->SP);
	unsigned short ptr=(unsigned short)this->getCharIn(sp);
	sp--;
	ptr=(ptr<<8)|(unsigned short)this->getCharIn(sp);
	sp--;
	this->setChar(this->SP,sp);
	this->PC=ptr;
}
void m80C32::RETI(){
	//////////////////////////////////////////////////////////////////////////////////////
	this->decreaseInterruptLevel();
	this->RET();
}
void m80C32::RL(){
	unsigned char a=this->getCharIn(this->ACC);
	a=(a<<1)|(a>>7);
	this->setChar(this->ACC,a);
}
void m80C32::RLC(){
	unsigned char a=this->getCharIn(this->ACC);
	bool b=this->getBitIn(this->CY);
	this->setChar(this->CY,(bool)(a>>7));
	a=(a<<1)|(b?0x01:0x00);
	this->setChar(this->ACC,a);
}
void m80C32::RR(){
	unsigned char a=this->getCharIn(this->ACC);
	a=(a>>1)|(a<<7);
	this->setChar(this->ACC,a);
}
void m80C32::RRC(){
	unsigned char a=this->getCharIn(this->ACC);
	bool b=this->getBitIn(this->CY);
	this->setChar(this->CY,(bool)(a&0x01));
	a=(a>>1)|(b?0x80:0x00);
	this->setChar(this->ACC,a);
}
void m80C32::SETB(unsigned char ba){
	unsigned char bit;
	this->bitaddress2address(&ba,&bit);
	unsigned char d=this->getCharOut(ba);
	d|=1<<bit;
	this->setChar(ba,d);
}
void m80C32::SJMP(){
	this->PC+=(unsigned short)(signed char)this->instruction[1];
}
void m80C32::SUBB(unsigned char d){
	if this->getBitIn(this->CY) d++;
	unsigned char a=this->getBitIn(this->ACC);
	this->SetBitIn(this->CY,(a>UCHAR_MAX+d));
	this->SetBitIn(this->AC,(a&0x0F>NIBBLE_MAX+(d&0x0F)));
	if (((signed char)d)<0){
		this->SetBitIn(this->OV,(((signed char)a)>SCHAR_MAX+((signed char)d)));
	}
	else{
		this->SetBitIn(this->OV,(((signed char)a)<SCHAR_MIN+((signed char)d)));
	}
	this->setChar(this->ACC,a-d);
}
void m80C32::SWAP(){
	unsigned char a=this->getCharIn(this->ACC);
	a=(a>>4)|(a<<4);
	this->setChar(this->ACC,a);
}
void m80C32::XCH(unsigned char a){
	unsigned char d=this->getCharIn(this->ACC);
	this->setChar(this->ACC,this->getCharIn(a));//getCharIn???
	this->setChar(a,d);
}
void m80C32::XCHD(unsigned char a){
	unsigned char d1=this->getCharIn(this->ACC);
	unsigned char d2=this->getCharIn(a);
	this->setChar(this->ACC,(d1&0xF0)|(d2&0x0F));//getCharIn???
	this->setChar(a,(d2&0xF0)|(d1&0x0F));
}
void m80C32::XRL(unsigned char a,unsigned char d){
	this->setChar(a,d^this->getCharOut(a));
}