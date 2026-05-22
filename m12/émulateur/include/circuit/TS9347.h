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
		unsigned char getYFromRow(unsigned char Y);
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




//inline member functions / called only in one place inside the hot path

__attribute__((always_inline)) inline void TS9347wVRAM::CLKTickIn(){
	this->main_clk_div++;
	if (this->main_clk_div<6) return;
	this->main_clk_div=0;
	
	this->clk_frame++;
	if (this->clk_frame>=39936){//312*(40+24)*2
		this->clk_frame=0;
		this->n_frame++;
		if (this->n_frame>=50) {
			this->n_frame=0;
		}
	}
	unsigned short n_line=this->clk_frame>>7;
	unsigned char pos_line=(this->clk_frame>>1)&0x3F;
	
	if (this->clk_frame==0x0A80){
		if (!this->vsync_mask) this->STATUS.fetch_or(this->VSYNC_MASK,std::memory_order_relaxed);
		this->sendVideo(this->VIDEO_OUTPUT);
	}
	if (this->clk_frame==0x0B80) this->STATUS.fetch_and(~this->VSYNC_MASK,std::memory_order_relaxed);
	
	if (this->late_cmd_end){
		this->late_cmd_end=false;
		this->STATUS.fetch_and((unsigned char)~this->BUSY_MASK,std::memory_order_relaxed);//before cmd exec -> delay 0.5T
	}
	
	if (n_line>=62){
		switch ((unsigned char)(n_line-62)){
			case 9:
			case 19:
			case 29:
			case 39:
			case 49:
			case 59:
			case 69:
			case 79:
			case 89:
			case 99:
			case 109:
			case 119:
			case 129:
			case 139:
			case 149:
			case 159:
			case 169:
			case 179:
			case 189:
			case 199:
			case 209:
			case 219:
			case 229:
			case 239:
			case 249:
				if (pos_line<40){
					//UDS LD
					if((bool)(this->clk_frame&1)){
						this->loadRowBuffer();
						if (this->Rx[0].load(std::memory_order_relaxed)==0x95||this->Rx[0].load(std::memory_order_relaxed)==0x99) this->executeCommand();//VSM/VRM
					}
					else this->loadUDS();
				}
				else{
					//LD LD
					this->loadRowBuffer();
					if((bool)(this->clk_frame&1)){
						if (this->Rx[0].load(std::memory_order_relaxed)==0x95||this->Rx[0].load(std::memory_order_relaxed)==0x99) this->executeCommand();//VSM/VRM
					}
				}
				break;
			case 0:
			case 10:
			case 20:
			case 30:
			case 40:
			case 50:
			case 60:
			case 70:
			case 80:
			case 90:
			case 100:
			case 110:
			case 120:
			case 130:
			case 140:
			case 150:
			case 160:
			case 170:
			case 180:
			case 190:
			case 200:
			case 210:
			case 220:
			case 230:
			case 240:
				if (pos_line<40){
					//UDS LD
					if((bool)(this->clk_frame&1)){
						this->loadRowBuffer();
						if (this->Rx[0].load(std::memory_order_relaxed)==0x95||this->Rx[0].load(std::memory_order_relaxed)==0x99) this->executeCommand();//VSM/VRM
					}
					else this->loadUDS();
				}
				else{
					//uP
					if((bool)(this->clk_frame&1)) this->executeCommand();
				}
				break;
			default:
				if((bool)(this->clk_frame&1)) this->executeCommand();
				else if (pos_line<40){
					this->loadUDS();
				}
				break;
		}
	}
	else{
		//uP
		if((bool)(this->clk_frame&1)) this->executeCommand();
	}
}



#endif