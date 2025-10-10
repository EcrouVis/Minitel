#ifndef ROM_256K_H
#define ROM_256K_H
class ROM_256k{
	public:
		ROM_256k(FILE* fp);
		void ALChangeIn(unsigned char a);
		void AHChangeIn(unsigned char a);
		void A16ChangeIn(bool b);
		void A17ChangeIn(bool b);
		void nGChangeIn(bool b);
		void subscribeD(void (*f)(unsigned char));

	private:
		long address;
		unsigned char data;
		unsigned char eROM[16777216];
		bool nG=true;
		void (*sendD)(unsigned char);
		void updateState();
};
#endif