#include "thread_messaging.h"
#include "circuit/80C32.h"
#include "circuit/SRAM_64k.h"
#include "circuit/ROM_256k.h"
#include "circuit/IOLogger.h"
#include "circuit/Latch.h"
#include "circuit/TS7514.h"
#include "circuit/TS9347.h"
#include "circuit/CPLD.h"
#include "circuit/WatchdogTimer.h"
#include "circuit/Keyboard.h"
#include "circuit/clocks.h"

#include <chrono>

#include <cstdio>

#include <fstream>

#include <cstring>

#include <iostream>

#include <chrono>
#include <thread>

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

void thread_circuit_main(thread_mailbox* p_mb_circuit,thread_mailbox* p_mb_video,thread_mailbox* p_mb_audio,thread_mailbox* p_mb_log,GlobalState* p_gState){
	//create ic
	SRAM_64k eram;
	ROM_256k erom;
	IOLogger iol;//debug
	m80C32 uc;
	TS7514 modem;
	TS9347wVRAM video;
	//EdgeTriggeredLatchBus ALLatch;
	EdgeTriggeredLatchWire A16Latch;
	EdgeTriggeredLatchWire A17Latch;
	MBSL_4000FH5_5 cpld;
	WatchdogTimer wt;
	Keyboard kb;
	Clocks CLKs;
	
	//construct circuit
	auto Dbus=[&cpld,&eram,&video,&uc,&iol](unsigned char d){//in ic
		//printf("Dbus %#02X\n",d);
		cpld.DChangeIn(d);
		eram.DChangeIn(d);
		video.DChangeIn(d);
		uc.PXChangeIn(0,d);
		iol.DChangeIn(d);
	};
	uc.subscribeP0(Dbus);//out ic
	cpld.subscribeD(Dbus);
	eram.subscribeD(Dbus);
	erom.subscribeD(Dbus);
	video.subscribeD(Dbus);
	
	auto ALbus=[&eram,&erom](unsigned char d){
		//printf("ALbus %#02X\n",d);
		eram.ALChangeIn(d);
		erom.ALChangeIn(d);
	};
	//ALLatch.subscribeOUT(ALbus);
	cpld.subscribeAL(ALbus);
	
	auto AHbus=[&eram,&erom,&uc](unsigned char d){
		//printf("AHbus %#02X\n",d);
		eram.AHChangeIn(d);
		erom.AHChangeIn(d);
		uc.PXChangeIn(2,d);
	};
	uc.subscribeP2(AHbus);
	
	auto nPSENwire=[&erom](bool b){
		/*printf("nPSENwire ");
		printf(b?"true":"false");
		printf("\n");*/
		erom.nGChangeIn(b);
	};
	uc.subscribenPSEN(nPSENwire);
	
	auto ALEwire=[&cpld,&A16Latch,&A17Latch,&video,&iol](bool b){
		/*printf("ALEwire ");
		printf(b?"true":"false");
		printf("\n");*/
		cpld.ALEChangeIn(b);
		A16Latch.CChangeIn(b);
		A17Latch.CChangeIn(b);
		iol.ALEChangeIn(b);
		video.ASChangeIn(b);
	};
	uc.subscribeALE(ALEwire);
	
	auto A16wire=[&erom](bool b){
		/*printf("A16wire ");
		printf(b?"true":"false");
		printf("\n");*/
		erom.A16ChangeIn(b);
	};
	A16Latch.subscribeOUT(A16wire);
	
	auto A17wire=[&erom](bool b){
		/*printf("A17wire ");
		printf(b?"true":"false");
		printf("\n");*/
		erom.A17ChangeIn(b);
	};
	A17Latch.subscribeOUT(A17wire);
	
	auto P1bus=[&A16Latch,&A17Latch,&cpld,&iol,&modem,&video,&uc](unsigned char d){
		//printf("P1bus %#02X\n",d);
		A16Latch.INChangeIn((bool)(d&1));
		A17Latch.INChangeIn((bool)(d&2));
		modem.ATxIChangeIn((bool)(d&(1<<2)));
		modem.TxDChangeIn((bool)(d&(1<<3)));
		modem.nRTSChangeIn((bool)(d&(1<<4)));
		bool nCSVideo=(bool)(d&(1<<5));
		video.nCSChangeIn(nCSVideo);
		cpld.nCSChangeIn(nCSVideo);
		iol.nCSChangeIn(nCSVideo);
		uc.PXChangeIn(1,d);
	};
	uc.subscribeP1(P1bus);
	
	auto nCSRAMwire=[&eram](bool b){
		eram.nCSChangeIn(b);
	};
	cpld.subscribenCSRAM(nCSRAMwire);
	
	auto P3bus=[&cpld,&eram,&video,&uc,&iol](unsigned char d){
		bool nRD=(bool)(d&0x80);
		bool nWR=(bool)(d&0x40);
		eram.nWEChangeIn(nWR);
		video.RnWChangeIn(nWR);
		cpld.nWEChangeIn(nWR);
		iol.nWEChangeIn(nWR);
		eram.nOEChangeIn(nRD);
		video.DSChangeIn(nRD);
		cpld.nOEChangeIn(nRD);
		iol.nOEChangeIn(nRD);
		uc.PXChangeIn(3,d);
		static bool txd=false;
		if (txd!=((bool)(d&0x02))){
			txd=!txd;
			printf("------------ TxD %i\n",(int)txd);
		}
	};
	uc.subscribeP3(P3bus);
	
	auto nDCDwire=[&uc](bool b){
		uc.PXYChangeIn(1,6,b);
	};
	modem.subscribenDCD(nDCDwire);
	
	auto mRxDwire=[&uc](bool b){
		uc.PXYChangeIn(3,3,b);
	};
	modem.subscribeRxD(mRxDwire);
	
	bool kb_s1=false;
	bool kb_s2=false;
	
	auto CPLDIObus=[&modem,&wt,&kb,&kb_s1,&kb_s2](unsigned char d){
		modem.MCnBCChangeIn((bool)(d&1));
		modem.MODEMnDTMFChangeIn((bool)(d&2));
		wt.KICKChangeIn((bool)(d&8));
		kb_s1=!(bool)(d&4);
		kb.serialChangeIn(kb_s1||kb_s2);
	};
	cpld.subscribePIO(CPLDIObus);
	
	auto RSTwire=[&uc](bool b){
		uc.ResetChangeIn(b);
	};
	cpld.subscribeRST(RSTwire);
	
	auto WTwire=[&cpld](bool b){
		cpld.WATCHDOGChangeIn(b);
	};
	wt.subscribeRST(WTwire);
	
	auto KeyboardSerialOut=[&cpld](bool b){
		cpld.serialChangeIn(b);
	};
	kb.subscribeSerial(KeyboardSerialOut);
	auto KeyboardSerialIn=[&kb,&kb_s1,&kb_s2](bool b){
		kb_s2=b;
		kb.serialChangeIn(kb_s1||kb_s2);
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	};
	cpld.subscribeSerial(KeyboardSerialIn);
	
	//debug
	stackMonitor sm=stackMonitor(&uc);
	/*sm.SPManualyModified=[p_gState,&uc](){
		printf("SP manualy modified c:%05lX\n",(((unsigned long)uc.PX_out[1]&3)<<16)|((unsigned long)(uc.PC-uc.i_length[uc.instruction[0]])));
		//p_gState->stepByStep.store(true,std::memory_order_relaxed);
	};
	sm.addressPOPed=[p_gState,&uc](bool high){
		printf("Address ");
		printf(high?"high":"low");
		printf(" poped c:%05lX\n",(((unsigned long)uc.PX_out[1]&3)<<16)|((unsigned long)(uc.PC-uc.i_length[uc.instruction[0]])));
		p_gState->stepByStep.store(true,std::memory_order_relaxed);
	};
	sm.stackOverwrited=[p_gState,&uc](){
		printf("Stack overwrited c:%05lX\n",(((unsigned long)uc.PX_out[1]&3)<<16)|((unsigned long)(uc.PC-uc.i_length[uc.instruction[0]])));
		p_gState->stepByStep.store(true,std::memory_order_relaxed);		
	};
	sm.funcReturned=[p_gState,&uc](bool ud,bool ind,bool old){
		if (ud){
			printf("Return to undefined c:%05lX\n",(((unsigned long)uc.PX_out[1]&3)<<16)|((unsigned long)(uc.PC-uc.i_length[uc.instruction[0]])));
			p_gState->stepByStep.store(true,std::memory_order_relaxed);
		}
		else{
			if (ind||old){
				printf("Return to ");
				if (old) printf("old ");
				if (ind) printf("indirect ");
				unsigned char sp=uc.getSFRByteIn(uc.SP);
				printf("address c:%05lX -> c:%05lX\n",(((unsigned long)uc.PX_out[1]&3)<<16)|((unsigned long)(uc.PC-uc.i_length[uc.instruction[0]])),(((unsigned long)uc.PX_out[1]&3)<<16)|(((unsigned short)uc.getRAMByte(sp))<<8)|((unsigned short)uc.getRAMByte(sp-1)));
			}
		}
	};*/
	
	auto dbgIOIN=[p_gState,&modem](unsigned char a,unsigned char d){
		if ((a&0xF0)==0x20){
			//printf("To TS9347 A: 0x%02X / D: 0x%02X\n",a,d);
		}
		else if ((a&0xF0)==0x40){
			//printf("To RTC A: 0x%02X / D: 0x%02X\n",a,d);
		}
		else if (a==0x50){
			printf("To Keyboard A: 0x%02X / D: 0x%02X\n",a,d);
		}
		else if (a==0x51){
			printf("To CPLD Status A: 0x%02X / D: 0x%02X\n",a,d);
		}
		else if ((a&0xF0)==0x70){
			static unsigned char io=0;
			if ((bool)((d^io)&(~(1<<3)))){//dont print when watchdog timer kicked
				io=d;
				printf("To CPLD Pin A: 0x%02X / D: 0x%02X\n",a,d);
			}
			//p_gState->stepByStep.store(true,std::memory_order_relaxed);
		}
		else{
			printf("To IO A: 0x%02X / D: 0x%02X\n",a,d);
			//p_gState->stepByStep.store(true,std::memory_order_relaxed);
		}
	};
	iol.subscribeIN(dbgIOIN);
	auto dbgIOOUT=[p_gState](unsigned char a,unsigned char d){
		if ((a&0xF0)==0x20){
			//printf("From TS9347 A: 0x%02X / D: 0x%02X\n",a,d);
		}
		else if ((a&0xF0)==0x40){
			//printf("From RTC A: 0x%02X / D: 0x%02X\n",a,d);
		}
		else if (a==0x50){
			printf("From Keyboard A: 0x%02X / D: 0x%02X\n",a,d);
			//p_gState->stepByStep.store(true,std::memory_order_relaxed);
		}
		else if (a==0x51){
			static unsigned char stat=0;
			if (stat!=d){
				stat=d;
				printf("From CPLD Status A: 0x%02X / D: 0x%02X\n",a,d);
			}
			//p_gState->stepByStep.store(true,std::memory_order_relaxed);
		}
		else if ((a&0xF0)==0x70){
			//printf("From CPLD Pin A: 0x%02X / D: 0x%02X\n",a,d);
			//p_gState->stepByStep.store(true,std::memory_order_relaxed);
		}
		else{
			printf("From IO A: 0x%02X / D: 0x%02X\n",a,d);
			//p_gState->stepByStep.store(true,std::memory_order_relaxed);
		}
		//if(a!=0x20) r=0xff;
	};
	iol.subscribeOUT(dbgIOOUT);
	uc.debug_signal_alu_before_exec=[p_gState,&uc,&sm](){
		sm.updateState();
		unsigned long addr=((unsigned long)uc.PC)-uc.i_length[uc.instruction[0]]+(((unsigned long)uc.PX_out[1]&3)<<16);
		/*if (addr==0x1CC68){
			p_gState->stepByStep.store(true,std::memory_order_relaxed);
		}*/
		/*if (((unsigned long)uc.PC)-uc.i_length[uc.instruction[0]]+(((unsigned long)uc.PX_out[1]&3)<<16)==0x10074){
			p_gState->stepByStep.store(true,std::memory_order_relaxed);
		}*/
		/*if (uc.instruction[0]==0xD2&&uc.instruction[1]==0x99){//SETB TI
			p_gState->stepByStep.store(true,std::memory_order_relaxed);
		}*/
		if ((uc.instruction[0]==0xE0||uc.instruction[0]==0xF0)&&(!(bool)(uc.PX_out[1]&(1<<5)))){
			p_gState->stepByStep.store(true,std::memory_order_relaxed);
		}
		//print_m12_alu_instruction(&uc);
	};
	
	thread_message ms_p_ram;
	ms_p_ram.p=(void*)&eram;
	ms_p_ram.cmd=ERAM;
	thread_send_message(p_mb_video,&ms_p_ram);
	
	thread_message ms_p_rom;
	ms_p_rom.p=(void*)&erom;
	ms_p_rom.cmd=EROM;
	thread_send_message(p_mb_video,&ms_p_rom);
	
	thread_message ms_p_vc;
	ms_p_vc.p=(void*)&video;
	ms_p_vc.cmd=VC;
	thread_send_message(p_mb_video,&ms_p_vc);
	
	thread_message ms_p_uc;
	ms_p_uc.p=(void*)&uc;
	ms_p_uc.cmd=UC;
	thread_send_message(p_mb_video,&ms_p_uc);
	
	thread_message ms_p_cpld;
	ms_p_cpld.p=(void*)&cpld;
	ms_p_cpld.cmd=CPLD;
	thread_send_message(p_mb_video,&ms_p_cpld);
	
	/*thread_message ms_p_notif;
	ms_p_notif.cmd=NOTIFICATION_BUZZER;
	for (int i=0;i<16;i++) thread_send_message(p_mb_video,&ms_p_notif);*/
	
	bool next_step=false;
	
	auto stopC=[p_gState](){
		return p_gState->shutdown.load(std::memory_order_relaxed);
	};
	CLKs.setStopCondition(stopC);
	auto pauseC=[p_gState,&next_step,&uc](){
		bool p=(!p_gState->minitelOn.load(std::memory_order_relaxed))||(p_gState->stepByStep.load(std::memory_order_relaxed)&&(uc.exec_instruction||!next_step));
		if (uc.exec_instruction){
			uc.exec_instruction=false;
			next_step=false;
		}
		return p;
	};
	CLKs.setPauseCondition(pauseC);
	auto Loop=[p_mb_circuit,&eram,&erom,&uc,&modem,&next_step,&kb,p_gState](){
		thread_message ms;
		std::ifstream eram_file;
		std::ifstream erom_file;
		const char* eram_fn;
		const char* erom_fn;
		while (thread_receive_message(p_mb_circuit,&ms)>=0){
			switch(ms.cmd){
				case LOAD_ERAM:
					if (ms.p==NULL){
						unsigned char eram_cpy[ERAM_SIZE];
						std::fill_n(eram_cpy,ERAM_SIZE,0);
						eram.set(eram_cpy);
						printf("erase eram\n");
					}
					else{
						eram_fn=(const char*)ms.p;
						unsigned char eram_cpy[ERAM_SIZE];
						eram_file.open(eram_fn,std::ios::in|std::ios::binary);
						eram_file.read((char*)eram_cpy,ERAM_SIZE);
						eram_file.close();
						eram.set(eram_cpy);
						printf("load eram ");
						printf(eram_fn);
						printf("\n");
						free(ms.p);
					}
					break;
				case LOAD_EROM:
					if(ms.p==NULL){
						unsigned char erom_cpy[EROM_SIZE];
						std::fill_n(erom_cpy,EROM_SIZE,0);
						erom.set(erom_cpy);
						printf("erase erom\n");
					}
					else{
						erom_fn=(const char*)ms.p;
						unsigned char erom_cpy[EROM_SIZE];
						erom_file.open(erom_fn,std::ios::in|std::ios::binary);
						erom_file.read((char*)erom_cpy,EROM_SIZE);
						erom_file.close();
						erom.set(erom_cpy);
						printf("load erom ");
						printf(erom_fn);
						printf("\n");
						free(ms.p);
					}
					break;
				case EMU_ON:
					uc.Reset();
					modem.Reset();
					p_gState->minitelOn.store(true,std::memory_order_relaxed);
					printf("power on\n");
					break;
				case EMU_OFF:
					p_gState->minitelOn.store(false,std::memory_order_relaxed);
					printf("power off\n");
					break;
				case EMU_NEXT_STEP:
					next_step=true;
					break;
				case KEYBOARD_STATE_UPDATE:
				{
					keyboard_message* kbm=(keyboard_message*)ms.p;
					kb.KeyboardChangeIn(kbm);
					delete kbm;
					break;
				}
				default:
					fprintf(stdout,"unknown cmd %i\n",ms.cmd);
					break;
			}
		}
	};
	CLKs.subscribeLoop(Loop);
	auto CLKTick14745600=[&uc,&wt](){
		uc.CLKTickIn();
		wt.incrementTimer();
	};
	CLKs.subscribe14745600Hz(CLKTick14745600);
	auto CLKTick600=[&kb,&cpld](){
		cpld.CLKTickIn();
		kb.CLKTickIn();
	};
	CLKs.subscribe600Hz(CLKTick600);
	
	
	
	
	CLKs.start();
	
	
	
}