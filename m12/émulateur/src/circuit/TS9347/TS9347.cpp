#include "circuit/TS9347.h"

#define GLAD_GL_IMPLEMENTATION
#include "glad/glad.h"
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

void decomposePointer(unsigned char Reven,unsigned char Rodd,AddressDecomposition* address){
	address->Y=Reven&0b00011111;
	address->District=Reven>>5;
	address->X=Rodd&0b00111111;
	const unsigned char b_invert[4]={0,2,1,3};
	address->Block=b_invert[Rodd>>6];
}
void recomposePointer(unsigned char* p_Reven,unsigned char* p_Rodd,AddressDecomposition* address){
	const unsigned char b_invert[4]={0,2,1,3};
	*p_Reven=(address->Y&0b00011111)|(address->District<<5);
	*p_Rodd=(address->X&0b00111111)|(b_invert[address->Block&0x03]<<6);
}
int address2PAddress(AddressDecomposition* address){
	int A=((int)address->District)<<12;
	A|=((int)address->Block)<<10;
	A|=((int)address->X&0x07);
	if(address->Y>=8){
		if ((bool)(address->X&(1<<5))){
			A|=((int)address->Y&0x18);
			A|=((int)address->Y&0x07)<<5;
		}
		else{
			A|=((int)address->X&0x18);
			A|=((int)address->Y)<<5;
		}
	}
	else{
		if ((bool)(address->Y&1)) return -1;
		A|=((int)address->X&0x38)<<2;
	}
	return A;
}


void TS9347wVRAM::subscribeD(std::function<void(unsigned char)> f){
	this->sendD=f;
}
void TS9347wVRAM::DChangeIn(unsigned char d){
	this->D=d;
}
void TS9347wVRAM::ASChangeIn(bool b){
	if (this->AS&&!b){
		this->ADDR_BUF.store(this->D|(this->nCS?0x100:0),std::memory_order_relaxed);
	}
	this->AS=b;
}
void TS9347wVRAM::nCSChangeIn(bool b){
	this->nCS=b;
}
void TS9347wVRAM::DSChangeIn(bool b){
	if (this->DS&&!b){
		this->DS=b;
		//read reg
		if (this->isSelected()){
			unsigned int addr=this->ADDR_BUF.load(std::memory_order_relaxed);
			if ((addr&0x07)==0) this->sendD(this->STATUS.load(std::memory_order_relaxed));
			else if(!this->isBusy()) this->sendD(this->Rx[addr&0x07].load(std::memory_order_relaxed));
			else printf("TS9347 read garbage!!!\n");
		}
	}
	else{
		this->DS=b;
	}
}
void TS9347wVRAM::RnWChangeIn(bool b){
	if ((!this->RnW)&&b){
		this->RnW=b;
		//write reg
		unsigned int addr=this->ADDR_BUF.load(std::memory_order_relaxed);
		if (this->isSelected()){
			if ((addr&0x07)==0){
				if (((!this->isBusy())||this->requestExecution())&&this->isCMDCorrect(this->D)){
					this->Rx[0].store(this->D,std::memory_order_relaxed);
					this->STATUS.fetch_or(this->BUSY_MASK,std::memory_order_relaxed);
					this->executeCommand();
					glfwPostEmptyEvent();
				}
			}
			else if (!this->isBusy()) this->Rx[addr&0x07].store(this->D,std::memory_order_relaxed);
			else printf("TS9347 write inefective!!!\n");
		}
	}
	else{
		this->RnW=b;
	}
}

bool TS9347wVRAM::isSelected(){
	return (this->ADDR_BUF.load(std::memory_order_relaxed)&0x1F0)==0x020;
}
bool TS9347wVRAM::isBusy(){
	return (bool)(this->STATUS.load(std::memory_order_relaxed)&this->BUSY_MASK);
}
bool TS9347wVRAM::requestExecution(){
	return (this->ADDR_BUF.load(std::memory_order_relaxed)&0x008)==0x008;
}
bool TS9347wVRAM::isCMDCorrect(unsigned char cmd){
	bool b=false;
	switch(cmd&0xF0){
		case 0x00:
		case 0x20:
		case 0x30:
		case 0x40:
		case 0x50:
		case 0x60:
		case 0x70:
		case 0x80:
		case 0x90:
		case 0xB0:
		case 0xD0:
		case 0xE0:
		case 0xF0:
			b=true;
			break;
		default:
			printf("not CMD %02X\n",cmd);
			//not cmd
			break;
	}
	return b;
}

void TS9347wVRAM::executeCommand(){
	unsigned char x=this->Rx[0].load(std::memory_order_relaxed);
	//printf("CMD %02X\n",x);
	switch(x&0xF0){
		case 0b10000000://IND
			this->IND(x&0x07,(bool)(x&this->RnW_MASK));
			this->STATUS.fetch_and(~this->BUSY_MASK,std::memory_order_relaxed);
			break;
		case 0b00000000://TLM/CLL/TSM/CLS
			switch (x&0x06){
				case 0://TLM
					this->TLSMA(true,(bool)(x&0x08),(bool)(x&0x01),true);
					break;
				case 2://TLM/TSM
					if ((bool)(x&8)){//TLM
						this->TLSMA(true,true,(bool)(x&0x01),true);
					}
					else{//TSM
						this->TLSMA(true,false,(bool)(x&0x01),false);
					}
					break;
				case 4://CLL
				{
					bool incy=false;
					do{/////////////////////////////////////////////////////////////////////////////////////////////////
						this->TLSMA(true,false,true,true);
						incy=(bool)(this->STATUS.load(std::memory_order_relaxed)&this->Al_MASK);
						if (incy) this->INY();
						//printf("%i loop\n",this->Rx[6].load(std::memory_order_relaxed)&0x1F);
					}while(!((this->Rx[6].load(std::memory_order_relaxed)&0x1F)==8&&incy));
					break;
				}
				case 6://CLS
				{
					bool incy=false;
					do{
						this->TLSMA(true,false,true,false);
						incy=(bool)(this->STATUS.load(std::memory_order_relaxed)&this->Al_MASK);
						if (incy) this->INY();
						//printf("%i loop\n",this->Rx[6].load(std::memory_order_relaxed)&0x1F);
					}while(!((this->Rx[6].load(std::memory_order_relaxed)&0x1F)==8&&incy));
					break;
				}
			}
			this->STATUS.fetch_and(~this->BUSY_MASK,std::memory_order_relaxed);
			break;
		case 0b00100000://TLA
			//printf("TLA\n");
			this->TLSMA(false,(bool)(x&0x08),(bool)(x&0x01),true);
			this->STATUS.fetch_and(~this->BUSY_MASK,std::memory_order_relaxed);
			break;
		case 0b01100000://TSM/CLS
			if ((bool)(x&0x04)){
				//printf("CLS\n");
				bool incy=false;
				do{
					this->TLSMA(true,false,true,false);
					incy=(bool)(this->STATUS.load(std::memory_order_relaxed)&this->Al_MASK);
					if (incy) this->INY();
				}while(!((this->Rx[6].load(std::memory_order_relaxed)&0x1F)==8&&incy));
			}
			else{
				//printf("TSM\n");
				this->TLSMA(true,(bool)(x&0x08),(bool)(x&0x01),false);
			}
			this->STATUS.fetch_and(~this->BUSY_MASK,std::memory_order_relaxed);
			break;
		case 0b01110000://TSA
			//printf("TSA\n");
			this->TLSMA(false,(bool)(x&0x08),(bool)(x&0x01),false);
			this->STATUS.fetch_and(~this->BUSY_MASK,std::memory_order_relaxed);
			break;
		case 0b01000000://KRS
			//printf("KRS\n");
			this->KRLS((bool)(x&0x08),(bool)(x&0x01),false);
			this->STATUS.fetch_and(~this->BUSY_MASK,std::memory_order_relaxed);
			break;
		case 0b01010000://KRL
			//printf("KRL\n");
			this->KRLS((bool)(x&0x08),(bool)(x&0x01),true);
			this->STATUS.fetch_and(~this->BUSY_MASK,std::memory_order_relaxed);
			break;
		case 0b00110000://TBM/TBA
			//printf("TBM/TBA\n");
			this->TBMA(!(bool)(x&0x04),(bool)(x&0x08),(bool)(x&0x01));
			this->STATUS.fetch_and(~this->BUSY_MASK,std::memory_order_relaxed);
			break;
		case 0b11010000://MVB
			//printf("MVB\n");
			this->MV(1,(bool)(x&0x04),(bool)(x&0x01));
			if ((bool)(x&0x01)) this->STATUS.fetch_and(~this->BUSY_MASK,std::memory_order_relaxed);
			break;
		case 0b11100000://MVD
			//printf("MVD\n");
			this->MV(2,(bool)(x&0x04),(bool)(x&0x01));
			if ((bool)(x&0x01)) this->STATUS.fetch_and(~this->BUSY_MASK,std::memory_order_relaxed);
			break;
		case 0b11110000://MVT
			//printf("MVT\n");
			this->MV(3,(bool)(x&0x04),(bool)(x&0x01));
			if ((bool)(x&0x01)) this->STATUS.fetch_and(~this->BUSY_MASK,std::memory_order_relaxed);
			break;
		case 0b10010000://VSM/VRM/NOP
		{
			bool vsm=((x&0x0F)==0b00001001);
			bool vrm=((x&0x0F)==0b00000101);
			if (vsm||vrm){
				if (vsm) this->VSM();
				else this->VRM();
			}
			this->STATUS.fetch_and(~this->BUSY_MASK,std::memory_order_relaxed);
			break;
		}
		case 0b10110000://INY
			this->INY();
			this->STATUS.fetch_and(~this->BUSY_MASK,std::memory_order_relaxed);
			break;
		default:
			this->STATUS.fetch_and(~this->BUSY_MASK,std::memory_order_relaxed);
			//not cmd
			break;
	}
}

void TS9347wVRAM::IND(unsigned char r,bool RnW){
	if (r==0){
		this->Rx[1].store(0,std::memory_order_relaxed);///////////////////////////////////////////////
	}
	else{
		std::atomic_uchar* p_reg[8]={NULL,&(this->TGS),&(this->MAT),&(this->PAT),&(this->DOR),NULL,NULL,&(this->ROR)};
		if (RnW) this->Rx[1].store(p_reg[r]->load(std::memory_order_relaxed),std::memory_order_relaxed);
		else p_reg[r]->store(this->Rx[1].load(std::memory_order_relaxed),std::memory_order_relaxed);
	}
	unsigned char m=this->Al_MASK|this->LXm_MASK|this->LXa_MASK;
	this->STATUS.fetch_and(~m,std::memory_order_relaxed);
}

void TS9347wVRAM::VSM(){
	unsigned char m=this->Al_MASK|this->LXm_MASK|this->LXa_MASK|this->VSYNC_MASK;
	this->STATUS.fetch_and(~m,std::memory_order_relaxed);
}

void TS9347wVRAM::VRM(){
	unsigned char m=this->Al_MASK|this->LXm_MASK|this->LXa_MASK|this->VSYNC_MASK;
	this->STATUS.fetch_and(~m,std::memory_order_relaxed);
}

void TS9347wVRAM::INY(){
	unsigned char R6=this->Rx[6].load(std::memory_order_relaxed);
	if ((R6&0x1F)==0x1F){
		this->Rx[6].store((R6&0xE0)|0x08,std::memory_order_relaxed);
	}
	else{
		this->Rx[6].store(R6+1,std::memory_order_relaxed);
	}
	unsigned char m=this->Al_MASK|this->LXm_MASK|this->LXa_MASK;
	this->STATUS.fetch_and(~m,std::memory_order_relaxed);
}

void TS9347wVRAM::TBMA(bool MnA,bool RnW,bool inc){
	unsigned char P0=Rx[MnA?6:4].load(std::memory_order_relaxed);
	unsigned char P1=Rx[MnA?7:5].load(std::memory_order_relaxed);
	AddressDecomposition AD;
	decomposePointer(P0,P1,&AD);
	int PA=address2PAddress(&AD);
	if (PA>=0){
		if (RnW) this->Rx[1].store(this->VRAM[PA].load(std::memory_order_relaxed),std::memory_order_relaxed);
		else this->VRAM[PA].store(this->Rx[1].load(std::memory_order_relaxed),std::memory_order_relaxed);
	}
	unsigned char m=this->Al_MASK|this->LXm_MASK|this->LXa_MASK;
	this->STATUS.fetch_and(~m,std::memory_order_relaxed);
	if ((P1&0x3F)==39) this->STATUS.fetch_or(MnA?this->LXm_MASK:this->LXa_MASK,std::memory_order_relaxed);
	if (inc){
		if ((P1&0x3F)==39){
			Rx[MnA?7:5].store(P1&0xC0,std::memory_order_relaxed);
			this->STATUS.fetch_or(this->Al_MASK,std::memory_order_relaxed);
			if (MnA){
				if ((P0&0x1F)==0x1F) Rx[MnA?6:4].store((P0&0xE0)|0x08,std::memory_order_relaxed);
				else Rx[MnA?6:4].store(P0+1,std::memory_order_relaxed);
			}
		}
		else{
			Rx[MnA?7:5].store((P1&0xC0)|((P1&0x3F)+1),std::memory_order_relaxed);
		}
	}
}

void TS9347wVRAM::KRLS(bool RnW,bool inc,bool long_code){
	unsigned char P0=Rx[6].load(std::memory_order_relaxed);
	unsigned char P1=Rx[7].load(std::memory_order_relaxed);
	/*bool odd_p=(bool)(P1&1);
	AddressDecomposition AD;
	decomposePointer(P0,(P1>>1)|(odd_p?0x80:0),&AD);
	int PA_C=address2PAddress(&AD);
	
	unsigned char A_MASK=(odd_p?0x0F:0xF0);*/
	AddressDecomposition AD;
	decomposePointer(P0,P1,&AD);
	int PA_C=address2PAddress(&AD);
	unsigned char A_MASK=(((bool)(P1&0x80))?0x0F:0xF0);
	//
	AD.Block=((AD.Block&0x02)+2)%4;
	int PA_A=address2PAddress(&AD);
	
	if (PA_C>=0){
		if (RnW){
			this->Rx[1].store(this->VRAM[PA_C].load(std::memory_order_relaxed),std::memory_order_relaxed);
			if(long_code){
				unsigned char A=this->VRAM[PA_A].load(std::memory_order_relaxed)&A_MASK;
				A|=(A<<4)|(A>>4);
				//A|=this->Rx[3].load(std::memory_order_relaxed)&(~A_MASK);
				this->Rx[3].store(A,std::memory_order_relaxed);
			}
		}
		else{
			this->VRAM[PA_C].store(this->Rx[1].load(std::memory_order_relaxed),std::memory_order_relaxed);
			if(long_code){
				unsigned char A=this->VRAM[PA_A].load(std::memory_order_relaxed)&(~A_MASK);
				A|=this->Rx[3].load(std::memory_order_relaxed)&A_MASK;
				this->VRAM[PA_A].store(A,std::memory_order_relaxed);
			}
		}
	}
	
	unsigned char m=this->Al_MASK|this->LXm_MASK|this->LXa_MASK;
	this->STATUS.fetch_and(~m,std::memory_order_relaxed);
	/*if ((P1&0x7F)==79) this->STATUS.fetch_or(this->LXm_MASK,std::memory_order_relaxed);
	if (inc){
		if ((P1&0x7F)==79){
			Rx[7].store(P1&0x80,std::memory_order_relaxed);
			this->STATUS.fetch_or(this->Al_MASK,std::memory_order_relaxed);
		}
		else{
			Rx[7].store((P1&0x80)|((P1&0x7F)+1),std::memory_order_relaxed);
		}
	}*/
	if ((P1&0x3F)==39){
		this->STATUS.fetch_or(this->LXm_MASK,std::memory_order_relaxed);
		if (inc) this->STATUS.fetch_or(this->Al_MASK,std::memory_order_relaxed);
	}
	if (inc){
		if ((bool)(P1&0x80)){
			P1=P1^0x80;
			if ((P1&0x3F)==39){
				Rx[7].store(P1&0xC0,std::memory_order_relaxed);
			}
			else{
				Rx[7].store(P1+1,std::memory_order_relaxed);
			}
		}
		else{
			Rx[7].store(P1^0x80,std::memory_order_relaxed);
		}
	}
	//
}

void TS9347wVRAM::TLSMA(bool MnA,bool RnW,bool inc, bool long_code){
	unsigned char P0=Rx[MnA?6:4].load(std::memory_order_relaxed);
	unsigned char P1=Rx[MnA?7:5].load(std::memory_order_relaxed);
	AddressDecomposition AD;
	decomposePointer(P0,P1&0x7F,&AD);
	//AD.Block&=0x02;
	int PA_C=address2PAddress(&AD);
	AD.Block=(AD.Block+1)%4;
	int PA_B=address2PAddress(&AD);
	
	AD.Block=(AD.Block+1)%4;
	int PA_A=address2PAddress(&AD);
	
	if (PA_C>=0){
		if (RnW){
			this->Rx[1].store(this->VRAM[PA_C].load(std::memory_order_relaxed),std::memory_order_relaxed);
			this->Rx[2].store(this->VRAM[PA_B].load(std::memory_order_relaxed),std::memory_order_relaxed);
			if(long_code) this->Rx[3].store(this->VRAM[PA_A].load(std::memory_order_relaxed),std::memory_order_relaxed);
		}
		else{
			this->VRAM[PA_C].store(this->Rx[1].load(std::memory_order_relaxed),std::memory_order_relaxed);
			this->VRAM[PA_B].store(this->Rx[2].load(std::memory_order_relaxed),std::memory_order_relaxed);
			if(long_code) this->VRAM[PA_A].store(this->Rx[2].load(std::memory_order_relaxed),std::memory_order_relaxed);
		}
	}
	
	unsigned char m=this->Al_MASK|this->LXm_MASK|this->LXa_MASK;
	this->STATUS.fetch_and(~m,std::memory_order_relaxed);
	if ((P1&0x3F)==39) this->STATUS.fetch_or(MnA?this->LXm_MASK:this->LXa_MASK,std::memory_order_relaxed);
	if (inc){
		if ((P1&0x3F)==39){
			Rx[MnA?7:5].store(P1&0xC0,std::memory_order_relaxed);
			this->STATUS.fetch_or(this->Al_MASK,std::memory_order_relaxed);
		}
		else{
			Rx[MnA?7:5].store((P1&0xC0)|((P1&0x3F)+1),std::memory_order_relaxed);
		}
	}
}
void TS9347wVRAM::MV(unsigned char n_buf,bool M2A,bool stop){
	
	unsigned char P0_M=Rx[6].load(std::memory_order_relaxed);
	unsigned char P1_M=Rx[7].load(std::memory_order_relaxed);
	unsigned char P0_A=Rx[4].load(std::memory_order_relaxed);
	unsigned char P1_A=Rx[5].load(std::memory_order_relaxed);
	AddressDecomposition AD_M;
	decomposePointer(P0_M,P1_M&0x7F,&AD_M);
	//AD_M.Block&=0x02;
	AddressDecomposition AD_A;
	decomposePointer(P0_A,P1_A&0x7F,&AD_A);
	//AD_A.Block&=0x02;
	AddressDecomposition AD_from=M2A?AD_M:AD_A;
	AddressDecomposition AD_to=M2A?AD_A:AD_M;
	
	int fB=AD_from.Block;
	int tB=AD_to.Block;
	if (stop){
		do{
			int Pf=address2PAddress(&AD_from);
			int Pt=address2PAddress(&AD_to);
			if (Pf>=0&&Pt>=0){
				this->VRAM[Pt].store(this->VRAM[Pf].load(std::memory_order_relaxed),std::memory_order_relaxed);
				if (n_buf>=2){
					AD_from.Block=(AD_from.Block+1)%4;
					AD_to.Block=(AD_to.Block+1)%4;
					this->VRAM[address2PAddress(&AD_to)].store(this->VRAM[address2PAddress(&AD_from)].load(std::memory_order_relaxed),std::memory_order_relaxed);
					if (n_buf==3){
						AD_from.Block=(AD_from.Block+1)%4;
						AD_to.Block=(AD_to.Block+1)%4;
						this->VRAM[address2PAddress(&AD_to)].store(this->VRAM[address2PAddress(&AD_from)].load(std::memory_order_relaxed),std::memory_order_relaxed);
					}
					AD_from.Block=fB;
					AD_to.Block=tB;
				}
			}
			if(AD_from.X==39) AD_from.X=0;
			else AD_from.X++;
			if(AD_to.X==39) AD_to.X=0;
			else AD_to.X++;
		}while (AD_from.X!=0&&AD_to.X!=0);
	}
	else{
		for(int i=0;i<2*40*25;i++){/////////////////////////////////////////////////////////////////////////////////////////
			int Pf=address2PAddress(&AD_from);
			int Pt=address2PAddress(&AD_to);
			if (Pf>=0&&Pt>=0){
				this->VRAM[Pt].store(this->VRAM[Pf].load(std::memory_order_relaxed),std::memory_order_relaxed);
				if (n_buf>=2){
					AD_from.Block=(AD_from.Block+1)%4;
					AD_to.Block=(AD_to.Block+1)%4;
					this->VRAM[address2PAddress(&AD_to)].store(this->VRAM[address2PAddress(&AD_from)].load(std::memory_order_relaxed),std::memory_order_relaxed);
					if (n_buf==3){
						AD_from.Block=(AD_from.Block+1)%4;
						AD_to.Block=(AD_to.Block+1)%4;
						this->VRAM[address2PAddress(&AD_to)].store(this->VRAM[address2PAddress(&AD_from)].load(std::memory_order_relaxed),std::memory_order_relaxed);
					}
					AD_from.Block=fB;
					AD_to.Block=tB;
				}
			}
			if(AD_from.X==39){
				AD_from.X=0;
				AD_from.Y++;
				if (AD_from.Y==32) AD_from.Y=8;
			}
			else AD_from.X++;
			if(AD_to.X==39){
				AD_to.X=0;
				AD_to.Y++;
				if (AD_to.Y==32) AD_to.Y=8;
			}
			else AD_to.X++;
		}
	}
	recomposePointer(M2A?&P0_M:&P0_A,M2A?&P1_M:&P1_A,&AD_from);
	recomposePointer(M2A?&P0_A:&P0_M,M2A?&P1_A:&P1_M,&AD_to);
	Rx[6].store(P0_M,std::memory_order_relaxed);
	Rx[7].store(P1_M,std::memory_order_relaxed);
	Rx[4].store(P0_A,std::memory_order_relaxed);
	Rx[5].store(P1_A,std::memory_order_relaxed);
	unsigned char m=this->Al_MASK|this->LXm_MASK|this->LXa_MASK;
	this->STATUS.fetch_and(~m,std::memory_order_relaxed);
}