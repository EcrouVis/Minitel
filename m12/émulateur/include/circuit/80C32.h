#ifndef M80C32_H
#define M80C32_H
#include <atomic>
#include <functional>

const int IRAM_SIZE=256;
const int SFR_SIZE=128;

class m80C32{
	public:
		bool exec_instruction=false;
		
		//address
		enum Constants{
			ACC=0xE0,
			B=0xF0,
			DPH=0x83,
			DPL=0x82,
			IE=0xA8,
			IP=0xB8,
			P0=0x80,
			P1=0x90,
			P2=0xA0,
			P3=0xB0,
			PCON=0x87,
			PSW=0xD0,
			RCAP2H=0xCB,
			RCAP2L=0xCA,
			SBUF=0x99,
			SCON=0x98,
			SP=0x81,
			TCON=0x88,
			T2CON=0xC8,
			TH0=0x8C,
			TH1=0x8D,
			TH2=0xCD,
			TL0=0x8A,
			TL1=0x8B,
			TL2=0xCC,
			TMOD=0x89
		};
	
		std::atomic_uchar iRAM[IRAM_SIZE];
		std::atomic_uint last_memory_operation;
		std::atomic_uchar SFR[SFR_SIZE];
		unsigned char PX_out[4];
		unsigned char SBUF_out;
		unsigned char SBUF_in;//buffer
		bool TEN=false;
		
		m80C32();
		
		static void bitaddress2address(unsigned char*,unsigned char*);
		
		static unsigned char getBitMask(unsigned char);
		static unsigned char getBitDirectAddress(unsigned char);
		
		bool getBitIn(unsigned char);
		bool getBitOut(unsigned char);
		//change state + callback for PX port change
		void setBitIn(unsigned char,bool);
		void setBitOut(unsigned char,bool);
		
		//unsigned char getCharIn(unsigned char);
		unsigned char getRAMByte(unsigned char);
		unsigned char getSFRByteIn(unsigned char);
		unsigned char getDirectByteIn(unsigned char);
		//for read-modify-write instruction
		//unsigned char getCharOut(unsigned char);
		unsigned char getSFRByteOut(unsigned char);
		unsigned char getDirectByteOut(unsigned char);
		//change state + callback for PX port change
		//void setChar(unsigned char,unsigned char);
		void setRAMByte(unsigned char,unsigned char);
		void setSFRByte(unsigned char,unsigned char);
		void setDirectByte(unsigned char,unsigned char);
		
		unsigned char getR(unsigned char);
		
		void CLKTickIn();
		void ResetChangeIn(bool);
		void Reset();
		void PXChangeIn(unsigned char,unsigned char);
		void PXYChangeIn(unsigned char,unsigned char,bool);
		/*void subscribeP0(void (*f)(unsigned char)){this->sendP0=f;}
		void subscribeP1(void (*f)(unsigned char)){this->sendP1=f;}
		void subscribeP2(void (*f)(unsigned char)){this->sendP2=f;}
		void subscribeP3(void (*f)(unsigned char)){this->sendP3=f;}
		void subscribenRD(void (*f)(bool)){this->sendnRD=f;}
		void subscribenWR(void (*f)(bool)){this->sendnWR=f;}
		void subscribeTxD(void (*f)(bool)){this->sendTxD=f;}
		void subscribeRxD(void (*f)(bool)){this->sendRxD=f;}
		void subscribeALE(void (*f)(bool)){this->sendALE=f;}
		void subscribenPSEN(void (*f)(bool)){this->sendnPSEN=f;}*/
		void subscribeP0(std::function<void(unsigned char)> f){this->sendP0=f;}
		void subscribeP1(std::function<void(unsigned char)> f){this->sendP1=f;}
		void subscribeP2(std::function<void(unsigned char)> f){this->sendP2=f;}
		void subscribeP3(std::function<void(unsigned char)> f){this->sendP3=f;}
		void subscribeTxD(std::function<void(bool)> f){this->sendTxD=f;}
		void subscribeRxD(std::function<void(bool)> f){this->sendRxD=f;}
		void subscribeALE(std::function<void(bool)> f){this->sendALE=f;}
		void subscribenPSEN(std::function<void(bool)> f){this->sendnPSEN=f;}
		
		std::function<void(void)> debug_signal_alu_before_exec=[](){};
		std::function<void(void)> debug_signal_alu_after_exec=[](){};
	
		const unsigned char periodPerCycle=12;
		
		//bit address
		const unsigned char EA=0xAF;
		const unsigned char ET2=0xAD;
		const unsigned char ES=0xAC;
		const unsigned char ET1=0xAB;
		const unsigned char EX1=0xAA;
		const unsigned char ET0=0xA9;
		const unsigned char EX0=0xA8;
		const unsigned char PT2=0xBD;
		const unsigned char PS=0xBC;
		const unsigned char PT1=0xBB;
		const unsigned char PX1=0xBA;
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
		
		
		unsigned char period=0;
			
		unsigned short PC=0;
		
		unsigned char instruction[3]={0,0,0};
		unsigned char i_cycle_n=0xFF;
		unsigned char i_part_n;
		
		
		bool reset_level=true;
		unsigned char reset_count=0;
		
		
		unsigned char TX_bit=0;
		unsigned char RX_bit=0;
		bool RX_state=true;
		
		unsigned char tx_prescaler=0;
		unsigned char rx_prescaler=0;
		unsigned char tx_subtick=0;
		unsigned char rx_subtick=0;
		
	private:
		
		void PXChange(unsigned char,unsigned char);
		
		void ACALL(unsigned short);
		void ADD_A(unsigned char);
		void ADDC_A(unsigned char);
		void AJMP(unsigned short);
		static unsigned char ANL(unsigned char,unsigned char);
		void ANL_C(bool);
		void CJNE(unsigned char,unsigned char,signed char);
		void CLR_A();
		static unsigned char CLR_bit(unsigned char,unsigned char);
		void CPL_A();
		static unsigned char CPL_bit(unsigned char,unsigned char);
		void DA();
		static unsigned char DEC(unsigned char);
		void DIV();
		unsigned char DJNZ(unsigned char,signed char);
		static unsigned char INC(unsigned char);
		void INC_DPTR();
		void JB(unsigned char,unsigned char,signed char);
		unsigned char JBC(unsigned char,unsigned char,signed char);
		void JC(signed char);
		void JMP();
		void JNB(unsigned char,unsigned char,signed char);
		void JNC(signed char);
		void JNZ(signed char);
		void JZ(signed char);
		void LCALL(unsigned short);
		void LJMP(unsigned short);
		void MOVC(unsigned short);
		void MOVX_I(unsigned char);
		void MOVX_I(unsigned short);
		void MOVX_O(unsigned char);
		void MOVX_O(unsigned short);
		void MUL();
		static unsigned char ORL(unsigned char,unsigned char);
		void ORL_C(bool);
		unsigned char POP();
		void PUSH(unsigned char);
		void RET();
		void RETI();
		void RL();
		void RLC();
		void RR();
		void RRC();
		void SJMP(signed char);
		void SUBB(unsigned char);
		void SWAP();
		unsigned char XCH(unsigned char);
		unsigned char XCHD(unsigned char);
		static unsigned char XRL(unsigned char,unsigned char);
		
		void nextCycleALU();
		void execInstruction();
		void setACCParity();
		
		
		unsigned char interrupt_level=0;
		bool interrupt_change=false;
		
		void checkInterrupts();
		void decreaseInterruptLevel();
		
		void ResetCountdown();
		void Idle();
		void PowerDown();
		void PCONChange();
		
		std::function<void(unsigned char)> sendP0=[](unsigned char d){};
		std::function<void(unsigned char)> sendP1=[](unsigned char d){};
		std::function<void(unsigned char)> sendP2=[](unsigned char d){};
		std::function<void(unsigned char)> sendP3=[](unsigned char d){};
		std::function<void(bool)> sendnRD=[](bool d){};
		std::function<void(bool)> sendnWR=[](bool d){};
		std::function<void(bool)> sendTxD=[](bool d){};
		std::function<void(bool)> sendRxD=[](bool d){};
		std::function<void(bool)> sendALE=[](bool d){};
		std::function<void(bool)> sendnPSEN=[](bool d){};
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
		
};

void print_m12_alu_instruction(m80C32* uc);

#endif