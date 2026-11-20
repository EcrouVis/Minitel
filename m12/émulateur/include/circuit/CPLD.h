#ifndef CPLD_H
#define CPLD_H
#include <functional>
#include <cstdio>
#include <ctime>
#include <atomic>

class MBSL_4000FH5_5{//work in progress - logic circuit deduced with the 80C32 behavior
	public:
		std::atomic_bool OS_RTC=false;
		
		void DChangeIn(unsigned char d);
		void ALEChangeIn(bool b);
		void nOEChangeIn(bool b);
		void nWEChangeIn(bool b);
		void nCSChangeIn(bool b);
		void WATCHDOGChangeIn(bool b);
		void serialChangeIn(bool b);
		
		void CLKTickIn9600Hz();
		
		void subscribenCSRAM(std::function<void(bool)> f);
		void subscribeAL(std::function<void(unsigned char)> f);
		void subscribeD(std::function<void(unsigned char)> f);
		void subscribePIO(std::function<void(unsigned char)> f);
		void subscribeRST(std::function<void(bool)> f);
		void subscribeSerial(std::function<void(bool)> f);
		
	private:
		bool nCS=true;
		bool nOE=true;
		bool nWE=true;
		bool ALE=true;
		bool WATCHDOG=true;
		bool RST_out=true;
		unsigned short RST_div=0;
		
		unsigned char S_in_step=0;
		unsigned char S_out_step=0;
		unsigned char S_in_div=0;
		unsigned char S_out_div=0;
		bool S_in=false;
		
		unsigned char address=0;
		unsigned char data=0;
		std::function<void(bool)> sendnCSRAM=[](bool b){};
		std::function<void(unsigned char)> sendAL=[](unsigned char d){};
		std::function<void(unsigned char)> sendD=[](unsigned char d){};
		std::function<void(unsigned char)> sendPIO=[](unsigned char d){};
		std::function<void(bool)> sendRST=[](bool b){};
		std::function<void(bool)> sendSerial=[](bool b){};
		
		unsigned char STATUS=0x80;
		unsigned char SCON=0;
		unsigned char IO=0x80;//0x70
		unsigned char SBUF_in;
		unsigned char SBUF_in_tmp;
		unsigned char SBUF_out;
		int diff_time=0;
		unsigned char old_mins=0;
		
		static unsigned char D2BCD(unsigned char D);
		static unsigned char BCD2D(unsigned char BCD);
		
		void UC2CPLD();
		void CPLD2UC();
		
		void updateRx();
		void updateTx();
		void checkRTCChange();
};
#endif