#ifndef DECOMP_M12_ROM_H
#define DECOMP_M12_ROM_H
#include <cstdio>
#include "circuit/ROM_256k.h"
#include "circuit/80C32.h"

class RuntimeDecompiler{
	public:
		RuntimeDecompiler(m80C32* uc){
			this->uc=uc;
			FILE* f=fopen("./rom_map.bin","rb");
			fseek(f,0,SEEK_SET);
			fread( this->rom_map, 1, EROM_SIZE, f );
			fclose(f);
			for (int i=0;i<4;i++){
				this->rom_map[(i<<16)]|=this->ADDRESS_INTERRUPT;
				this->rom_map[(i<<16)|0x03]|=this->ADDRESS_INTERRUPT;
				this->rom_map[(i<<16)|0x0B]|=this->ADDRESS_INTERRUPT;
				this->rom_map[(i<<16)|0x13]|=this->ADDRESS_INTERRUPT;
				this->rom_map[(i<<16)|0x1B]|=this->ADDRESS_INTERRUPT;
				this->rom_map[(i<<16)|0x23]|=this->ADDRESS_INTERRUPT;
				this->rom_map[(i<<16)|0x2B]|=this->ADDRESS_INTERRUPT;
			}
		}
		~RuntimeDecompiler(){
			FILE* f=fopen("./rom_map.bin","wb");
			fseek(f,0,SEEK_SET);
			fwrite( this->rom_map, 1, EROM_SIZE, f );
			fclose(f);
		}
		void update(){
			unsigned long addr;
			//don't forget interrupt call
			if (!this->isProcessingInterruptCall()){
				unsigned char l=this->uc->i_length[this->uc->instruction[0]];
				for (int i=1;i<l;i++){
					addr=(((unsigned long)this->uc->PX_out[1]&3)<<16)|((unsigned long)((unsigned short)(this->uc->PC-i)));
					this->rom_map[addr]|=this->INSTRUCTION_OTHER_BYTE;
				}
				addr=(((unsigned long)this->uc->PX_out[1]&3)<<16)|((unsigned long)((unsigned short)this->uc->PC-l));
				this->rom_map[addr]|=this->INSTRUCTION_FIRST_BYTE;
			}
			
			this->updateRamTracker();
			
			switch(this->uc->instruction[0]){
				case 0x11://CALL
				case 0x31:
				case 0x51:
				case 0x71:
				case 0x91:
				case 0xB1:
				case 0xD1:
				case 0xF1:
					addr=((unsigned long)this->uc->PX_out[1]&3)<<16;
					addr|=((unsigned long)this->uc->PC&0xF800);
					addr|=((unsigned long)this->uc->instruction[0]&0xE0)<<3;
					addr|=this->uc->instruction[1];
					this->rom_map[addr]|=this->ADDRESS_CALL;
					break;
				case 0x12://LCALL
					addr=(((unsigned long)this->uc->PX_out[1]&3)<<16)|((unsigned long)(((unsigned short)this->uc->instruction[1])<<8)|((unsigned short)this->uc->instruction[2]));
					this->rom_map[addr]|=this->ADDRESS_CALL;
					break;
					
				case 0x22://RET
					addr=(((unsigned long)this->uc->PX_out[1]&3)<<16)|((unsigned long)((((unsigned short)this->uc->getRAMByte(this->uc->getSFRByteIn(this->uc->SP)))<<8)+((unsigned short)this->uc->getRAMByte(this->uc->getSFRByteIn(this->uc->SP)-1))));
					this->rom_map[addr]|=this->ADDRESS_RET;
					if (!(this->ram_address_tracker[this->uc->getSFRByteIn(this->uc->SP)]&&this->ram_address_tracker[this->uc->getSFRByteIn(this->uc->SP)-1])) this->rom_map[addr]|=this->ADDRESS_CALL_INDIRECT;
					break;
				case 0x32://RETI
					addr=(((unsigned long)this->uc->PX_out[1]&3)<<16)|((unsigned long)((((unsigned short)this->uc->getRAMByte(this->uc->getSFRByteIn(this->uc->SP)))<<8)+((unsigned short)this->uc->getRAMByte(this->uc->getSFRByteIn(this->uc->SP)-1))));
					if (!(this->ram_address_tracker[this->uc->getSFRByteIn(this->uc->SP)]&&this->ram_address_tracker[this->uc->getSFRByteIn(this->uc->SP)-1])) this->rom_map[addr]|=this->ADDRESS_CALL_INDIRECT;
					break;
					
				case 0x83://MOVC
					addr=(((unsigned long)this->uc->PX_out[1]&3)<<16)|((unsigned long)(this->uc->PC+this->uc->getSFRByteIn(this->uc->ACC)));
					this->rom_map[addr]|=this->DATA;
					break;
				case 0x93:
					addr=(((unsigned long)this->uc->PX_out[1]&3)<<16)|((unsigned long)((((unsigned short)this->uc->getSFRByteIn(uc->DPH))<<8)+((unsigned short)this->uc->getSFRByteIn(uc->DPL))+((unsigned short)this->uc->getSFRByteIn(this->uc->ACC))));
					this->rom_map[addr]|=this->DATA;
					break;
			}
		}
	private:
		m80C32* uc;
		unsigned char rom_map[EROM_SIZE]={0};
		constexpr static unsigned char INSTRUCTION_FIRST_BYTE=0x01;
		constexpr static unsigned char INSTRUCTION_OTHER_BYTE=0x02;
		constexpr static unsigned char ADDRESS_CALL=0x04;
		constexpr static unsigned char ADDRESS_RET=0x08;
		constexpr static unsigned char ADDRESS_INTERRUPT=0x10;
		constexpr static unsigned char ADDRESS_CALL_INDIRECT=0x20;
		constexpr static unsigned char DATA=0x80;
		
		bool ram_address_tracker[256]={false};//track if memory is address data to know when there is instructions like PUSH, PUSH then RET
		
		bool isProcessingInterruptCall(){//not exact but good enough: false positive possible -> trigger if LCALL to interrupt address (could be a normal function call but likely not)
			bool b=false;
			if (this->uc->instruction[0]==0x12&&this->uc->instruction[1]==0x00&&this->uc->getBitIn(this->uc->EA)){
				switch (this->uc->instruction[2]){
					case 0x03:
					case 0x0B:
					case 0x13:
					case 0x1B:
					case 0x23:
					case 0x2B:
						b=true;
						break;
				}
			}
			return b;
		}
		
		void updateRamTracker(){
			switch(this->uc->instruction[0]){
				case 0x11://CALL
				case 0x31:
				case 0x51:
				case 0x71:
				case 0x91:
				case 0xB1:
				case 0xD1:
				case 0xF1:
				case 0x12://LCALL
					this->ram_address_tracker[this->uc->getSFRByteIn(this->uc->SP)+1]=true;
					this->ram_address_tracker[this->uc->getSFRByteIn(this->uc->SP)+2]=true;
					break;
				case 0xC0://PUSH
					this->ram_address_tracker[this->uc->getSFRByteIn(this->uc->SP)+1]=false;
					break;
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
					if (!(bool)(this->uc->instruction[1]&0x80)) this->ram_address_tracker[this->uc->instruction[1]]=false;
					break;
				
				case 0x85://direct 2
					if (!(bool)(this->uc->instruction[2]&0x80)) this->ram_address_tracker[this->uc->instruction[2]]=false;
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
					this->ram_address_tracker[this->uc->getRAMByte(this->uc->getR(this->uc->instruction[0]&1))]=false;
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
					this->ram_address_tracker[this->uc->getR(this->uc->instruction[0]&7)]=false;
					break;
				
				case 0x10://bit
				case 0x92:
				case 0xB2:
				case 0xC2:
				case 0xD2:
					if (!(bool)(this->uc->instruction[1]&0x80)) this->ram_address_tracker[this->uc->getBitDirectAddress(this->uc->instruction[1])]=false;
			}
		}
};



#endif