#include "thread_messaging.h"
#include "circuit/80C32.h"
#include "circuit/SRAM_64k.h"
#include "circuit/ROM_256k.h"
#include "circuit/IOLogger.h"
#include "circuit/Latch.h"
#include "circuit/TS7514.h"
#include "circuit/TS9347.h"

#include <chrono>

#include <cstdio>

#include <fstream>

#include <cstring>

#include <iostream>

#include <chrono>
#include <thread>

void thread_circuit_main(thread_mailbox* p_mb_circuit,thread_mailbox* p_mb_video,thread_mailbox* p_mb_audio,thread_mailbox* p_mb_log,GlobalState* p_gState){
	//create ic
	SRAM_64k eram;
	ROM_256k erom;
	IOLogger iol;//debug
	m80C32 uc;
	TS7514 modem;
	TS9347wVRAM video;
	EdgeTriggeredLatchBus ALLatch;
	EdgeTriggeredLatchWire A16Latch;
	EdgeTriggeredLatchWire A17Latch;
	
	//construct circuit
	auto Dbus=[&ALLatch,&eram,&video,&uc,&iol](unsigned char d){//in ic
		//printf("Dbus %#02X\n",d);
		ALLatch.INChangeIn(d);
		eram.DChangeIn(d);
		video.DChangeIn(d);
		uc.PXChangeIn(0,d);
		iol.DChangeIn(d);
	};
	uc.subscribeP0(Dbus);//out ic
	eram.subscribeD(Dbus);
	erom.subscribeD(Dbus);
	video.subscribeD(Dbus);
	
	auto ALbus=[&eram,&erom](unsigned char d){
		//printf("ALbus %#02X\n",d);
		eram.ALChangeIn(d);
		erom.ALChangeIn(d);
	};
	ALLatch.subscribeOUT(ALbus);
	
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
	
	auto ALEwire=[&ALLatch,&A16Latch,&A17Latch,&video,&iol](bool b){
		/*printf("ALEwire ");
		printf(b?"true":"false");
		printf("\n");*/
		ALLatch.CChangeIn(b);
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
	
	auto P1bus=[&A16Latch,&A17Latch,&eram,&iol,&modem,&video,&uc](unsigned char d){
		//printf("P1bus %#02X\n",d);
		A16Latch.INChangeIn((bool)(d&1));
		A17Latch.INChangeIn((bool)(d&2));
		modem.ATxIChangeIn((bool)(d&(1<<2)));
		modem.TxDChangeIn((bool)(d&(1<<3)));
		modem.nRTSChangeIn((bool)(d&(1<<4)));
		bool nCSVideo=(bool)(d&(1<<5));
		video.nCSChangeIn(nCSVideo);
		eram.nCSChangeIn(!nCSVideo);
		iol.nCSChangeIn(nCSVideo);
		uc.PXChangeIn(1,d);
	};
	uc.subscribeP1(P1bus);
	
	auto P3bus=[&eram,&video,&uc,&iol](unsigned char d){
		bool nRD=(bool)(d&0x80);
		bool nWR=(bool)(d&0x40);
		eram.nWEChangeIn(nWR);
		video.RnWChangeIn(nWR);
		iol.nWEChangeIn(nWR);
		eram.nOEChangeIn(nRD);
		video.DSChangeIn(nRD);
		iol.nOEChangeIn(nRD);
		uc.PXChangeIn(3,d);
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
	
	//debug
	auto dbgIOIN=[p_gState,&modem](unsigned char a,unsigned char d){
		if ((a&0xF0)==0x20){
			printf("To TS9347 A: 0x%02X / D: 0x%02X\n",a,d);
		}
		else{
			printf("To IO A: 0x%02X / D: 0x%02X\n",a,d);
			//p_gState->stepByStep.store(true,std::memory_order_relaxed);
		}
		if(a==0x70){
			modem.MCnBCChangeIn((bool)(d&1));
			modem.MODEMnDTMFChangeIn((bool)(d&2));
		}
	};
	iol.subscribeIN(dbgIOIN);
	auto dbgIOOUT=[p_gState](unsigned char a,unsigned char d){
		if ((a&0xF0)==0x20){
			printf("From TS9347 A: 0x%02X / D: 0x%02X\n",a,d);
		}
		else{
			printf("From IO A: 0x%02X / D: 0x%02X\n",a,d);
			//p_gState->stepByStep.store(true,std::memory_order_relaxed);
		}
		//if(a!=0x20) r=0xff;
	};
	iol.subscribeOUT(dbgIOOUT);
	
	
	
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
	
	/*thread_message ms_p_notif;
	ms_p_notif.cmd=NOTIFICATION_BUZZER;
	for (int i=0;i<16;i++) thread_send_message(p_mb_video,&ms_p_notif);*/
	bool next_step=false;
	
	while (!p_gState->shutdown.load(std::memory_order_relaxed)){
		
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
				default:
					fprintf(stdout,"unknown cmd %i\n",ms.cmd);
					break;
			}
		}
		if (p_gState->minitelOn.load(std::memory_order_relaxed)&&(!p_gState->stepByStep.load(std::memory_order_relaxed)||next_step)){
			while (!uc.exec_instruction){
				uc.CLKTickIn();
			}
			uc.exec_instruction=false;
			next_step=false;
			/*static int div_=0;
			if (div_==0) std::this_thread::sleep_for(std::chrono::milliseconds(1));
			div_++;
			div_%=50;*/
		}
		//eram.last_memory_operation.store((eram.last_memory_operation.load(std::memory_order_acquire)+1)%65536,std::memory_order_relaxed);
	}
}