#ifndef L6720_H
#define L6720_H
#include <functional>
class L6720{
	public:
		void PTSChangeIn(bool b){
			this->PTS=b;
			b=b&&this->PT_in;
			if (b!=this->PT_out){
				this->PT_out=b;
				this->sendPT(b);
				this->sendPTE(!b);
			}
		}
		void PTChangeIn(bool b){
			this->PT_in=b;
			b=b&&this->PTS;
			if (b!=this->PT_out){
				this->PT_out=b;
				this->sendPT(b);
				this->sendPTE(!b);
			}
		}
		void subscribePT(std::function<void(bool)> f){
			this->sendPT=f;
		}
		void subscribePTE(std::function<void(bool)> f){
			this->sendPTE=f;
		}
	private:
		bool PT_in=false;
		bool PT_out=false;
		bool PTE=false;
		bool PTS=false;
		std::function<void(bool)> sendPT=[](bool b){};
		std::function<void(bool)> sendPTE=[](bool b){};
};
#endif