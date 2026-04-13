#ifndef TS9347_H
#define TS9347_H
#include <atomic>
#include <functional>
#include <cstdio>
const int VRAM_SIZE=32768;
const int ROM_CHARSET_SIZE=2880;
const int VIDEO_FRAME_SIZE=(250+2)*(3*40*8+2)/2;

//not accurate implementation (timming/behavior)
class TS9347wVRAM{
	public:
		void DChangeIn(unsigned char d);
		void nCSChangeIn(bool b);
		void ASChangeIn(bool b);
		void DSChangeIn(bool b);
		void RnWChangeIn(bool b);
		void CLKTickIn();
		void subscribeD(std::function<void(unsigned char)> f);
		void subscribeVideo(std::function<void(unsigned char*)> f);
		
		void setROMCharset(const unsigned char* cs);
		
		std::atomic_uchar STATUS=0;
		std::atomic_uchar Rx[8]={0,0,0,0,0,0,0,0};//COMMAND,R1,R2,..,R7
		std::atomic_uchar DOR=0;
		std::atomic_uchar PAT=0;
		std::atomic_uchar MAT=0;
		std::atomic_uchar TGS=0;
		std::atomic_uchar ROR=0;
		
		std::atomic_uchar VRAM[VRAM_SIZE];
		std::atomic_uchar ROW_BUFFER[128];
		unsigned char VIDEO_OUTPUT[VIDEO_FRAME_SIZE];//format byte: ABGR ABGR
	private:
		unsigned char main_clk_div=0;
		unsigned short clk_frame=0;
		unsigned char n_frame=0;
		unsigned int cmd_step=0;
		bool late_cmd_end=false;
		bool vsync_mask=false;
	
		std::function<void(unsigned char)> sendD=[](unsigned char d){};
		std::function<void(unsigned char*)> sendVideo=[](unsigned char* v){};
		std::atomic_uint ADDR_BUF=0;
		unsigned char D;//address/data 8-bit bus
		bool AS;//address latch enable
		bool DS;//read low
		bool RnW;//write low
		bool nCS;//chip select
		
		constexpr static unsigned char BUSY_MASK=0b10000000;
		constexpr static unsigned char Al_MASK=0b01000000;
		constexpr static unsigned char LXm_MASK=0b00100000;
		constexpr static unsigned char LXa_MASK=0b00010000;
		constexpr static unsigned char VSYNC_MASK=0b00000100;
		constexpr static unsigned char RESET_MASK=0b01111000;
		
		constexpr static unsigned char RnW_MASK=0b00001000;
		constexpr static unsigned char INC_MASK=0b00000001;
		
		bool VS_MASK_FLAG=false;
		
		std::atomic<unsigned char> ROM_CHARSET[ROM_CHARSET_SIZE]={0};
		
		bool shift_slice=false;//double length
		
		inline unsigned char getMarginABGR();
		static inline int getVideoIndex(const unsigned int line, const unsigned char column, bool mode40=true);
		void setVideoOtputABGR(int index,unsigned char abgr);
		static int getSliceRAMIndex(unsigned char slice, unsigned char C, unsigned char Z);
		unsigned char getRowFromY(unsigned char Y);
		inline unsigned char getDisplayDistrict();
		void loadRowBuffer();
		void loadUDS();
		
		bool isICSelected();
		bool isBusy();
		bool requestExecution();
		bool incrementX(bool MP);
		inline unsigned char getX(bool MP);
		void incrementY(bool MP);
		inline unsigned char getY(bool MP);
		void incrementB(bool MP);
		void addB(bool MP,unsigned char d);
		inline unsigned char getB(bool MP);
		inline unsigned char getD(bool MP);
		int pointer2RAMAddress(bool MP);
		void executeCommand();
		void NOP();
		void VRM();
		void VSM();
		void INY();
		void IND();
		void MVB();
		void MVD();
		void MVT();
		void TBM();
		void TBA();
		void KRS();
		void KRL();
		void TSM();
		void TSA();
		void CLS();
		void TLM();
		void TLA();
		void CLL();
};
#endif