#ifndef M80C32_H
#define M80C32_H
#include <atomic>
#include <functional>

const int IRAM_SIZE=256;

class m80C32{
	public:
		bool exec_instruction=false;
	
		std::atomic_uchar iRAM[IRAM_SIZE];
		std::atomic_uint last_memory_operation;
		unsigned char SFR[128];
		unsigned char PX_out[4];
		unsigned char SBUF_out;
		unsigned char SBUF_in;//buffer
		bool TEN=false;
		
		m80C32();
		
		void CLKTickIn();
		void ResetChangeIn(bool);
		void Reset();
		void PXChangeIn(unsigned char,unsigned char);
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
		
		
	private:
	
		const unsigned char periodPerCycle=12;
		
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
		/*const unsigned char ACC=0xE0;
		const unsigned char B=0xF0;
		const unsigned char DPH=0x83;
		const unsigned char DPL=0x82;
		const unsigned char IE=0xA8;
		const unsigned char IP=0xB8;
		const unsigned char P0=0x80;
		const unsigned char P1=0x90;
		const unsigned char P2=0xA0;
		const unsigned char P3=0xB0;
		const unsigned char PCON=0x87;
		const unsigned char PSW=0xD0;
		const unsigned char RCAP2H=0xCB;
		const unsigned char RCAP2L=0xCA;
		const unsigned char SBUF=0x99;
		const unsigned char SCON=0x98;
		const unsigned char SP=0x81;
		const unsigned char TCON=0x88;
		const unsigned char T2CON=0xC8;
		const unsigned char TH0=0x8C;
		const unsigned char TH1=0x8D;
		const unsigned char TH2=0xCD;
		const unsigned char TL0=0x8A;
		const unsigned char TL1=0x8B;
		const unsigned char TL2=0xCC;
		const unsigned char TMOD=0x89;*/
		
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
		
		void bitaddress2address(unsigned char*,unsigned char*);
		
		bool getBitIn(unsigned char);
		//change state + callback for PX port change
		void setBitIn(unsigned char,bool);
		
		//unsigned char getCharIn(unsigned char);
		unsigned char getRAMByte(unsigned char);
		unsigned char getSFRByteIn(unsigned char);
		//for read-modify-write instruction
		//unsigned char getCharOut(unsigned char);
		unsigned char getSFRByteOut(unsigned char);
		//change state + callback for PX port change
		//void setChar(unsigned char,unsigned char);
		void setRAMByte(unsigned char,unsigned char);
		void setSFRByte(unsigned char,unsigned char);
		
		unsigned char getR(unsigned char);
		
		void PXChange(unsigned char,unsigned char);
		
		unsigned char period=0;
			
		unsigned short PC=0;
		
		unsigned char instruction[3]={0,0,0};
		unsigned char i_cycle_n=0xFF;
		unsigned char i_part_n;
		
		void ACALL();
		void AJMP();
		void ADD(unsigned char);
		void ADDC(unsigned char);
		void ANL(unsigned char,unsigned char);
		void ANLcy(bool);
		void CJNE(unsigned char,unsigned char);
		void CLRa();
		void CLRb(unsigned char);
		void CPLa();
		void CPLb(unsigned char);
		void DA();
		void DEC(unsigned char);
		void DECd(unsigned char);
		void DIV();
		void DJNZ(unsigned char);
		void DJNZd(unsigned char);
		void INC(unsigned char);
		void INCd(unsigned char);
		void INCdptr();
		void JB();
		void JBC();
		void JC();
		void JMP();
		void JNB();
		void JNC();
		void JNZ();
		void JZ();
		void LCALL();
		void LJMP();
		void MOV(unsigned char,unsigned char);
		void MOVd(unsigned char,unsigned char);
		void MOVdptr();
		void MOVb(unsigned char,unsigned char);
		void MOVC(unsigned short);
		void MOVXin(unsigned char);
		void MOVXin(unsigned short);
		void MOVXout(unsigned char);
		void MOVXout(unsigned short);
		void MUL();
		void ORL(unsigned char,unsigned char);
		void ORLcy(bool);
		void POP();
		void PUSH();
		void RET();
		void RETI();
		void RL();
		void RLC();
		void RR();
		void RRC();
		void SETB(unsigned char);
		void SJMP();
		void SUBB(unsigned char);
		void SWAP();
		void XCH(unsigned char);
		void XCHd(unsigned char);
		void XCHD(unsigned char);
		void XRL(unsigned char,unsigned char);
		
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
		
		bool reset_level=true;
		unsigned char reset_count=0;
		
		/*void (*sendP0)(unsigned char)=[](unsigned char d){};
		void (*sendP1)(unsigned char)=[](unsigned char d){};
		void (*sendP2)(unsigned char)=[](unsigned char d){};
		void (*sendP3)(unsigned char)=[](unsigned char d){};
		void (*sendnRD)(bool)=[](bool b){};
		void (*sendnWR)(bool)=[](bool b){};
		void (*sendTxD)(bool)=[](bool b){};
		void (*sendRxD)(bool)=[](bool b){};
		void (*sendALE)(bool)=[](bool b){};
		void (*sendnPSEN)(bool)=[](bool b){};*/
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
		
		unsigned char TX_bit=0;
		unsigned char RX_bit=0;
		bool RX_state=true;
		
		unsigned char tx_prescaler=0;
		unsigned char rx_prescaler=0;
		unsigned char tx_subtick=0;
		unsigned char rx_subtick=0;
};
#endif