#ifndef SRAM_64K_H
#define SRAM_64K_H
class SRAM_64k{
	public:
		void ALChangeIn(unsigned char a);
		void AHChangeIn(unsigned char a);
		void DChangeIn(unsigned char d);
		void nOEChangeIn(bool b);
		void nWEChangeIn(bool b);
		void nCSChangeIn(bool b);
		void subscribeD(void (*f)(unsigned char));
		
	private:
		unsigned char eRAM[65536];
		unsigned short address;
		unsigned char data;
		bool nOE=true;
		bool nWE=true;
		bool nCS=true;
		void (*sendD)(unsigned char);
		bool isInput();
		bool isOutput();
		void updateState();
};
#endif