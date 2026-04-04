#ifndef DIN5INTERFACEBASE_H
#define DIN5INTERFACEBASE_H

#include <functional>
#include "cJSON/cJSON.h"
#include <atomic>
#include <queue>
#include <vector>
#include <algorithm>

class DIN5InterfaceBase{
	public:
	
		std::atomic_bool plugged=false;//read if connected to the minitel
		
		virtual ~DIN5InterfaceBase(){}
		
		void RxChangeIn(bool b){
			if (b!=this->wireRx){
				this->wireRx=b;
				if (this->plugged.load(std::memory_order_acquire)) this->_RxChangeIn(b);
			}
		}
		void PTChangeIn(bool b){//don't call sendPT when reacting to PTChangeIn to avoid PT state desync between IC
			if (b!=this->wirePT){
				this->wirePT=b;
				if (this->plugged.load(std::memory_order_acquire)) this->_PTChangeIn(b);
			}
		}
		void PWRChangeIn(bool b){
			if (b!=this->wirePWR){
				this->wirePWR=b;
				if (this->plugged.load(std::memory_order_acquire)) this->_PWRChangeIn(b);
			}
		}
		void CLKTickIn9600Hz(){
			bool p=this->plugged.load(std::memory_order_acquire);
			if (p!=this->last_plugged){
				this->last_plugged=p;
				if (p){
					this->sendTx=this->sendWireTx;
					this->sendPT=this->sendWirePT;
					this->_PTChangeIn(this->wirePT);
					this->_RxChangeIn(this->wireRx);
					this->_PWRChangeIn(this->wirePWR);
				}
				else{
					this->_PWRChangeIn(false);
					this->_RxChangeIn(false);
					this->_PTChangeIn(false);
					this->sendTx=[](bool b){};
					this->sendPT=[](bool b){};
					this->sendWireTx(true);
					this->sendWirePT(true);
				}
			}
			if (p) this->_CLKTickIn9600Hz();
		}
		
		void subscribeTx(std::function<void(bool)> f){
			this->sendWireTx=f;
			if (this->plugged.load(std::memory_order_acquire)) this->sendTx=this->sendWireTx;
		}
		void subscribePT(std::function<void(bool)> f){
			this->sendWirePT=f;
			if (this->plugged.load(std::memory_order_acquire)) this->sendPT=this->sendWirePT;
		}
		
	private:
		bool last_plugged=false;
		bool wirePWR=false;
		bool wirePT=false;
		bool wireRx=false;
		std::function<void(bool)> sendWireTx=[](bool b){};
		std::function<void(bool)> sendWirePT=[](bool b){};
		
	protected:
		std::function<void(bool)> sendTx=[](bool b){};//change the state of the Tx wire
		std::function<void(bool)> sendPT=[](bool b){};//change the state of the PT wire
		
		//custom logic here
		virtual void _RxChangeIn(bool b){};//called when the Rx wire is updated / it is possible that the value didn't change
		virtual void _PTChangeIn(bool b){};//called when the PT wire is updated / it is possible that the value didn't change
		virtual void _PWRChangeIn(bool b){};//called when the PWR wire is updated / it is possible that the value didn't change
		virtual void _CLKTickIn9600Hz(){};//called when 1/9600s ellapsed in the simulation and it is connected to the minitel
};
#endif