#include "thread_messaging.h"
#include "circuit/80C32.h"
#include "circuit/SRAM_64k.h"
#include "circuit/ROM_256k.h"
#include "circuit/Latch.h"
#include "circuit/TS7514.h"
#include "circuit/TS9347.h"
#include "circuit/CPLD.h"
#include "circuit/WatchdogTimer.h"
#include "circuit/Keyboard.h"
#include "circuit/clocks.h"
#include "circuit/CRTBuffer.h"
#include "circuit/AudioBuffer.h"
#include "circuit/BuzzerFilter.h"
#include "circuit/SpeakerFilter.h"
//#include "circuit/DIN5Interface.h"
#include "circuit/PhoneLineWirring.h"

#include "circuit/debug/IOLogger.h"
#include "circuit/debug/dbg_80C32.h"
#include "circuit/debug/decomp_m12_rom.h"

#include "FileAccess.h"

#include <chrono>

#include <cstdio>

#include <fstream>

#include <cstring>

#include <iostream>

#include <thread>

//#include "circuit/DIN5/DIN5InterfaceLocalWebsocket.h"
#include "circuit/DIN5/MinitelNetwork.h"
#include "circuit/RTC/RTCNetwork.h"

#include <vector>

#include <filesystem>

#define GLAD_GL_IMPLEMENTATION
#include "glad/glad.h"
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

void thread_circuit_main(Mailbox* p_mb_circuit,Mailbox* p_mb_video,GlobalState* p_gState){
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
	SimplifiedMinitelNetwork smn;
	SimplifiedMinitelNetworkAppLocalWebsocket smnalw;
	smn.registerApp(&smnalw);
	SimplifiedMinitelNetworkAppPrinter smnap;
	smn.registerApp(&smnap);
	SimplifiedMinitelNetworkAppAutoStart smnas;
	smn.registerApp(&smnas);
	
	CRTBuffer crtb;
	AudioBuffer ab;
	BuzzerFilter bzf(3000);
	SpeakerFilter spkf;
	
	RTCNetwork rtcn;
	RTCServiceMinipavi rtcmp;
	rtcn.subscribeService((RTCService*)&rtcmp);
	PhoneLineWire phoneLine;
	
#ifdef M12_USE_DECOMP_TOOLS
	RuntimeDecompiler rtd(&uc);
#endif
	
	
	
	//construct circuit
	auto Dbus=[&cpld,&eram,&video,&uc,&iol](unsigned char d){//in ic
		cpld.DChangeIn(d);
		eram.DChangeIn(d);
		video.DChangeIn(d);
		uc.PXChangeIn(uc.P0,d);
		iol.DChangeIn(d);
	};
	uc.subscribeP0(std::cref(Dbus));//out ic
	cpld.subscribeD(std::cref(Dbus));
	eram.subscribeD(std::cref(Dbus));
	erom.subscribeD(std::cref(Dbus));
	video.subscribeD(std::cref(Dbus));
	
	auto ALbus=[&eram,&erom](unsigned char d){
		eram.ALChangeIn(d);
		erom.ALChangeIn(d);
	};
	cpld.subscribeAL(ALbus);
	
	auto AHbus=[&eram,&erom,&uc](unsigned char d){
		eram.AHChangeIn(d);
		erom.AHChangeIn(d);
		uc.PXChangeIn(uc.P2,d);
	};
	uc.subscribeP2(AHbus);
	
	auto nPSENwire=[&erom](bool b){
		erom.nGChangeIn(b);
	};
	uc.subscribenPSEN(nPSENwire);
	
	auto ALEwire=[&cpld,&A16Latch,&A17Latch,&video,&iol](bool b){
		cpld.ALEChangeIn(b);
		A16Latch.CChangeIn(b);
		A17Latch.CChangeIn(b);
		iol.ALEChangeIn(b);
		video.ASChangeIn(b);
	};
	uc.subscribeALE(std::cref(ALEwire));
	
	auto A16wire=[&erom](bool b){
		erom.A16ChangeIn(b);
	};
	A16Latch.subscribeOUT(A16wire);
	
	auto A17wire=[&erom](bool b){
		erom.A17ChangeIn(b);
	};
	A17Latch.subscribeOUT(A17wire);
	
	unsigned char P1_uc=0xFF;
	unsigned char P1_ext=0xFF;
	auto P1bus=[&A16Latch,&A17Latch,&cpld,&iol,&modem,&video,&uc,&P1_uc,&P1_ext](unsigned char d){
		P1_uc=d;
		d&=P1_ext;
		A16Latch.INChangeIn((bool)(d&1));
		A17Latch.INChangeIn((bool)(d&2));
		modem.ATxIChangeIn((bool)(d&(1<<2)));
		modem.TxDChangeIn((bool)(d&(1<<3)));
		modem.nRTSChangeIn((bool)(d&(1<<4)));
		bool nCSVideo=(bool)(d&(1<<5));
		video.nCSChangeIn(nCSVideo);
		cpld.nCSChangeIn(nCSVideo);
		iol.nCSChangeIn(nCSVideo);
		uc.PXChangeIn(uc.P1,d);
	};
	uc.subscribeP1(std::cref(P1bus));
	
	auto nDCDwire=[&uc,&P1_uc,&P1_ext](bool b){
		P1_ext&=~(1<<6);
		P1_ext|=(b?(1<<6):0);
		uc.PXChangeIn(uc.P1,P1_ext&P1_uc);
	};
	modem.subscribenDCD(nDCDwire);
	
	auto nCSRAMwire=[&eram](bool b){
		eram.nCSChangeIn(b);
	};
	cpld.subscribenCSRAM(nCSRAMwire);
	
	unsigned char P3_uc=0xFF;
	unsigned char P3_ext=0xFF;
	bool PT_in=true;
	//P3 unfinished
	auto P3bus=[&cpld,&eram,&video,&uc,&iol,&wt,&smn,&P3_uc,&P3_ext,&PT_in](unsigned char d){
		P3_uc=d;
		d&=P3_ext;
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
		
		//l6720.PTSChangeIn((bool)(d&0x10));
		P3_ext&=~0x20;
		P3_ext|=(PT_in&&((bool)(P3_uc&0x10)))?0:0x20;
		d&=P3_ext;
		smn.PTChangeIn(!(bool)(d&0x20));
		
		uc.PXChangeIn(uc.P3,d);
		wt.ENChangeIn((bool)(d&0x04));
		smn.RxChangeIn((bool)(d&0x02));
		//bypass L6720 -> buffer
	};
	uc.subscribeP3(std::cref(P3bus));
	
	auto PTEwire=[&uc,&smn,&P3_uc,&P3_ext,&PT_in](bool b){
		PT_in=b;
		
		P3_ext&=~0x20;
		P3_ext|=(PT_in&&((bool)(P3_uc&0x10)))?0:0x20;
		smn.PTChangeIn(b&&(bool)(P3_uc&0x10));
		uc.PXChangeIn(uc.P3,P3_uc&P3_ext);
	};
	smn.subscribePT(PTEwire);
	
	//bypass L6720 + dissociate PT in and out
	//PTE is on P3_ext
	//PTE=!(PTS&&PT_in)
	//PT_out=PTS&&PT_in
	
	//bypass L6720 -> buffer
	auto DIN5Txwire=[&uc,&P3_uc,&P3_ext](bool b){
		if (b) P3_ext|=1;
		else P3_ext&=~(1);
		uc.PXChangeIn(uc.P3,P3_uc&P3_ext);
	};
	smn.subscribeTx(DIN5Txwire);
	
	auto mRxDwire=[&uc,&P3_uc,&P3_ext](bool b){
		P3_ext&=~(1<<3);
		P3_ext|=(b?(1<<3):0);
		uc.PXChangeIn(uc.P3,P3_ext&P3_uc);
	};
	modem.subscribeRxD(mRxDwire);
	
	bool kb_s1=false;
	bool kb_s2=false;
	
	auto CPLDIObus=[&modem,&wt,&kb,&kb_s1,&kb_s2,&smn,&phoneLine,p_mb_video,&crtb](unsigned char d){
		modem.MCnBCChangeIn((bool)(d&1));
		modem.MODEMnDTMFChangeIn((bool)(d&2));
		kb_s1=(bool)(d&4);
		kb.serialChangeIn(kb_s1&&kb_s2);
		wt.KICKChangeIn((bool)(d&8));
		//0x10->close modem circuit
		static bool close_modem=false;
		if (close_modem!=((bool)(d&0x10))){
			close_modem=(bool)(d&0x10);
			phoneLine.closeRelay(close_modem);
			//modem.RA2ChangeIn(close_modem?0x0400:0);//test
			if (close_modem) printf("modem connected\n");
			else printf("modem disconnected\n");
		}
		//0x20->crt power
		crtb.CRTPowerChangeIn((bool)(d&0x20));
		//0x40->din power - not used in the emulator
		smn.PWRChangeIn((bool)(d&0x40));
		//0x80->minitel memory access? - not wired
	};
	cpld.subscribePIO(std::cref(CPLDIObus));
	
	auto RSTwire=[&uc,p_mb_video,p_gState,&eram](bool b){
		uc.ResetChangeIn(b);
		static bool bp=true;
		if ((!b)&&bp){
			//printf("reboot\n");
			thread_message ms_p_notif;
			ms_p_notif.cmd=NOTIFICATION_REBOOT;
			p_mb_video->send(&ms_p_notif);
		}
		else if((!bp)&&b){//save ram state
			//printf("power down\n");
			static unsigned char eram_cpy[ERAM_SIZE];
			eram.copy((unsigned char*)eram_cpy);
			writeM(p_gState->p_thread_mutex,p_gState->eram,eram_cpy,ERAM_SIZE);
		}
		bp=b;
		p_gState->minitelOn.store(!b,std::memory_order_relaxed);
			
	};
	cpld.subscribeRST(RSTwire);
	
	auto WTwire=[&cpld](bool b){
		cpld.WATCHDOGChangeIn(b);
	};
	wt.subscribeRST(WTwire);
	
	auto WWTwire=[&uc,&P3_uc,&P3_ext,&wt](bool b){
		P3_ext&=~0x04;
		P3_ext|=(b?0x04:0);
		unsigned char d=P3_ext&P3_uc;
		uc.PXChangeIn(uc.P3,d);
		wt.ENChangeIn((bool)(d&0x04));
	};
	wt.subscribenWRST(WWTwire);
	
	auto KeyboardSerialOut=[&cpld](bool b){
		cpld.serialChangeIn(b);
	};
	kb.subscribeSerial(KeyboardSerialOut);
	auto KeyboardSerialIn=[&kb,&kb_s1,&kb_s2](bool b){
		kb_s2=b;
		kb.serialChangeIn(kb_s1&&kb_s2);
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	};
	cpld.subscribeSerial(KeyboardSerialIn);
	
	auto CRTVideoIn=[&crtb](unsigned char* v){
		crtb.VideoChangeIn(v);
	};
	video.subscribeVideo(CRTVideoIn);
	
	auto VideoSignal=[](){
		glfwPostEmptyEvent();
	};
	crtb.subscribeSignal(VideoSignal);
	
	/*unsigned short ModemPhoneLineState=0;
	unsigned short KeyboardPhoneLineState=0;
	unsigned short RTCPhoneLineState=0;
	modem.subscribeATO([&rtcn,&kb,&modem,&ModemPhoneLineState,&KeyboardPhoneLineState,&RTCPhoneLineState](unsigned short d){
		printf("ATO %04X\n",d);
		ModemPhoneLineState=d;
		unsigned short st=ModemPhoneLineState|KeyboardPhoneLineState|RTCPhoneLineState;
		printf("PL %04X\n",st);
		rtcn.phoneLineChangeIn(st);
		kb.phoneLineChangeIn(st);
		modem.RA2ChangeIn(st);
	});
	kb.subscribePhoneLine([&rtcn,&kb,&modem,&ModemPhoneLineState,&KeyboardPhoneLineState,&RTCPhoneLineState](unsigned short d){
		printf("KB %04X\n",d);
		KeyboardPhoneLineState=d;
		unsigned short st=ModemPhoneLineState|KeyboardPhoneLineState|RTCPhoneLineState;
		printf("PL %04X\n",st);
		rtcn.phoneLineChangeIn(st);
		kb.phoneLineChangeIn(st);
		modem.RA2ChangeIn(st);
	});
	rtcn.subscribePhoneLine([&rtcn,&kb,&modem,&ModemPhoneLineState,&KeyboardPhoneLineState,&RTCPhoneLineState](unsigned short d){
		printf("RTC %04X\n",d);
		RTCPhoneLineState=d;
		unsigned short st=ModemPhoneLineState|KeyboardPhoneLineState|RTCPhoneLineState;
		printf("PL %04X\n",st);
		rtcn.phoneLineChangeIn(st);
		kb.phoneLineChangeIn(st);
		modem.RA2ChangeIn(st);
	});*/
	rtcn.subscribePhoneLine([&phoneLine](unsigned short d){phoneLine.wireRTCIn(d);});
	kb.subscribePhoneLine([&phoneLine](unsigned short d){phoneLine.wireKeyboardIn(d);});
	modem.subscribeATO([&phoneLine](unsigned short d){phoneLine.wireModemIn(d);});
	phoneLine.subscribeWireLine([&rtcn,&kb](unsigned short d){rtcn.phoneLineChangeIn(d);kb.phoneLineChangeIn(d);});
	phoneLine.subscribeWireModem([&modem](unsigned short d){modem.RA2ChangeIn(d);});
	
	smnap.subscribePrintFinished([p_mb_video](const char* p){
		char* pc=(char*)malloc(strlen(p)*sizeof(char)+1);
		strcpy(pc,p);
		thread_message ms_p_notif;
		ms_p_notif.cmd=PRINT_FINISHED;
		ms_p_notif.p=pc;
		p_mb_video->send(&ms_p_notif);
	});
	
	//debug
	
	modem.debug_cmd=[p_mb_video](unsigned char cmd){
		static unsigned char buzzer=0x3F;
		if (cmd>=0x38&&cmd<=0x3B&&!(buzzer>=0x38&&buzzer<=0x3B)){
			thread_message ms_p_notif;
			ms_p_notif.cmd=NOTIFICATION_BUZZER;
			p_mb_video->send(&ms_p_notif);
		}
		if ((cmd&0xF0)==0x30) buzzer=cmd;
	};

	auto dbgIOIN=[p_gState,p_mb_video](unsigned char a,unsigned char d){
		switch (a){
			case 0x20:
			case 0x21:
			case 0x22:
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
				//printf("To TS9347 A: 0x%02X / D: 0x%02X\n",a,d);
				break;
			case 0x40:
			case 0x41:
			case 0x42:
			case 0x43:
			case 0x44:
			case 0x45:
			{
				/*const char* type[]={"minute","hour","day","month","year low","year high"};
				printf("To CPLD RTC %s 0x%02X\n",type[a&0x0F],d);*/
				break;
			}
			case 0x50:
				//printf("To Keyboard 0x%02X\n",d);
				break;
			case 0x51:
				//printf("To CPLD Status 0x%02X\n",d);
				break;
			case 0x70:
			{
				/*static unsigned char io=0;
				if ((bool)((d^io)&(~(1<<3)))){//dont print when watchdog timer kicked
					io=d&(~(1<<3));
					printf("To CPLD Pin 0x%02X\n",io);
				}*/
				break;
			}
			default:
			{
				printf("To unknown IO A: 0x%02X / D: 0x%02X\n",a,d);
				//p_gState->stepByStep.store(true,std::memory_order_relaxed);
				
				thread_message ms_p_notif;
				ms_p_notif.cmd=NOTIFICATION_RED;
				char* buffer=(char*)calloc(51,sizeof(char));
				snprintf(buffer,50,"E/S inconnue (0x%02X)<=0x%02X",a,d);
				ms_p_notif.p=(void*)buffer;
				p_mb_video->send(&ms_p_notif);
				break;
			}
		}
	};
	iol.subscribeIN(dbgIOIN);
	
	auto dbgIOOUT=[p_gState,p_mb_video](unsigned char a,unsigned char d){
		switch (a){
			case 0x20:
			case 0x21:
			case 0x22:
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
				//printf("From TS9347 A: 0x%02X / D: 0x%02X\n",a,d);
				break;
			case 0x40:
			case 0x41:
			case 0x42:
			case 0x43:
			case 0x44:
			case 0x45:
			{
				/*const char* type[]={"minute","hour","day","month","year low","year high"};
				printf("From CPLD RTC %s 0x%02X\n",type[a&0x0F],d);*/
				break;
			}
			case 0x50:
			{
				/*static unsigned char data=0xFF;
				if (d!=data){//deduplicate reading of the same data
					data=d;
					printf("From Keyboard 0x%02X\n",d);
				}*/
				break;
			}
			case 0x51:
				//printf("From CPLD Status 0x%02X\n",d);
				break;
			case 0x70:
				//printf("From CPLD Pin A: 0x%02X / D: 0x%02X\n",a,d);
				break;
			default:
			{
				printf("From unknown IO A: 0x%02X / D: 0x%02X\n",a,d);
				//p_gState->stepByStep.store(true,std::memory_order_relaxed);
				
				thread_message ms_p_notif;
				ms_p_notif.cmd=NOTIFICATION_RED;
				char* buffer=(char*)calloc(51,sizeof(char));
				snprintf(buffer,50,"E/S inconnue (0x%02X)=>0x%02X",a,d);
				ms_p_notif.p=(void*)buffer;
				p_mb_video->send(&ms_p_notif);
				break;
			}
		}
	};
	iol.subscribeOUT(dbgIOOUT);
	
	/*stackMonitor sm=stackMonitor(&uc);
	sm.SPManualyModified=[p_gState,&uc](){
		printf("SP manualy modified c:%05lX\n",(((unsigned long)uc.PX_out[1]&3)<<16)|((unsigned long)(uc.PC-uc.i_length[uc.instruction[0]])));
		//p_gState->stepByStep.store(true,std::memory_order_relaxed);
	};
	sm.addressPOPed=[p_gState,&uc](bool high){
		printf("Address ");
		printf(high?"high":"low");
		printf(" poped c:%05lX\n",(((unsigned long)uc.PX_out[1]&3)<<16)|((unsigned long)(uc.PC-uc.i_length[uc.instruction[0]])));
		//p_gState->stepByStep.store(true,std::memory_order_relaxed);
	};
	sm.stackOverwrited=[p_gState,&uc](){
		printf("Stack overwrited c:%05lX\n",(((unsigned long)uc.PX_out[1]&3)<<16)|((unsigned long)(uc.PC-uc.i_length[uc.instruction[0]])));
		//p_gState->stepByStep.store(true,std::memory_order_relaxed);		
	};
	sm.funcReturned=[p_gState,&uc](bool ud,bool ind,bool old){
		if (ud){
			printf("Return to undefined c:%05lX\n",(((unsigned long)uc.PX_out[1]&3)<<16)|((unsigned long)(uc.PC-uc.i_length[uc.instruction[0]])));
			//p_gState->stepByStep.store(true,std::memory_order_relaxed);
		}
		else{
			if (ind||old){
				printf("Return to ");
				if (old) printf("already used ");
				if (ind) printf("indirect ");
				unsigned char sp=uc.getSFRByteIn(uc.SP);
				printf("address c:%05lX -> c:%05lX\n",(((unsigned long)uc.PX_out[1]&3)<<16)|((unsigned long)(uc.PC-uc.i_length[uc.instruction[0]])),(((unsigned long)uc.PX_out[1]&3)<<16)|(((unsigned short)uc.getRAMByte(sp))<<8)|((unsigned short)uc.getRAMByte(sp-1)));
			}
		}
	};*/
	
	//bool pause_emu=false;
#ifdef M12_USE_DECOMP_TOOLS
	uc.debug_signal_alu_before_exec=[p_gState,&CLKs,&rtd,&uc](){
		rtd.update();
#else
	uc.debug_signal_alu_before_exec=[p_gState,&CLKs,&uc](){
#endif
		CLKs.setPause(p_gState->stepByStep.load(std::memory_order_relaxed));
		if (p_gState->stepByStep.load(std::memory_order_relaxed)){
			print_m12_alu_instruction(&uc);
		}
	};
	//clock
	
	/*auto stopC=[p_gState](){
		return p_gState->shutdown.load(std::memory_order_relaxed);
	};
	CLKs.setStopCondition(stopC);
	auto pauseC=inline [&pause_emu](){
		return pause_emu;
	};
	CLKs.setPauseCondition(pauseC);*/
	
	//mailbox
	
	auto checkMB=[p_mb_circuit,&eram,&erom,&modem,&wt,&CLKs,&kb,p_gState,&rtcn,&rtcmp,&crtb](){
		//pause_emu=p_gState->stepByStep.load(std::memory_order_relaxed);
		CLKs.setPause(p_gState->stepByStep.load(std::memory_order_relaxed));
		thread_message ms;
		static unsigned char erom_cpy[EROM_SIZE];//static var to avoid reinitializing at each loop even if there is no messages (because gcc optimizations)
		static unsigned char eram_cpy[ERAM_SIZE];
		while (p_mb_circuit->receive(&ms)){
			switch(ms.cmd){
				case EMU_ON:
					//load rom/ram files
					readM(p_gState->p_thread_mutex,p_gState->erom,erom_cpy,EROM_SIZE);
					erom.set((unsigned char*)erom_cpy);
					readM(p_gState->p_thread_mutex,p_gState->eram,eram_cpy,ERAM_SIZE);
					eram.set((unsigned char*)eram_cpy);
					//reset
					wt.PWRChangeIn(true);
					break;
				case EMU_OFF:
					//wait RST signal before saving the ram
					modem.Reset();
					wt.PWRChangeIn(false);
					//TODO: reset keyboard
					break;
				case EMU_NEXT_STEP:
					//pause_emu=false;
					CLKs.setPause(false);
					break;
				case SPECIAL:
					rtcn.requestPhoneLine((RTCService*)&rtcmp);
					break;
				case EMU_SHUTDOWN:
					CLKs.setStop(true);
					break;
				default:
					fprintf(stdout,"unknown cmd %i\n",ms.cmd);
					break;
			}
		}
	};
	
	//clock wirring
	
	CLKs.subscribeMailbox(checkMB);
	auto CLKTick14745600=[&uc,&video,&modem](){
		uc.CLKTickIn();
		video.CLKTickIn();
		modem.CLKTickIn();//to resample ATxI input
	};
	CLKs.subscribe14745600Hz(std::cref(CLKTick14745600));
	auto CLKTick600=[&kb,&cpld,&rtcn](){
		cpld.CLKTickIn();
		kb.CLKTickIn();
		rtcn.CLKTickIn600Hz();
	};
	CLKs.subscribe600Hz(CLKTick600);
	auto CLKTick9600=[&smn,&wt,&rtcmp](){
		smn.CLKTickIn9600Hz();
		wt.incrementTimer();
		rtcmp.CLKTickIn9600Hz();
	};
	CLKs.subscribe9600Hz(CLKTick9600);
	
	auto CLKAudio=[&ab,&kb,&bzf,&spkf,&modem,&rtcn,&phoneLine](unsigned long sr){
		static unsigned long srp=0;
		if (sr!=srp){
			bzf.setSampleRate(sr);
			srp=sr;
		}
		
		phoneLine.setRTCSample(rtcn.getPhoneLineSample(sr));
		phoneLine.setKeyboardSample(kb.getPhoneLineSample(sr));
		phoneLine.setModemSample(modem.getTxOutSample());
		kb.setPhoneLineSample(phoneLine.getPhoneLineSample());
		
		ab.AudioIn(spkf.filter(kb.getSpeakerSample(sr))+bzf.filter(modem.getBuzzerSample(sr)));
	};
	CLKs.subscribeAudioSample(CLKAudio);
	
	
	
	
	
	
	
	
	
	
	
	//send pointers to GUI thread
	
	thread_message ms;
	
	ms.p=(void*)&eram;
	ms.cmd=ERAM;
	p_mb_video->send(&ms);
	
	ms.p=(void*)&erom;
	ms.cmd=EROM;
	p_mb_video->send(&ms);
	
	ms.p=(void*)&video;
	ms.cmd=VC;
	p_mb_video->send(&ms);
	
	ms.p=(void*)&uc;
	ms.cmd=UC;
	p_mb_video->send(&ms);
	
	ms.p=(void*)&cpld;
	ms.cmd=CPLD;
	p_mb_video->send(&ms);
	
	ms.p=(void*)&kb;
	ms.cmd=KEYBOARD;
	p_mb_video->send(&ms);
	
	ms.p=(void*)&modem;
	ms.cmd=MODEM;
	p_mb_video->send(&ms);
	
	ms.p=(void*)&CLKs;
	ms.cmd=CLOCK;
	p_mb_video->send(&ms);
	
	ms.p=(void*)&crtb;
	ms.cmd=CRT_BUFFER;
	p_mb_video->send(&ms);
	
	ms.p=(void*)&ab;
	ms.cmd=AUDIO_BUFFER;
	p_mb_video->send(&ms);
	
	ms.p=(void*)&spkf;
	ms.cmd=SPEAKER_FILTER;
	p_mb_video->send(&ms);
	
	ms.p=(void*)&bzf;
	ms.cmd=BUZZER_FILTER;
	p_mb_video->send(&ms);
	
	ms.p=(void*)&ab;
	ms.cmd=AUDIO_BUFFER;
	p_mb_video->send(&ms);
	
	ms.p=(void*)&smnap;
	ms.cmd=PRINTER;
	p_mb_video->send(&ms);
	
	ms.p=(void*)&smnalw;
	ms.cmd=WEBSOCKET_DIN;
	p_mb_video->send(&ms);
	
	ms.p=(void*)&smnas;
	ms.cmd=AUTO_START_MODULE;
	p_mb_video->send(&ms);
	
	
	
	ms.p=NULL;
	ms.cmd=EMULATOR_READY;
	p_mb_video->send(&ms);
	
	//start the emulation
	
	CLKs.start();
	
	//save ram if possible
	unsigned char eram_cpy[ERAM_SIZE];
	eram.copy((unsigned char*)eram_cpy);
	writeM(p_gState->p_thread_mutex,p_gState->eram,eram_cpy,ERAM_SIZE);
	
	//signal emulation end
	p_gState->minitelOn.store(false,std::memory_order_relaxed);
}