#ifndef DBG_80C32_H
#define DBG_80C32_H

#include "circuit/80C32.h"

class stackMonitor{
	public:
		const unsigned char UNDEFINED_DATA=0;
		
		const unsigned char ADDRESS_LOW=1;
		const unsigned char ADDRESS_HIGH=2;
		const unsigned char MASK_DATA_USAGE=0x0F;
		
		const unsigned char RETURN_ADDRESS=1<<4;
		const unsigned char USER_DATA_WRITE=2<<4;
		const unsigned char USER_DATA_PUSH=3<<4;
		const unsigned char MASK_DATA_ORIGIN=0x70;
		
		const unsigned char OLD_DATA=0x80;
		
		std::function<void(void)> SPManualyModified=[](){};
		std::function<void(void)> stackPOPed=[](){};
		std::function<void(void)> stackPUSHed=[](){};
		std::function<void(bool)> addressPOPed=[](bool high){};
		std::function<void(void)> funcCalled=[](){};
		std::function<void(bool,bool,bool)> funcReturned=[](bool ud,bool ind,bool old){};
		std::function<void(void)> stackOverwrited=[](){};
		stackMonitor(m80C32* uc){
			this->uc=uc;
			for (int i=0;i<256;i++){
				this->mem[i]=this->UNDEFINED_DATA;
			}
		}
		void updateState(){
			unsigned char aw;
			bool wm=this->isManualyWritingToRam(&aw);
			bool pop=this->isPOPing();
			if (pop){
				unsigned char a=this->uc->getSFRByteIn(this->uc->SP);
				unsigned char s=this->mem[a];
				this->mem[a]|=this->OLD_DATA;
				this->stackPOPed();
				if ((s&this->MASK_DATA_ORIGIN)==this->RETURN_ADDRESS&&(s&this->OLD_DATA)==0) this->addressPOPed((s&this->MASK_DATA_USAGE)==this->ADDRESS_HIGH);
			}
			if (wm){
				unsigned char s=this->mem[aw];
				this->mem[aw]=this->USER_DATA_WRITE;
				if (((s&this->MASK_DATA_ORIGIN)==this->USER_DATA_PUSH||(s&this->MASK_DATA_ORIGIN)==this->RETURN_ADDRESS)&&(s&this->OLD_DATA)==0) this->stackOverwrited();
			}
			bool push=this->isPUSHing();
			if (push){
				this->mem[this->uc->getSFRByteIn(this->uc->SP)+1]=this->USER_DATA_PUSH;
				this->stackPUSHed();
			}
			
			if (this->isCALLing()){
				unsigned char a=this->uc->getSFRByteIn(this->uc->SP);
				this->mem[a+1]=this->ADDRESS_LOW|this->RETURN_ADDRESS;
				this->mem[a+2]=this->ADDRESS_HIGH|this->RETURN_ADDRESS;
				this->funcCalled();
				return;
			}
			if (this->isRETing()){
				unsigned char a=this->uc->getSFRByteIn(this->uc->SP);
				unsigned char al=this->mem[a-1];
				unsigned char ah=this->mem[a];
				this->mem[a-1]&=~this->MASK_DATA_USAGE;
				this->mem[a-1]|=this->ADDRESS_LOW;
				this->mem[a-1]|=this->OLD_DATA;
				this->mem[a]&=~this->MASK_DATA_USAGE;
				this->mem[a]|=this->ADDRESS_HIGH;
				this->mem[a]|=this->OLD_DATA;
				
				bool ud=(al&this->MASK_DATA_ORIGIN)!=(ah&this->MASK_DATA_ORIGIN);
				ud=ud||((al&this->MASK_DATA_USAGE)==this->ADDRESS_HIGH||(ah&this->MASK_DATA_USAGE)==this->ADDRESS_LOW);
				bool old=(al&this->OLD_DATA)==this->OLD_DATA;
				bool ind=(al&this->MASK_DATA_ORIGIN)!=this->RETURN_ADDRESS;
				
				this->funcReturned(ud,ind,old);
				return;
			}
			if(this->isWritingToSP()){
				this->SPManualyModified();
				return;
			}
		}
	private:
		m80C32* uc;
		unsigned char mem[256];
		
		bool isWritingToSP(){
			unsigned char a=0x00;
			switch (this->uc->instruction[0]){
				case 0x05:
				case 0x15:
				case 0x42:
				case 0x43:
				case 0x52:
				case 0x53:
				case 0x62:
				case 0x63:
				case 0x75:
				case 0x86:
				case 0x87:
				case 0x88:
				case 0x89:
				case 0x8A:
				case 0x8B:
				case 0x8C:
				case 0x8D:
				case 0x8E:
				case 0x8F:
				case 0xC5:
				case 0xD0:
				case 0xD5:
				case 0xF5:a=this->uc->instruction[1];break;
				case 0x85:a=this->uc->instruction[2];break;
			}
			return (a==this->uc->SP);
		}
		bool isPUSHing(){
			return this->uc->instruction[0]==0xC0;
		}
		bool isPOPing(){
			return this->uc->instruction[0]==0xD0;
		}
		bool isRETing(){
			return this->uc->instruction[0]==0x22||this->uc->instruction[0]==0x32;
		}
		bool isCALLing(){
			bool b=false;
			switch(this->uc->instruction[0]){
				case 0x11:
				case 0x12:
				case 0x31:
				case 0x51:
				case 0x71:
				case 0x91:
				case 0xB1:
				case 0xD1:
				case 0xF1:b=true;break;
			}
			return b;
		}
		bool isManualyWritingToRam(unsigned char* p_a){
			bool b=false;
			switch (this->uc->instruction[0]){
				case 0x05://direct 1
				case 0x15:
				case 0x42:
				case 0x43:
				case 0x52:
				case 0x53:
				case 0x62:
				case 0x63:
				case 0x75:
				case 0x86:
				case 0x87:
				case 0x88:
				case 0x89:
				case 0x8A:
				case 0x8B:
				case 0x8C:
				case 0x8D:
				case 0x8E:
				case 0x8F:
				case 0xC5:
				case 0xD0:
				case 0xD5:
				case 0xF5:
					b=!(bool)(this->uc->instruction[1]&0x80);
					if (b) *p_a=this->uc->instruction[1];
					break;
				
				case 0x85://direct 2
					b=!(bool)(this->uc->instruction[2]&0x80);
					if (b) *p_a=this->uc->instruction[2];
					break;
				
				case 0x06://@Ri
				case 0x07:
				case 0x16:
				case 0x17:
				case 0x76:
				case 0x77:
				case 0xA6:
				case 0xA7:
				case 0xC6:
				case 0xC7:
				case 0xD6:
				case 0xD7:
				case 0xF6:
				case 0xF7:
					b=true;
					*p_a=this->uc->getRAMByte(this->uc->getR(this->uc->instruction[0]&1));
					break;
				
				case 0x08://Rn
				case 0x09:
				case 0x0A:
				case 0x0B:
				case 0x0C:
				case 0x0D:
				case 0x0E:
				case 0x0F:
				case 0x18:
				case 0x19:
				case 0x1A:
				case 0x1B:
				case 0x1C:
				case 0x1D:
				case 0x1E:
				case 0x1F:
				case 0x78:
				case 0x79:
				case 0x7A:
				case 0x7B:
				case 0x7C:
				case 0x7D:
				case 0x7E:
				case 0x7F:
				case 0xA8:
				case 0xA9:
				case 0xAA:
				case 0xAB:
				case 0xAC:
				case 0xAD:
				case 0xAE:
				case 0xAF:
				case 0xC8:
				case 0xC9:
				case 0xCA:
				case 0xCB:
				case 0xCC:
				case 0xCD:
				case 0xCE:
				case 0xCF:
				case 0xD8:
				case 0xD9:
				case 0xDA:
				case 0xDB:
				case 0xDC:
				case 0xDD:
				case 0xDE:
				case 0xDF:
				case 0xF8:
				case 0xF9:
				case 0xFA:
				case 0xFB:
				case 0xFC:
				case 0xFD:
				case 0xFE:
				case 0xFF:
					b=true;
					*p_a=this->uc->getR(this->uc->instruction[0]&7);
					break;
				
				case 0x10://bit
				case 0x92:
				case 0xB2:
				case 0xC2:
				case 0xD2:
					b=!(bool)(this->uc->instruction[1]&0x80);
					if (b) *p_a=this->uc->getBitDirectAddress(this->uc->instruction[1]);
			}
			return b;
		}
};




const char* getDirectAddressName(unsigned char a){
	switch (a){
		default:return NULL;
		case 0xE0:return "ACC";
		case 0xF0:return "B";
		case 0x83:return "DPH";
		case 0x82:return "DPL";
		case 0xA8:return "IE";
		case 0xB8:return "IP";
		case 0x80:return "P0";
		case 0x90:return "P1";
		case 0xA0:return "P2";
		case 0xB0:return "P3";
		case 0x87:return "PCON";
		case 0xD0:return "PSW";
		case 0xCB:return "RCAP2H";
		case 0xCA:return "RCAP2L";
		case 0x99:return "SBUF";
		case 0x98:return "SCON";
		case 0x81:return "SP";
		case 0x88:return "TCON";
		case 0xC8:return "T2CON";
		case 0x8C:return "TH0";
		case 0x8D:return "TH1";
		case 0xCD:return "TH2";
		case 0x8A:return "TL0";
		case 0x8B:return "TL1";
		case 0xCC:return "TL2";
		case 0x89:return "TMOD";
		case 0x00:return "B0_R0";
		case 0x01:return "B0_R1";
		case 0x02:return "B0_R2";
		case 0x03:return "B0_R3";
		case 0x04:return "B0_R4";
		case 0x05:return "B0_R5";
		case 0x06:return "B0_R6";
		case 0x07:return "B0_R7";
		case 0x08:return "B1_R0";
		case 0x09:return "B1_R1";
		case 0x0A:return "B1_R2";
		case 0x0B:return "B1_R3";
		case 0x0C:return "B1_R4";
		case 0x0D:return "B1_R5";
		case 0x0E:return "B1_R6";
		case 0x0F:return "B1_R7";
		case 0x10:return "B2_R0";
		case 0x11:return "B2_R1";
		case 0x12:return "B2_R2";
		case 0x13:return "B2_R3";
		case 0x14:return "B2_R4";
		case 0x15:return "B2_R5";
		case 0x16:return "B2_R6";
		case 0x17:return "B2_R7";
		case 0x18:return "B3_R0";
		case 0x19:return "B3_R1";
		case 0x1A:return "B3_R2";
		case 0x1B:return "B3_R3";
		case 0x1C:return "B3_R4";
		case 0x1D:return "B3_R5";
		case 0x1E:return "B3_R6";
		case 0x1F:return "B3_R7";
	}
}
const char* getBitAddressName(unsigned char a){
	switch (a){
		default:return NULL;
		case 0xAF:return "EA";
		case 0xAD:return "ET2";
		case 0xAC:return "ES";
		case 0xAB:return "ET1";
		case 0xAA:return "EX1";
		case 0xA9:return "ET0";
		case 0xA8:return "EX0";
		case 0xBD:return "PT2";
		case 0xBC:return "PS";
		case 0xBB:return "PT1";
		case 0xBA:return "PX1";
		case 0xB9:return "PT0";
		case 0xB8:return "PX0";
		case 0x87:return "AD7";
		case 0x86:return "AD6";
		case 0x85:return "AD5";
		case 0x84:return "AD4";
		case 0x83:return "AD3";
		case 0x82:return "AD2";
		case 0x81:return "AD1";
		case 0x80:return "AD0";
		case 0x91:return "T2EX";
		case 0x90:return "T2";
		case 0xA7:return "A15";
		case 0xA6:return "A14";
		case 0xA5:return "A13";
		case 0xA4:return "A12";
		case 0xA3:return "A11";
		case 0xA2:return "A10";
		case 0xA1:return "A9";
		case 0xA0:return "A8";
		case 0xB7:return "nRD";
		case 0xB6:return "nWR";
		case 0xB5:return "T1";
		case 0xB4:return "T0";
		case 0xB3:return "nINT1";
		case 0xB2:return "nINT0";
		case 0xB1:return "TxD";
		case 0xB0:return "RxD";
		case 0xD7:return "CY";
		case 0xD6:return "AC";
		case 0xD5:return "F0";
		case 0xD4:return "RS1";
		case 0xD3:return "RS0";
		case 0xD2:return "OV";
		case 0xD0:return "P";
		case 0x9F:return "SM0";
		case 0x9E:return "SM1";
		case 0x9D:return "SM2";
		case 0x9C:return "REN";
		case 0x9B:return "TB8";
		case 0x9A:return "RB8";
		case 0x99:return "TI";
		case 0x98:return "RI";
		case 0x8F:return "TF1";
		case 0x8E:return "TR1";
		case 0x8D:return "TF0";
		case 0x8C:return "TR0";
		case 0x8B:return "IE1";
		case 0x8A:return "IT1";
		case 0x89:return "IE0";
		case 0x88:return "IT0";
		case 0xCF:return "TF2";
		case 0xCE:return "EXF2";
		case 0xCD:return "RCLK";
		case 0xCC:return "TCLK";
		case 0xCB:return "EXEN2";
		case 0xCA:return "TR2";
		case 0xC9:return "C/nT2";
		case 0xC8:return "CP/nRL2";
	}
}

void print_m12_alu_instruction(m80C32* uc){
	char l=uc->i_length[uc->instruction[0]];
	printf("c:%05lX ",((unsigned long)uc->PC)-l+(((unsigned long)uc->PX_out[1]&3)<<16));
	for(int i=0;i<3;i++){
		if (i<l) printf("%02X",uc->instruction[i]);
		else printf("  ");
	}
	printf(" ");
	switch(uc->instruction[0]){
		case 0x00:printf("NOP  ");break;
		case 0x01:
		case 0x21:
		case 0x41:
		case 0x61:
		case 0x81:
		case 0xA1:
		case 0xC1:
		case 0xE1:printf("AJMP ");break;
		case 0x02:printf("LJMP ");break;
		case 0x03:printf("RR   ");break;
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07:
		case 0x08:
		case 0x09:
		case 0x0A:
		case 0x0B:
		case 0x0C:
		case 0x0D:
		case 0x0E:
		case 0x0F:
		case 0xA3:printf("INC  ");break;
		case 0x10:printf("JBC  ");break;
		case 0x11:
		case 0x31:
		case 0x51:
		case 0x71:
		case 0x91:
		case 0xB1:
		case 0xD1:
		case 0xF1:printf("ACALL");break;
		case 0x12:printf("LCALL");break;
		case 0x13:printf("RRC  ");break;
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
		case 0x18:
		case 0x19:
		case 0x1A:
		case 0x1B:
		case 0x1C:
		case 0x1D:
		case 0x1E:
		case 0x1F:printf("DEC  ");break;
		case 0x20:printf("JB   ");break;
		case 0x22:printf("RET  ");break;
		case 0x23:printf("RL   ");break;
		case 0x24:
		case 0x25:
		case 0x26:
		case 0x27:
		case 0x28:
		case 0x29:
		case 0x2A:
		case 0x2B:
		case 0x2C:
		case 0x2D:
		case 0x2E:
		case 0x2F:printf("ADD  ");break;
		case 0x30:printf("JNB  ");break;
		case 0x32:printf("RETI ");break;
		case 0x33:printf("RLC  ");break;
		case 0x34:
		case 0x35:
		case 0x36:
		case 0x37:
		case 0x38:
		case 0x39:
		case 0x3A:
		case 0x3B:
		case 0x3C:
		case 0x3D:
		case 0x3E:
		case 0x3F:printf("ADDC ");break;
		case 0x40:printf("JC   ");break;
		case 0x42:
		case 0x43:
		case 0x44:
		case 0x45:
		case 0x46:
		case 0x47:
		case 0x48:
		case 0x49:
		case 0x4A:
		case 0x4B:
		case 0x4C:
		case 0x4D:
		case 0x4E:
		case 0x4F:
		case 0xA0:printf("ORL  ");break;
		case 0x50:printf("JNC  ");break;
		case 0x52:
		case 0x53:
		case 0x54:
		case 0x55:
		case 0x56:
		case 0x57:
		case 0x58:
		case 0x59:
		case 0x5A:
		case 0x5B:
		case 0x5C:
		case 0x5D:
		case 0x5E:
		case 0x5F:
		case 0x82:
		case 0xB0:printf("ANL  ");break;
		case 0x60:printf("JZ   ");break;
		case 0x62:
		case 0x63:
		case 0x64:
		case 0x65:
		case 0x66:
		case 0x67:
		case 0x68:
		case 0x69:
		case 0x6A:
		case 0x6B:
		case 0x6C:
		case 0x6D:
		case 0x6E:
		case 0x6F:printf("XRL  ");break;
		case 0x70:printf("JNZ  ");break;
		case 0x72:printf("ORL  ");break;
		case 0x73:printf("JMP  ");break;
		case 0x74:
		case 0x75:
		case 0x76:
		case 0x77:
		case 0x78:
		case 0x79:
		case 0x7A:
		case 0x7B:
		case 0x7C:
		case 0x7D:
		case 0x7E:
		case 0x7F:
		case 0x85:
		case 0x86:
		case 0x87:
		case 0x88:
		case 0x89:
		case 0x8A:
		case 0x8B:
		case 0x8C:
		case 0x8D:
		case 0x8E:
		case 0x8F:
		case 0x90:
		case 0x92:
		case 0xA2:
		case 0xA6:
		case 0xA7:
		case 0xA8:
		case 0xA9:
		case 0xAA:
		case 0xAB:
		case 0xAC:
		case 0xAD:
		case 0xAE:
		case 0xAF:
		case 0xE5:
		case 0xE6:
		case 0xE7:
		case 0xE8:
		case 0xE9:
		case 0xEA:
		case 0xEB:
		case 0xEC:
		case 0xED:
		case 0xEE:
		case 0xEF:
		case 0xF5:
		case 0xF6:
		case 0xF7:
		case 0xF8:
		case 0xF9:
		case 0xFA:
		case 0xFB:
		case 0xFC:
		case 0xFD:
		case 0xFE:
		case 0xFF:printf("MOV  ");break;
		case 0x80:printf("SJMP ");break;
		case 0x83:
		case 0x93:printf("MOVC ");break;
		case 0x84:printf("DIV  ");break;
		case 0x94:
		case 0x95:
		case 0x96:
		case 0x97:
		case 0x98:
		case 0x99:
		case 0x9A:
		case 0x9B:
		case 0x9C:
		case 0x9D:
		case 0x9E:
		case 0x9F:printf("SUBB ");break;
		case 0xA4:printf("MUL  ");break;
		case 0xA5:printf("Reserved");break;
		case 0xB2:
		case 0xB3:printf("CPL  ");break;
		case 0xB4:
		case 0xB5:
		case 0xB6:
		case 0xB7:
		case 0xB8:
		case 0xB9:
		case 0xBA:
		case 0xBB:
		case 0xBC:
		case 0xBD:
		case 0xBE:
		case 0xBF:printf("CJNE ");break;
		case 0xC0:printf("PUSH ");break;
		case 0xC2:
		case 0xC3:
		case 0xE4:printf("CLR  ");break;
		case 0xC4:printf("SWAP ");break;
		case 0xC5:
		case 0xC6:
		case 0xC7:
		case 0xC8:
		case 0xC9:
		case 0xCA:
		case 0xCB:
		case 0xCC:
		case 0xCD:
		case 0xCE:
		case 0xCF:printf("XCH  ");break;
		case 0xD0:printf("POP  ");break;
		case 0xD2:
		case 0xD3:printf("SETB ");break;
		case 0xD4:printf("DA   ");break;
		case 0xD5:
		case 0xD8:
		case 0xD9:
		case 0xDA:
		case 0xDB:
		case 0xDC:
		case 0xDD:
		case 0xDE:
		case 0xDF:printf("DJNZ ");break;
		case 0xD6:
		case 0xD7:printf("XCHD ");break;
		case 0xE0:
		case 0xE2:
		case 0xE3:
		case 0xF0:
		case 0xF2:
		case 0xF3:printf("MOVX ");break;
		case 0xF4:printf("CPL  ");break;
	}
	printf(" ");
	switch(uc->instruction[0]){
		case 0x00:
		case 0x22:
		case 0x32:
		case 0xA5:break;
		//addr11
		case 0x01:
		case 0x11:
		case 0x21:
		case 0x31:
		case 0x41:
		case 0x51:
		case 0x61:
		case 0x71:
		case 0x81:
		case 0x91:
		case 0xA1:
		case 0xB1:
		case 0xC1:
		case 0xD1:
		case 0xE1:
		case 0xF1:printf("c:%05lX",(((unsigned long)uc->PX_out[1]&3)<<16)|((unsigned long)(uc->PC)&0xF800)|((((unsigned long)uc->instruction[0])<<3)&0x0700)|((unsigned long)uc->instruction[1]));break;
		//A
		case 0x03:
		case 0x04:
		case 0x13:
		case 0x14:
		case 0x23:
		case 0x24:
		case 0x25:
		case 0x26:
		case 0x27:
		case 0x28:
		case 0x29:
		case 0x2A:
		case 0x2B:
		case 0x2C:
		case 0x2D:
		case 0x2E:
		case 0x2F:
		case 0x33:
		case 0x34:
		case 0x35:
		case 0x36:
		case 0x37:
		case 0x38:
		case 0x39:
		case 0x3A:
		case 0x3B:
		case 0x3C:
		case 0x3D:
		case 0x3E:
		case 0x3F:
		case 0x44:
		case 0x45:
		case 0x46:
		case 0x47:
		case 0x48:
		case 0x49:
		case 0x4A:
		case 0x4B:
		case 0x4C:
		case 0x4D:
		case 0x4E:
		case 0x4F:
		case 0x54:
		case 0x55:
		case 0x56:
		case 0x57:
		case 0x58:
		case 0x59:
		case 0x5A:
		case 0x5B:
		case 0x5C:
		case 0x5D:
		case 0x5E:
		case 0x5F:
		case 0x64:
		case 0x65:
		case 0x66:
		case 0x67:
		case 0x68:
		case 0x69:
		case 0x6A:
		case 0x6B:
		case 0x6C:
		case 0x6D:
		case 0x6E:
		case 0x6F:
		case 0x74:
		case 0x83:
		case 0x93:
		case 0x94:
		case 0x95:
		case 0x96:
		case 0x97:
		case 0x98:
		case 0x99:
		case 0x9A:
		case 0x9B:
		case 0x9C:
		case 0x9D:
		case 0x9E:
		case 0x9F:
		case 0xB4:
		case 0xB5:
		case 0xC4:
		case 0xC5:
		case 0xC6:
		case 0xC7:
		case 0xC8:
		case 0xC9:
		case 0xCA:
		case 0xCB:
		case 0xCC:
		case 0xCD:
		case 0xCE:
		case 0xCF:
		case 0xD4:
		case 0xD6:
		case 0xD7:
		case 0xE0:
		case 0xE2:
		case 0xE3:
		case 0xE4:
		case 0xE5:
		case 0xE6:
		case 0xE7:
		case 0xE8:
		case 0xE9:
		case 0xEA:
		case 0xEB:
		case 0xEC:
		case 0xED:
		case 0xEE:
		case 0xEF:
		case 0xF4:printf("A");break;
		//AB
		case 0x84:
		case 0xA4:printf("AB");break;
		//addr16
		case 0x02:
		case 0x12:printf("c:%05lX",(((unsigned long)uc->PX_out[1]&3)<<16)|(((unsigned long)uc->instruction[1])<<8)|((unsigned long)uc->instruction[2]));break;
		//rel
		case 0x40:
		case 0x50:
		case 0x60:
		case 0x70:
		case 0x80:printf("c:%05lX",(((unsigned long)uc->PX_out[1]&3)<<16)|((unsigned long)(uc->PC+((signed char)uc->instruction[1]))));break;
		//CY
		case 0x72:
		case 0x82:
		case 0xA0:
		case 0xA2:
		case 0xB0:
		case 0xB3:
		case 0xC3:
		case 0xD3:printf("CY");break;
		//@A+DPTR
		case 0x73:printf("@A+DPTR=>c:%05lX",(((unsigned long)uc->PX_out[1]&3)<<16)|
											((unsigned long)(((((unsigned short)uc->getSFRByteIn(uc->DPH))<<8)|((unsigned short)uc->getSFRByteIn(uc->DPL)))
											+uc->getSFRByteIn(uc->ACC))));break;
		//@DPTR
		case 0xF0:printf("@DPTR=>e:%04X",(((unsigned short)uc->getSFRByteIn(uc->DPH))<<8)|((unsigned short)uc->getSFRByteIn(uc->DPL)));break;
		//DPTR
		case 0x90:
		case 0xA3:printf("DPTR");break;
		//@Ri
		case 0x06:
		case 0x07:
		case 0x16:
		case 0x17:
		case 0x76:
		case 0x77:
		case 0xA6:
		case 0xA7:
		case 0xB6:
		case 0xB7:
		case 0xF6:
		case 0xF7:
			printf("@R%i=>",uc->instruction[0]&1);
			printf("@");
			printf(getDirectAddressName(uc->getR(uc->instruction[0]&1)));
			printf("=>i:%02X",uc->getRAMByte(uc->getR(uc->instruction[0]&1)));
			break;
		case 0xF2:
		case 0xF3:
			printf("@R%i=>",uc->instruction[0]&1);
			printf("@");
			printf(getDirectAddressName(uc->getR(uc->instruction[0]&1)));
			printf("=>e:%04X",(((unsigned short)uc->PX_out[2])<<8)|((unsigned short)uc->getRAMByte(uc->getR(uc->instruction[0]&1))));
			break;
		//Rn
		case 0x08:
		case 0x09:
		case 0x0A:
		case 0x0B:
		case 0x0C:
		case 0x0D:
		case 0x0E:
		case 0x0F:
		case 0x18:
		case 0x19:
		case 0x1A:
		case 0x1B:
		case 0x1C:
		case 0x1D:
		case 0x1E:
		case 0x1F:
		case 0x78:
		case 0x79:
		case 0x7A:
		case 0x7B:
		case 0x7C:
		case 0x7D:
		case 0x7E:
		case 0x7F:
		case 0xA8:
		case 0xA9:
		case 0xAA:
		case 0xAB:
		case 0xAC:
		case 0xAD:
		case 0xAE:
		case 0xAF:
		case 0xB8:
		case 0xB9:
		case 0xBA:
		case 0xBB:
		case 0xBC:
		case 0xBD:
		case 0xBE:
		case 0xBF:
		case 0xD8:
		case 0xD9:
		case 0xDA:
		case 0xDB:
		case 0xDC:
		case 0xDD:
		case 0xDE:
		case 0xDF:
		case 0xF8:
		case 0xF9:
		case 0xFA:
		case 0xFB:
		case 0xFC:
		case 0xFD:
		case 0xFE:
		case 0xFF:
			printf("R%i=>",uc->instruction[0]&7);
			printf(getDirectAddressName(uc->getR(uc->instruction[0]&7)));break;
		//direct
		case 0x05:
		case 0x15:
		case 0x42:
		case 0x43:
		case 0x52:
		case 0x53:
		case 0x62:
		case 0x63:
		case 0x75:
		case 0x86:
		case 0x87:
		case 0x88:
		case 0x89:
		case 0x8A:
		case 0x8B:
		case 0x8C:
		case 0x8D:
		case 0x8E:
		case 0x8F:
		case 0xC0:
		case 0xD0:
		case 0xD5:
		case 0xF5:
		{
			const char* an=getDirectAddressName(uc->instruction[1]);
			if (an==NULL){
				if ((bool)(uc->instruction[1]&0x80)) printf("s:%02X",uc->instruction[1]);
				else printf("i:%02X",uc->instruction[1]);
			}
			else printf(an);
			break;
		}
		case 0x85://///////////
		{
			const char* an=getDirectAddressName(uc->instruction[2]);
			if (an==NULL){
				if ((bool)(uc->instruction[2]&0x80)) printf("s:%02X",uc->instruction[2]);
				else printf("i:%02X",uc->instruction[2]);
			}
			else printf(an);
			break;
		}
		//bit
		case 0x10:
		case 0x20:
		case 0x30:
		case 0x92:
		case 0xB2:
		case 0xC2:
		case 0xD2:
		{
			const char* an=getBitAddressName(uc->instruction[1]);
			if (an==NULL){
				an=getDirectAddressName(uc->getBitDirectAddress(uc->instruction[1]));
				if (an==NULL){
					if ((bool)(uc->instruction[1]&0x80)) printf("s:%02X.%i",uc->instruction[1]&0xF8,uc->instruction[1]&7);
					else printf("i:%02X.%i",0x20+(uc->instruction[1]>>3),uc->instruction[1]&7);
				}
				else{
					printf(an);
					printf(".%i",uc->instruction[1]&7);
				}
			}
			else printf(an);
			break;
		}
	}
	
	if (uc->instruction[0]>0xB3&&uc->instruction[0]<0xC0){
		if (uc->instruction[0]==0xB5){
			const char* an=getDirectAddressName(uc->instruction[1]);
			if (an==NULL){
				if ((bool)(uc->instruction[1]&0x80)) printf(", s:%02X",uc->instruction[1]);
				else printf(", i:%02X",uc->instruction[1]);
			}
			else{
				printf(", ");
				printf(an);
			}
		}
		else printf(", #0x%02X",uc->instruction[1]);
	}

	switch(uc->instruction[0]){
		//direct
		case 0x25:
		case 0x35:
		case 0x45:
		case 0x55:
		case 0x65:
		case 0x85://////////////
		case 0x95:
		case 0xA6:
		case 0xA7:
		case 0xA8:
		case 0xA9:
		case 0xAA:
		case 0xAB:
		case 0xAC:
		case 0xAD:
		case 0xAE:
		case 0xAF:
		case 0xC5:
		case 0xE5:
		{
			const char* an=getDirectAddressName(uc->instruction[1]);
			if (an==NULL){
				if ((bool)(uc->instruction[1]&0x80)) printf(", s:%02X",uc->instruction[1]);
				else printf(", i:%02X",uc->instruction[1]);
			}
			else{
				printf(", ");
				printf(an);
			}
			break;
		}
		//data8
		case 0x24:
		case 0x34:
		case 0x44:
		case 0x54:
		case 0x64:
		case 0x74:
		case 0x76:
		case 0x77:
		case 0x78:
		case 0x79:
		case 0x7A:
		case 0x7B:
		case 0x7C:
		case 0x7D:
		case 0x7E:
		case 0x7F:
		case 0x94:printf(", #0x%02X",uc->instruction[1]);break;
		case 0x43:
		case 0x53:
		case 0x63:
		case 0x75:printf(", #0x%02X",uc->instruction[2]);break;
		//data16
		case 0x90:printf(", #0x%04X",(((unsigned short)uc->instruction[1])<<8)|((unsigned short)uc->instruction[2]));break;
		//rel
		case 0xD8:
		case 0xD9:
		case 0xDA:
		case 0xDB:
		case 0xDC:
		case 0xDD:
		case 0xDE:
		case 0xDF:printf(", c:%05lX",(((unsigned long)uc->PX_out[1]&3)<<16)|((unsigned long)(uc->PC+((signed char)uc->instruction[1]))));break;
		case 0x10:
		case 0x20:
		case 0x30:
		case 0xB4:
		case 0xB5:
		case 0xB6:
		case 0xB7:
		case 0xB8:
		case 0xB9:
		case 0xBA:
		case 0xBB:
		case 0xBC:
		case 0xBD:
		case 0xBE:
		case 0xBF:
		case 0xD5:printf(", c:%05lX",(((unsigned long)uc->PX_out[1]&3)<<16)|((unsigned long)(uc->PC+((signed char)uc->instruction[2]))));break;
		//@Ri
		case 0x26:
		case 0x27:
		case 0x36:
		case 0x37:
		case 0x46:
		case 0x47:
		case 0x56:
		case 0x57:
		case 0x66:
		case 0x67:
		case 0x86:
		case 0x87:
		case 0x96:
		case 0x97:
		case 0xC6:
		case 0xC7:
		case 0xD6:
		case 0xD7:
		case 0xE6:
		case 0xE7:
			printf(", @R%i=>",uc->instruction[0]&1);
			printf("@");
			printf(getDirectAddressName(uc->getR(uc->instruction[0]&1)));
			printf("=>i:%02X",uc->getRAMByte(uc->getR(uc->instruction[0]&1)));
			break;
		case 0xE2:
		case 0xE3:
			printf(", @R%i=>",uc->instruction[0]&1);
			printf("@");
			printf(getDirectAddressName(uc->getR(uc->instruction[0]&1)));
			printf("=>e:%04X",(((unsigned short)uc->PX_out[2])<<8)|((unsigned short)uc->getRAMByte(uc->getR(uc->instruction[0]&1))));
			break;
		//Rn
		case 0x28:
		case 0x29:
		case 0x2A:
		case 0x2B:
		case 0x2C:
		case 0x2D:
		case 0x2E:
		case 0x2F:
		case 0x38:
		case 0x39:
		case 0x3A:
		case 0x3B:
		case 0x3C:
		case 0x3D:
		case 0x3E:
		case 0x3F:
		case 0x48:
		case 0x49:
		case 0x4A:
		case 0x4B:
		case 0x4C:
		case 0x4D:
		case 0x4E:
		case 0x4F:
		case 0x58:
		case 0x59:
		case 0x5A:
		case 0x5B:
		case 0x5C:
		case 0x5D:
		case 0x5E:
		case 0x5F:
		case 0x68:
		case 0x69:
		case 0x6A:
		case 0x6B:
		case 0x6C:
		case 0x6D:
		case 0x6E:
		case 0x6F:
		case 0x88:
		case 0x89:
		case 0x8A:
		case 0x8B:
		case 0x8C:
		case 0x8D:
		case 0x8E:
		case 0x8F:
		case 0x98:
		case 0x99:
		case 0x9A:
		case 0x9B:
		case 0x9C:
		case 0x9D:
		case 0x9E:
		case 0x9F:
		case 0xC8:
		case 0xC9:
		case 0xCA:
		case 0xCB:
		case 0xCC:
		case 0xCD:
		case 0xCE:
		case 0xCF:
		case 0xE8:
		case 0xE9:
		case 0xEA:
		case 0xEB:
		case 0xEC:
		case 0xED:
		case 0xEE:
		case 0xEF:
			printf(", R%i=>",uc->instruction[0]&7);
			printf(getDirectAddressName(uc->getR(uc->instruction[0]&7)));
			break;
		//A
		case 0x42:
		case 0x52:
		case 0x62:
		case 0xF0:
		case 0xF2:
		case 0xF3:
		case 0xF5:
		case 0xF6:
		case 0xF7:
		case 0xF8:
		case 0xF9:
		case 0xFA:
		case 0xFB:
		case 0xFC:
		case 0xFD:
		case 0xFE:
		case 0xFF:printf(", A");break;
		//CY
		case 0x92:printf(", CY");break;
		//bit
		case 0x72:
		case 0x82:
		case 0xA2:
		{
			printf(", ");
			const char* an=getBitAddressName(uc->instruction[1]);
			if (an==NULL){
				an=getDirectAddressName(uc->getBitDirectAddress(uc->instruction[1]));
				if (an==NULL){
					if ((bool)(uc->instruction[1]&0x80)) printf("s:%02X.%i",uc->instruction[1]&0xF8,uc->instruction[1]&7);
					else printf("i:%02X.%i",0x20+(uc->instruction[1]>>3),uc->instruction[1]&7);
				}
				else{
					printf(an);
					printf(".%i",uc->instruction[1]&7);
				}
			}
			else printf(an);
			break;
		}
		//nbit
		case 0xA0:
		case 0xB0:
		{
			printf(", !");
			const char* an=getBitAddressName(uc->instruction[1]);
			if (an==NULL){
				an=getDirectAddressName(uc->getBitDirectAddress(uc->instruction[1]));
				if (an==NULL){
					if ((bool)(uc->instruction[1]&0x80)) printf("s:%02X.%i",uc->instruction[1]&0xF8,uc->instruction[1]&7);
					else printf("i:%02X.%i",0x20+(uc->instruction[1]>>3),uc->instruction[1]&7);
				}
				else{
					printf(an);
					printf(".%i",uc->instruction[1]&7);
				}
			}
			else printf(an);
			break;
		}
		//@A+PC
		case 0x83:printf(", @A+PC=>c:%05lX",(((unsigned long)uc->PX_out[1]&3)<<16)|((unsigned long)(uc->PC+uc->getSFRByteIn(uc->ACC))));break;
		//@A+DPTR
		case 0x93:printf(", @A+DPTR=>c:%05lX",(((unsigned long)uc->PX_out[1]&3)<<16)|((unsigned long)((((unsigned short)uc->getSFRByteIn(uc->DPH))<<8)+((unsigned short)uc->getSFRByteIn(uc->DPL))+uc->getSFRByteIn(uc->ACC))));break;
		//@DPTR
		case 0xE0:printf(", @DPTR=>e:%04X",(((unsigned short)uc->getSFRByteIn(uc->DPH))<<8)+((unsigned short)uc->getSFRByteIn(uc->DPL)));break;
	}
	printf("\n");
}

 

#endif