#ifndef TS9347_H
#define TS9347_H
#include <atomic>
#include <functional>
#include <cstdio>
const int VRAM_SIZE=32768;

//not accurate implementation (timming/behavior)
class TS9347wVRAM{
	public:
		void DChangeIn(unsigned char d);
		void ASChangeIn(bool b);
		void DSChangeIn(bool b);
		void nCSChangeIn(bool b);
		void RnWChangeIn(bool b);
		void subscribeD(std::function<void(unsigned char)>);
		
		std::atomic_uchar VRAM[VRAM_SIZE];
		void set(unsigned char* array);
	private:
		std::function<void(unsigned char)> sendD;
		std::atomic_uint ADDR_BUF=0;
		unsigned char D;//address/data 8-bit bus
		bool AS;//address latch enable
		bool DS;//read low
		bool RnW;//write low
		bool nCS;//chip select
		
		const unsigned char BUSY_MASK=0b10000000;
		const unsigned char Al_MASK=0b01000000;
		const unsigned char LXm_MASK=0b00100000;
		const unsigned char LXa_MASK=0b00010000;
		const unsigned char VSYNC_MASK=0b00000100;
		const unsigned char RESET_MASK=0b01111000;
		
		const unsigned char RnW_MASK=0b00001000;
		const unsigned char INC_MASK=0b00000001;
		
		bool VS_MASK_FLAG=false;
		
		std::atomic_uchar STATUS=0;
		std::atomic_uchar Rx[8]={0,0,0,0,0,0,0,0};//COMMAND,R1,R2,..,R7
		std::atomic_uchar DOR=0;
		std::atomic_uchar PAT=0;
		std::atomic_uchar MAT=0;
		std::atomic_uchar TGS=0;
		std::atomic_uchar ROR=0;
		
		std::atomic_uchar outside;
		
		bool isSelected();
		bool isBusy();
		bool requestExecution();
		bool isCMDCorrect(unsigned char);
		void executeCommand();
		
		void IND(unsigned char r,bool RnW);
		void VSM();
		void VRM();
		void INY();
		void TBMA(bool MnA,bool RnW,bool inc);
		void TLSMA(bool MnA,bool RnW,bool inc,bool long_code);
		void KRLS(bool RnW,bool inc,bool long_code);
		void MV(unsigned char n_buf,bool M2A,bool stop);
	
};
#endif