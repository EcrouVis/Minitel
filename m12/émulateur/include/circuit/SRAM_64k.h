#ifndef SRAM_64K_H
#define SRAM_64K_H
#include <atomic>
#include <functional>

const int ERAM_SIZE=65536;
class SRAM_64k{
	public:
		void ALChangeIn(unsigned char a);
		void AHChangeIn(unsigned char a);
		void DChangeIn(unsigned char d);
		void nOEChangeIn(bool b);
		void nWEChangeIn(bool b);
		void nCSChangeIn(bool b);
		//void subscribeD(void (*f)(unsigned char));
		void subscribeD(std::function<void(unsigned char)>);
		
		void copy(unsigned char* array);
		void set(unsigned char* array);
		
		std::atomic_uchar RAM[ERAM_SIZE];
		std::atomic_uint last_memory_operation;
		
		
	private:
		unsigned short address;
		unsigned char data;
		bool nOE=true;
		bool nWE=true;
		bool nCS=true;
		std::function<void(unsigned char)> sendD;
		//void (*sendD)(unsigned char);
		bool isInput();
		bool isOutput();
		void updateState();
};
#endif