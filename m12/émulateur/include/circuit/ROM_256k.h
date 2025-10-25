#ifndef ROM_256K_H
#define ROM_256K_H
#include <atomic>
#include <functional>

const int EROM_SIZE=262144;

class ROM_256k{
	public:
		void ALChangeIn(unsigned char a);
		void AHChangeIn(unsigned char a);
		void A16ChangeIn(bool b);
		void A17ChangeIn(bool b);
		void nGChangeIn(bool b);
		//void subscribeD(void (*f)(unsigned char));
		void subscribeD(std::function<void(unsigned char)>);
		
		std::atomic_uchar eROM[EROM_SIZE];
		std::atomic_uint last_memory_operation;
		
		void set(unsigned char* array);

	private:
		long address;
		unsigned char data;
		bool nG=true;
		//void (*sendD)(unsigned char);
		std::function<void(unsigned char)> sendD;
		void updateState();
};
#endif