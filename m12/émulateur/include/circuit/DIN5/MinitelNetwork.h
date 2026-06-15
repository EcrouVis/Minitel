#ifndef MINITEL_MODULENETWORK_H
#define MINITEL_MODULENETWORK_H
#include <cstdio>
#include <queue>
#include <mutex>
#include <atomic>
#include <ixwebsocket/IXWebSocket.h>
#include "circuit/DIN5/vdt.h"
#include "encoding.h"

class SimplifiedMinitelNetworkApp{
	public:
		virtual void CLKCallback(){}
		virtual void RxCallback(unsigned char d)=0;
		virtual void RxWaitCallback(){};//to send data by packets
		virtual void TxQueueEmptyCallback(){}
		virtual void PTCallback(bool b)=0;
		virtual void PWRCallback(bool b){};
		bool getPTout(){
			return this->PTout;
		}
		virtual bool TxEmpty(){return true;}
		virtual unsigned char TxPop(){return 0;}
	protected:
		bool PTout=true;
};
class SimplifiedMinitelNetwork{// ! slave process not implemented
	public:
		void RxChangeIn(bool b){
			if (b!=this->Rx){
				this->Rx=b;
			}
		}
		void PTChangeIn(bool b){//don't call sendPT when reacting to PTChangeIn to avoid PT state desync between IC
			if (b!=this->PT){
				this->PT=b;
				for (SimplifiedMinitelNetworkApp* app: this->Apps){
					app->PTCallback(b);
				}
				if (b) this->baudrate_div=BPS_1200;
			}
		}
		void PWRChangeIn(bool b){
			if (b!=this->PWR){
				this->PWR=b;
				for (SimplifiedMinitelNetworkApp* app: this->Apps){
					app->PWRCallback(b);
				}
				this->sendPT(true);
			}
		}
		void CLKTickIn9600Hz(){
			this->serial_clk++;
			if (this->serial_clk>=this->baudrate_div){
				this->serial_clk=0;
				this->SerialClockTick();
			}
			for (SimplifiedMinitelNetworkApp* app: this->Apps){
				app->CLKCallback();
			}
			if (this->AppSelected==NULL){
				for (SimplifiedMinitelNetworkApp* app: this->Apps){
					if(!app->getPTout()){
						this->AppSelected=app;
						this->sendPT(false);
						break;
					}
				}
			}
			
			if (this->AppSelected!=NULL&&this->AppSelected->getPTout()){
				this->AppSelected=NULL;
				for (SimplifiedMinitelNetworkApp* app: this->Apps){
					if(!app->getPTout()){
						this->AppSelected=app;
						break;
					}
				}
				if (this->AppSelected==NULL) this->sendPT(true);
			}
		}
		void subscribeTx(std::function<void(bool)> f){
			this->sendTx=f;
		}
		void subscribePT(std::function<void(bool)> f){
			this->sendPT=f;
		}
		void registerApp(SimplifiedMinitelNetworkApp* app){
			this->Apps.push_back(app);
		}
		
	private:
		std::vector<SimplifiedMinitelNetworkApp*> Apps;
		bool Rx=false;
		bool Rx_prev=false;
		unsigned char Rx_buf=0;
		unsigned char Tx_buf=0;
		SimplifiedMinitelNetworkApp* AppSelected=NULL;
		
		bool PT=false;
		bool PWR=false;
		std::function<void(bool)> sendPT=[](bool b){};
		std::function<void(bool)> sendTx=[](bool b){};
		unsigned char serial_clk=0;
		
		enum Baudrate{
			BPS_9600=1,
			BPS_4800=2,
			BPS_1200=8,
			BPS_300=32
		};
		
		enum Baudrate baudrate_div=this->BPS_1200;
		unsigned char rx_step=0;
		unsigned char tx_step=0;
		constexpr static unsigned char cmd_baudrate[3]={0x1B,0x3A,0xEB};
		unsigned char cmd_baudrate_step=0;
		enum Baudrate next_baudrate;
		unsigned char rx_wait=0;
		
		void SerialClockTick(){
			switch(this->rx_step){
				case 0:
					if (this->Rx_prev&&(!this->Rx)) this->rx_step++;
					else if (this->rx_wait!=0){
						if (this->rx_wait==1){
							for (SimplifiedMinitelNetworkApp* app: this->Apps){
								app->RxWaitCallback();
							}
						}
						this->rx_wait--;
					}
					break;
				default:
					this->Rx_buf=(this->Rx_buf>>1)|(this->Rx?0x80:0);
					this->rx_step++;
					break;
				case 9:
					this->rx_step=0;
					for (SimplifiedMinitelNetworkApp* app: this->Apps){
						app->RxCallback(this->Rx_buf);
					}
					this->rx_wait=10;
					break;
			}
			this->Rx_prev=this->Rx;
			switch (this->tx_step){
				case 0:
					if (this->AppSelected!=NULL&&!this->AppSelected->TxEmpty()){
						this->Tx_buf=this->AppSelected->TxPop();
						switch (this->cmd_baudrate_step){
							case 3:
								switch (this->Tx_buf){
									case 0xD2:this->next_baudrate=BPS_300;this->cmd_baudrate_step++;break;
									case 0xE4:this->next_baudrate=BPS_1200;this->cmd_baudrate_step++;break;
									case 0xF6:this->next_baudrate=BPS_4800;this->cmd_baudrate_step++;break;
									case 0xFF:this->next_baudrate=BPS_9600;this->cmd_baudrate_step++;break;
									default:this->cmd_baudrate_step=((this->Tx_buf==this->cmd_baudrate[0])?1:0);break;
								}
								break;
							default:
								if (this->Tx_buf==this->cmd_baudrate[this->cmd_baudrate_step]) this->cmd_baudrate_step++;
								else this->cmd_baudrate_step=((this->Tx_buf==this->cmd_baudrate[0])?1:0);
								break;
						}
						
						this->sendTx(false);
						this->tx_step++;
					}
					break;
				default:
					this->sendTx((bool)(this->Tx_buf&1));
					this->Tx_buf=this->Tx_buf>>1;
					this->tx_step++;
					break;
				case 9:
					this->sendTx(true);
					this->tx_step++;
					break;
				case 10://2 bits stop
					if (this->AppSelected->TxEmpty()) this->AppSelected->TxQueueEmptyCallback();
					this->tx_step=0;
					if (this->cmd_baudrate_step>=4){//must be here and not at case 0 -> timming issues from 300Bd to 9600Bd (start bit missed)
						this->baudrate_div=this->next_baudrate;
						this->cmd_baudrate_step=0;
					}
					break;
			}
		}
};

class SimplifiedMinitelNetworkAppLocalWebsocket: public SimplifiedMinitelNetworkApp{//TODO: rewrite
	public:
		enum State{
			RESTING,
			INIT_MODULE,
			PARAMETERS,
			CONFIGURE,
			CONNECTED,
			CLOSING,
			UNINIT_MODULE
		};
		enum State currentState=this->RESTING;
		
		enum Const{
			
			NOT_CMD=0x00,
			CMD_ONGOING=0x01,
			CMD_FINISHED=0x02
		};
		
		virtual void PWRCallback(bool b) final override{
			if (this->PWR&&!b){
				this->forceReset();
			}
			this->PWR=b;
		};
		virtual void PTCallback(bool b) override final{
			this->PTin=b;
		}
		virtual void RxCallback(unsigned char d) override final{
			constexpr unsigned short P=0b0110100110010110;
			
			switch (this->currentState){
				case this->RESTING:
					d=((bool)(((P>>(d&0x0F))^(P>>(d>>4)))&0x01))?0x1A:d&0x7F;
					this->RESTINGReceiveCMD(d);
					break;
				case this->INIT_MODULE:
					d=((bool)(((P>>(d&0x0F))^(P>>(d>>4)))&0x01))?0x1A:d&0x7F;
					this->INIT_MODULEReceiveCMD(d);
					break;
				case this->PARAMETERS:
					d=((bool)(((P>>(d&0x0F))^(P>>(d>>4)))&0x01))?0x1A:d&0x7F;
					this->PARAMETERSReceiveCMD(d);
					break;
				case this->CONFIGURE:
					d=((bool)(((P>>(d&0x0F))^(P>>(d>>4)))&0x01))?0x1A:d&0x7F;
					this->CONFIGUREReceiveCMD(d);
					break;
				case this->CONNECTED:
					//d=((bool)(((P>>(d&0x0F))^(P>>(d>>4)))&0x01))?0x1A:d&0x7F;
					this->CONNECTEDReceiveCMD(d);
					break;
				/*{
					printf("CONN %02X\n",d);
					this->CMDBuffer.push_back(d);
					unsigned char s=this->isModuleRestCMD();
					if (s==this->NOT_CMD) this->CMDBuffer.clear();
					else if (s==this->CMD_FINISHED){
						this->UNINIT_MODULESendCMD();
						//this->CMDBuffer.clear();
						//this->print(clear_bulk,sizeof(clear_bulk)/sizeof(clear_bulk[0]));
						//this->print(reset_minitel,sizeof(reset_minitel)/sizeof(reset_minitel[0]));//TODO
						//this->currentState=this->UNINIT_MODULE;
					}
					break;
				}*/
				case this->CLOSING:
					d=((bool)(((P>>(d&0x0F))^(P>>(d>>4)))&0x01))?0x1A:d&0x7F;
					break;
				case this->UNINIT_MODULE:
					d=((bool)(((P>>(d&0x0F))^(P>>(d>>4)))&0x01))?0x1A:d&0x7F;
					this->UNINIT_MODULEReceiveCMD(d);
					break;
			}
			
		}
		
		virtual void TxQueueEmptyCallback() override final{
			switch (this->currentState){
				case this->UNINIT_MODULE:this->UNINIT_MODULETxQueueEmpty();break;
				default:break;
			}
		}
		
		virtual bool TxEmpty() override final{
			std::lock_guard<std::mutex> lock(this->pMQMutex);
			return this->qTx.empty();
		}
		virtual unsigned char TxPop() override final{
			std::lock_guard<std::mutex> lock(this->pMQMutex);
			unsigned char d=this->qTx.front();
			constexpr unsigned short P=0b0110100110010110;
			d^=((P>>(d&0x0F))^(P>>(d>>4)))<<7;
			this->qTx.pop();
			return d;
		}
		virtual void RxWaitCallback() final override{
			switch (this->currentState){
				case this->CONNECTED:this->CONNECTEDRxWait();break;
				default:break;
			}
		};
		
		virtual void CLKCallback() final override{
			if (this->currentState==this->CONNECTED) this->CONNECTEDPollCMD();
			if (this->currentState==this->UNINIT_MODULE) this->UNINIT_MODULEPollCMD();
		}
		
		~SimplifiedMinitelNetworkAppLocalWebsocket(){
			this->disconnect();
		}
	private:
		bool PTin=false;
		
		ix::WebSocket webSocket;
		std::mutex pMQMutex;
		std::queue<unsigned char> qTx;
		std::vector<unsigned char> qRx;
		std::vector<unsigned char> CMDBuffer;
		unsigned char subState=0;
		bool PWR=false;
		
		struct {
			std::vector<unsigned char> wsBuffer;
			std::vector<unsigned char> wsSplit;
			
			int baudrate=3;
			bool parity=false;
			bool block=true;
			int ping=3;
			
			unsigned char display=0;
			bool echo=false;
			bool extendedKeyboard=false;
			bool upperCase=false;
			bool cursor=false;
			
			unsigned char currentLine=0;
		} parameters;
		
		void forceReset(){
			std::queue<unsigned char> empty;
			std::swap(this->qTx,empty);
			this->qRx.clear();
			this->CMDBuffer.clear();
			this->subState=0;
			this->currentState=this->RESTING;
			this->disconnect();
			this->PTout=true;
		}
		
		void addCharWsBuffer(){
			if (this->parameters.wsSplit.size()==29*2) this->print(bell,sizeof(bell)/sizeof(bell[0]));
			else{
				for (unsigned char d: this->CMDBuffer){
					this->qTx.push(d);
					this->parameters.wsBuffer.push_back(d);
				}
				this->parameters.wsSplit.push_back(this->CMDBuffer.size());
				if (this->parameters.wsSplit.size()==29) this->moveCursor(4,12);
			}
		}
		
		void removeCharWsBuffer(){
			if (this->parameters.wsSplit.size()==0) this->print(bell,sizeof(bell)/sizeof(bell[0]));
			else{
				unsigned char l=this->parameters.wsSplit.back();
				this->parameters.wsSplit.pop_back();
				for (int i=0;i<l;i++) this->parameters.wsBuffer.pop_back();
				if (this->parameters.wsSplit.size()==28) this->moveCursor(3,40);
				else this->qTx.push(0x08);
				this->selectLineStatic(false);
				this->printString(".");
				this->qTx.push(0x08);
				this->selectLineStatic(true);
			}
		}
		
		void deleteWsBuffer(){
			if (this->parameters.wsSplit.size()==0) this->print(bell,sizeof(bell)/sizeof(bell[0]));
			else{
				this->parameters.wsBuffer.clear();
				this->parameters.wsSplit.clear();
				this->refreshURLLine(this->parameters.currentLine==0);
			}
		}
		
		unsigned char isModuleWakeCMD(){
			constexpr unsigned char CMD[3]={0x13,0x49,0x57};
			if (this->CMDBuffer.size()<sizeof(CMD)/sizeof(CMD[0])){
				if (!(bool)memcmp(this->CMDBuffer.data(),CMD,this->CMDBuffer.size()*sizeof(unsigned char))){
					return this->CMD_ONGOING;
				}
			}
			else if (this->CMDBuffer.size()==sizeof(CMD)/sizeof(CMD[0])){
				if (!(bool)memcmp(this->CMDBuffer.data(),CMD,this->CMDBuffer.size()*sizeof(unsigned char))){
					return this->CMD_FINISHED;
				}
			}
			return this->NOT_CMD;
		}
		
		unsigned char isModuleRestCMD(){
			constexpr unsigned char CMD[2]={0x13,0x49};
			if (this->CMDBuffer.size()<sizeof(CMD)/sizeof(CMD[0])){
				if (!(bool)memcmp(this->CMDBuffer.data(),CMD,this->CMDBuffer.size()*sizeof(unsigned char))){
					return this->CMD_ONGOING;
				}
			}
			else if (this->CMDBuffer.size()==sizeof(CMD)/sizeof(CMD[0])){
				if (!(bool)memcmp(this->CMDBuffer.data(),CMD,this->CMDBuffer.size()*sizeof(unsigned char))){
					return this->CMD_FINISHED;
				}
			}
			return this->NOT_CMD;
		}
		
		unsigned char isModuleForceRestCMD(){
			constexpr unsigned char CMD[4]={0x13,0x49,0x13,0x49};
			if (this->CMDBuffer.size()<sizeof(CMD)/sizeof(CMD[0])){
				if (!(bool)memcmp(this->CMDBuffer.data(),CMD,this->CMDBuffer.size()*sizeof(unsigned char))){
					return this->CMD_ONGOING;
				}
			}
			else if (this->CMDBuffer.size()==sizeof(CMD)/sizeof(CMD[0])){
				if (!(bool)memcmp(this->CMDBuffer.data(),CMD,this->CMDBuffer.size()*sizeof(unsigned char))){
					return this->CMD_FINISHED;
				}
			}
			return this->NOT_CMD;
		}
		
		unsigned char isACKCMD(){
			if (this->CMDBuffer.size()<2){
				if (this->CMDBuffer[0]==0x13){
					return this->CMD_ONGOING;
				}
			}
			else if (this->CMDBuffer.size()==2){
				if (this->CMDBuffer[0]==0x13){
					return this->CMD_FINISHED;
				}
			}
			return this->NOT_CMD;
		}
		
		unsigned char isPRO1CMD(){
			constexpr unsigned char CMD[2]={0x1B,0x39};
			if (this->CMDBuffer.size()<3){
				if (!(bool)memcmp(this->CMDBuffer.data(),CMD,this->CMDBuffer.size()*sizeof(unsigned char))){
					return this->CMD_ONGOING;
				}
			}
			else if (this->CMDBuffer.size()==3){
				if (!(bool)memcmp(this->CMDBuffer.data(),CMD,2*sizeof(unsigned char))){
					return this->CMD_FINISHED;
				}
			}
			return this->NOT_CMD;
		}
		
		unsigned char isPRO2CMD(){
			constexpr unsigned char CMD[2]={0x1B,0x3A};
			if (this->CMDBuffer.size()<4){
				if (!(bool)memcmp(this->CMDBuffer.data(),CMD,(this->CMDBuffer.size()<2?this->CMDBuffer.size():2)*sizeof(unsigned char))){
					return this->CMD_ONGOING;
				}
			}
			else if (this->CMDBuffer.size()==4){
				if (!(bool)memcmp(this->CMDBuffer.data(),CMD,2*sizeof(unsigned char))){
					return this->CMD_FINISHED;
				}
			}
			return this->NOT_CMD;
		}
		
		unsigned char isPRO3CMD(){
			constexpr unsigned char CMD[2]={0x1B,0x3B};
			if (this->CMDBuffer.size()<5){
				if (!(bool)memcmp(this->CMDBuffer.data(),CMD,(this->CMDBuffer.size()<2?this->CMDBuffer.size():2)*sizeof(unsigned char))){
					return this->CMD_ONGOING;
				}
			}
			else if (this->CMDBuffer.size()==5){
				if (!(bool)memcmp(this->CMDBuffer.data(),CMD,2*sizeof(unsigned char))){
					return this->CMD_FINISHED;
				}
			}
			return this->NOT_CMD;
		}
		
		unsigned char isCursorPositionCMD(){
			switch(this->CMDBuffer.size()){
				case 1:
					if (this->CMDBuffer[0]==0x1F) return this->CMD_ONGOING;
					break;
				case 2:
					if (this->CMDBuffer[0]==0x1F&&((bool)(this->CMDBuffer[1]&0x40))) return this->CMD_ONGOING;
					break;
				case 3:
					if (this->CMDBuffer[0]==0x1F&&((bool)(this->CMDBuffer[1]&0x40))&&((bool)(this->CMDBuffer[2]&0x40))) return this->CMD_FINISHED;
					break;
			}
			return this->NOT_CMD;
		}
		
		unsigned char isSS2(){
			switch(this->CMDBuffer.size()){
				case 1:
					if (this->CMDBuffer[0]==0x19) return this->CMD_ONGOING;
					break;
				case 2:
					if (this->CMDBuffer[0]==0x19&&(this->CMDBuffer[1]&0xF0)==0x40) return this->CMD_ONGOING;
					else if (this->CMDBuffer[0]==0x19&&this->CMDBuffer[1]>=0x21&&this->CMDBuffer[1]<=0x7E) return this->CMD_FINISHED;
					break;
				case 3:
					if (this->CMDBuffer[0]==0x19&&(this->CMDBuffer[1]&0xF0)==0x40&&this->CMDBuffer[2]>=0x20&&this->CMDBuffer[2]<=0x7F) return this->CMD_FINISHED;
					break;
			}
			return this->NOT_CMD;
		}
		
		unsigned char isCSICMD(){
			switch(this->CMDBuffer.size()){
				case 1:
					if (this->CMDBuffer[0]==0x1B) return this->CMD_ONGOING;
					break;
				case 2:
					if (this->CMDBuffer[0]==0x1B&&this->CMDBuffer[1]==0x5B) return this->CMD_ONGOING;
					break;
				default:
					if (this->CMDBuffer[0]==0x1B&&this->CMDBuffer[1]==0x5B){
						size_t i=2;
						while (i<this->CMDBuffer.size()&&(this->CMDBuffer[i]&0xF0)==0x30) i++;
						while (i<this->CMDBuffer.size()&&(this->CMDBuffer[i]&0xF0)==0x20) i++;
						if (i==this->CMDBuffer.size()){
							return this->CMD_ONGOING;
						}
						if (i==this->CMDBuffer.size()-1&&this->CMDBuffer[i]>=0x40&&this->CMDBuffer[i]<=0x7E){
							return this->CMD_FINISHED;
						}
					}
					break;
			}
			return this->NOT_CMD;
		}
		
		unsigned char isISO2022CMD(){
			switch(this->CMDBuffer.size()){
				case 1:
					if (this->CMDBuffer[0]==0x1B) return this->CMD_ONGOING;
					break;
				case 2:
					if (this->CMDBuffer[0]==0x1B&&(this->CMDBuffer[1]&0xF0)==0x20) return this->CMD_ONGOING;
					break;
				default:
					if (this->CMDBuffer[0]==0x1B){
						for (size_t i=1;i<this->CMDBuffer.size()-1;i++){
							if ((this->CMDBuffer[i]&0xF0)!=0x20) return this->NOT_CMD;
						}
						switch (this->CMDBuffer[this->CMDBuffer.size()-1]){
							case 0x20 ... 0x2F:return this->CMD_ONGOING;
							case 0x30 ... 0x7E:return this->CMD_FINISHED;
						}
					}
					break;
			}
			return this->NOT_CMD;
		}
		
		void RESTINGReceiveCMD(unsigned char d){
			if (this->PTin){
				resync:
				this->CMDBuffer.push_back(d);
				unsigned char s=this->isModuleWakeCMD();//TODO: resync
				if (s==this->NOT_CMD){
					if (this->CMDBuffer.size()>1){
						this->CMDBuffer.clear();
						goto resync;
					}
					else this->CMDBuffer.clear();
				}
				else if (s==this->CMD_FINISHED){
					this->INIT_MODULESendCMD();
				}
			}
			else if (this->CMDBuffer.size()!=0) this->CMDBuffer.clear();
		}
		
		void INIT_MODULESendCMD(){
			if (this->currentState!=this->INIT_MODULE){
				this->subState=0;
				this->currentState=this->INIT_MODULE;
				this->CMDBuffer.clear();
			}
			switch (this->subState){
				case 0:this->PTout=false;break;
				case 1:this->qTx.push(0x1B);this->qTx.push(0x3A);this->qTx.push(0x6B);this->qTx.push(0x7F);break;
			}
		}
		
		void INIT_MODULEReceiveCMD(unsigned char d){
			resync:
			this->CMDBuffer.push_back(d);
			unsigned char s=this->NOT_CMD;
			switch (this->subState){
				case 0:
					s=this->isACKCMD();
					if (s==this->CMD_FINISHED&&this->CMDBuffer.back()==0x54){
						this->subState=1;
						this->INIT_MODULESendCMD();
					}
					break;
				case 1:
					s=this->isPRO2CMD();
					if (s==this->CMD_FINISHED&&this->CMDBuffer[2]==0x75){
						this->PARAMETERSSendCMD();
					}
					break;
			}
			if (s==this->NOT_CMD&&this->CMDBuffer.size()>1){
				this->CMDBuffer.clear();
				goto resync;
			}
			if (s==this->NOT_CMD||s==this->CMD_FINISHED) this->CMDBuffer.clear();
		}
		
		void PARAMETERSSendCMD(){
			this->currentState=this->PARAMETERS;
			this->CMDBuffer.clear();
			this->printParametersStaticPage();
			if (this->parameters.wsSplit.size()==0) this->setURL((char*)"ws://localhost:8080");
			this->changeSelection(this->parameters.currentLine);
		}
		
		void PARAMETERSReceiveCMD(unsigned char d){
			resync:
			this->CMDBuffer.push_back(d);
			unsigned char s=this->isModuleRestCMD();
			if (s==this->NOT_CMD){
				s=this->isACKCMD();
				if (s==this->NOT_CMD){
					s=this->isPRO1CMD()|this->isPRO2CMD()|this->isPRO3CMD()|this->isCursorPositionCMD()|this->isCSICMD()|this->isISO2022CMD();
					if (s==this->NOT_CMD){
						s=this->isSS2();
						if (s==this->NOT_CMD){
							if (this->CMDBuffer.size()>1){
								this->CMDBuffer.clear();
								goto resync;
							}
							if (this->parameters.currentLine!=0){
								if (this->CMDBuffer.size()==1&&this->CMDBuffer[0]==0x20) this->cycleParameterOption(this->parameters.currentLine);
								else this->print(bell,sizeof(bell)/sizeof(bell[0]));
							}
							else{
								if (this->CMDBuffer[0]>=0x20&&this->CMDBuffer[0]<=0x7E) this->addCharWsBuffer();
								else this->print(bell,sizeof(bell)/sizeof(bell[0]));
							}
							this->CMDBuffer.clear();
						}
						else if (s==this->CMD_FINISHED){
							if (this->parameters.currentLine==0) this->addCharWsBuffer();
							else this->print(bell,sizeof(bell)/sizeof(bell[0]));
							this->CMDBuffer.clear();
						}
					}
					else if ((bool)(s&this->CMD_FINISHED)) this->CMDBuffer.clear();
				}
				else if (s==this->CMD_FINISHED){
					switch (this->CMDBuffer[1]){
						case 0x41://envoi
						{
							char* url=this->getURL();
							printf("url:%s\n",url);
							free(url);
							/*this->currentState=this->CONFIGURE;
							this->subState=0;
							this->qTx.push(0x1B);this->qTx.push(0x3A);this->qTx.push(0x32);
							if ((bool)(this->parameters.display&1)) this->qTx.push(0x7D);
							else this->qTx.push(0x7E);*/
							this->CONFIGURESendCMD();
							break;
						}
						case 0x48://suite
							this->changeSelection(this->parameters.currentLine+1);
							break;
						case 0x42://retour
							this->changeSelection(this->parameters.currentLine-1);
							break;
						case 0x45://annulation
							if (this->parameters.currentLine!=0) this->print(bell,sizeof(bell)/sizeof(bell[0]));
							else{
								this->deleteWsBuffer();
							}
							break;
						case 0x47://correction
							if (this->parameters.currentLine!=0) this->print(bell,sizeof(bell)/sizeof(bell[0]));
							else{
								this->removeCharWsBuffer();
							}
							break;
						case 0x43:
						case 0x44:
						case 0x46:
							this->print(bell,sizeof(bell)/sizeof(bell[0]));
							break;
						case 0x53:
						case 0x5B://re disable local echo
							this->print(disable_local_echo,sizeof(disable_local_echo)/sizeof(disable_local_echo[0]));
							break;
						/*default:
							printf("SEP %02X\n",this->CMDBuffer[1]);
							this->print(bell,sizeof(bell)/sizeof(bell[0]));
							break;*/
					}
					this->CMDBuffer.clear();
				}
			}
			else if (s==this->CMD_FINISHED){
				this->UNINIT_MODULESendCMD();
				this->CMDBuffer.clear();
			}
		}
		
		void UNINIT_MODULESendCMD(){
			if (this->currentState!=this->UNINIT_MODULE){
				this->subState=0;
				this->currentState=this->UNINIT_MODULE;
				this->CMDBuffer.clear();
			}
			switch (this->subState){
				case 0:this->print(reset_minitel,sizeof(reset_minitel)/sizeof(reset_minitel[0]));break;
				case 1:this->print(clear_bulk,sizeof(clear_bulk)/sizeof(clear_bulk[0]));break;
			}
			
		}
		
		void UNINIT_MODULEReceiveCMD(unsigned char d){
			resync:
			this->CMDBuffer.push_back(d);
			unsigned char s=this->NOT_CMD;
			if(this->subState==0){
				s=this->isACKCMD();
				if (s==this->CMD_FINISHED&&this->CMDBuffer[1]==0x5E){
					this->subState=1;
					this->UNINIT_MODULESendCMD();
				}
			}
			if (s==this->NOT_CMD&&this->CMDBuffer.size()>1){
				this->CMDBuffer.clear();
				goto resync;
			}
			if (s==this->NOT_CMD||s==this->CMD_FINISHED) this->CMDBuffer.clear();
		}
		
		void UNINIT_MODULETxQueueEmpty(){
			if(this->subState==1){
				this->stop_delay_cnt=0;
				this->subState=2;
			}
		}
		
		constexpr static unsigned short stop_delay_50ms=480;
		unsigned short stop_delay_cnt=0;
		void UNINIT_MODULEPollCMD(){//wait 50ms as described in the STURM / not waiting could stop clear cmd in the middle of the screen + directory screen could be glitched (for the M12 we emulate)
			if (this->subState==2){
				this->stop_delay_cnt++;
				if (this->stop_delay_cnt>=this->stop_delay_50ms){
					this->PTout=true;
					this->currentState=this->RESTING;
					this->subState=0;
				}
			}
		}
		
		void CONFIGURESendCMD(){
			if (this->currentState!=this->CONFIGURE){
				this->subState=0;
				this->currentState=this->CONFIGURE;
				this->CMDBuffer.clear();
			}
			switch(this->subState){
				case 0://modem status
					this->qTx.push(0x1B);this->qTx.push(0x3A);this->qTx.push(0x62);this->qTx.push(0x5A);break;
				case 1://disable link keyboard->modem
					this->print(disable_local_echo,sizeof(disable_local_echo)/sizeof(disable_local_echo[0]));break;
				case 2://enable link keyboard->modem
					this->print(enable_local_echo,sizeof(enable_local_echo)/sizeof(enable_local_echo[0]));break;
				case 3://status
				case 9:
					this->qTx.push(0x1B);this->qTx.push(0x39);this->qTx.push(0x72);break;
				case 4://videotex standard
					this->qTx.push(0x1B);this->qTx.push(0x3A);this->qTx.push(0x32);this->qTx.push(0x7E);break;
				case 5://tele standard
					this->qTx.push(0x1B);this->qTx.push(0x3A);this->qTx.push(0x32);this->qTx.push(0x7D);break;
				case 6://keyboard status
					this->qTx.push(0x1B);this->qTx.push(0x3A);this->qTx.push(0x72);this->qTx.push(0x59);break;
				case 7://simple keyboard / TODO: does not work in tele mode (minitel ack but do nothing)
					this->qTx.push(0x1B);this->qTx.push(0x3B);this->qTx.push(0x6A);this->qTx.push(0x59);this->qTx.push(0x41);break;
				case 8://extended keyboard
					this->qTx.push(0x1B);this->qTx.push(0x3B);this->qTx.push(0x69);this->qTx.push(0x59);this->qTx.push(0x41);break;
				case 10://page
					this->qTx.push(0x1B);this->qTx.push(0x3A);this->qTx.push(0x6A);this->qTx.push(0x43);break;
				case 11://roll
					this->qTx.push(0x1B);this->qTx.push(0x3A);this->qTx.push(0x69);this->qTx.push(0x43);break;
				case 12://lower case
					this->print(default_lower_case,sizeof(default_lower_case)/sizeof(default_lower_case[0]));break;
				case 13://upper case
					this->print(default_upper_case,sizeof(default_upper_case)/sizeof(default_upper_case[0]));break;
				case 14://baudrate
					this->qTx.push(0x1B);this->qTx.push(0x3A);this->qTx.push(0x6B);
					switch (this->parameters.baudrate){
						case 0:this->qTx.push(0x52);break;
						case 1:this->qTx.push(0x64);break;
						case 2:this->qTx.push(0x76);break;
						case 3:this->qTx.push(0x7F);break;
					}
					break;
				case 15://disable cursor+clear screen+change state
					if ((bool)(this->parameters.display&1)){
						this->qTx.push(0x1B);this->qTx.push(0x5B);this->qTx.push(0x3C);this->qTx.push(0x31);
						this->qTx.push(0x68);
					}
					else{
						this->qTx.push(0x14);
					}
					this->qTx.push(0x0C);
					this->CONNECTEDSendCMD();
					break;
			}
		}
		
		void CONFIGUREReceiveCMD(unsigned char d){
			resync:
			this->CMDBuffer.push_back(d);
			unsigned char s=this->NOT_CMD;
			switch(this->subState){
				case 0 ... 2://local echo
					s=this->isPRO3CMD();
					if (s==this->CMD_FINISHED&&this->CMDBuffer[2]==0x63&&this->CMDBuffer[3]==0x5A){
						if (this->parameters.echo==(bool)(this->CMDBuffer[4]&0x02)) this->subState=3;
						else if (this->parameters.echo) this->subState=2;
						else this->subState=1;
						this->CONFIGURESendCMD();
					}
					break;
				case 3://standard
					s=this->isPRO2CMD();
					if (s==this->CMD_FINISHED&&this->CMDBuffer[2]==0x73){
						if ((bool)(this->parameters.display&0x01)==(bool)(this->CMDBuffer[3]&0x01)) this->subState=6;
						else if ((bool)(this->parameters.display&0x01)) this->subState=5;
						else this->subState=4;
						this->CONFIGURESendCMD();
					}
					break;
				case 4 ... 5://ack standard
					s=this->isACKCMD();
					if (s==this->CMD_FINISHED&&this->CMDBuffer[1]==0x70){
						this->subState=6;
						this->CONFIGURESendCMD();
					}
					break;
				case 6 ... 8://keyboard
					s=this->isPRO3CMD();
					if (s==this->CMD_FINISHED&&this->CMDBuffer[2]==0x73&&this->CMDBuffer[3]==0x59){
						if (this->parameters.extendedKeyboard==(bool)(this->CMDBuffer[4]&0x01)) this->subState=9;
						else if (this->parameters.extendedKeyboard) this->subState=8;
						else this->subState=7;
						this->CONFIGURESendCMD();
					}
					break;
				case 9 ... 13://case+page/roll
					s=this->isPRO2CMD();
					if (s==this->CMD_FINISHED&&this->CMDBuffer[2]==0x73){
						if ((bool)(this->parameters.display&0x02)==(bool)(this->CMDBuffer[3]&0x02)){
							if ((!this->parameters.upperCase)==(bool)(this->CMDBuffer[3]&0x08)) this->subState=14;
							else if (this->parameters.upperCase) this->subState=13;
							else this->subState=12;
						}
						else if ((bool)(this->parameters.display&0x02)) this->subState=11;
						else this->subState=10;
						this->CONFIGURESendCMD();
					}
					break;
				case 14://baudrate
					s=this->isPRO2CMD();
					if (s==this->CMD_FINISHED&&this->CMDBuffer[2]==0x75){
						this->subState=15;
						this->CONFIGURESendCMD();
					}
					else if (s==this->NOT_CMD){//fail
						if (this->parameters.baudrate<2) this->parameters.baudrate++;
						else if (this->parameters.baudrate>2) this->parameters.baudrate--;
						this->CONFIGURESendCMD();
					}
					break;
					
			}
			if (s==this->NOT_CMD&&this->CMDBuffer.size()>1){
				this->CMDBuffer.clear();
				goto resync;
			}
			if (s==this->NOT_CMD||s==this->CMD_FINISHED) this->CMDBuffer.clear();
					
		}
		
		void CONNECTEDSendCMD(){
			if (this->currentState!=this->CONNECTED){
				this->currentState=this->CONNECTED;
				this->subState=0;
				this->CMDBuffer.clear();
			}
			switch (this->subState){
				case 0:
					this->moveCursor(0,1);
					this->printString("` ");
					{
						int n=0;
						for (size_t i=0;i<32&&i<this->parameters.wsSplit.size();i++){
							n+=this->parameters.wsSplit[i];
						}
						this->print(this->parameters.wsBuffer.data(),n);
					}
					this->qTx.push(0x18);
					//this->qTx.push(0x1E);
					
					this->moveCursor(0,1);
					this->disconnect();
					this->connect(this->getURL());
					break;
				case 1:
					if (this->qTx.empty()){//could not use this->TxEmpty -> deadlock
						this->connection_poll_anim=(this->connection_poll_anim+1)%4;
						//this->moveCursor(0,1);
						switch (this->connection_poll_anim){
							case 0:this->qTx.push(0x60);break;
							case 1:this->qTx.push(0x5C);break;
							case 2:this->qTx.push(0x7C);break;
							case 3:this->qTx.push(0x2F);break;
						}
						this->qTx.push(0x08);
						//this->qTx.push(0x0A);
					}
					break;
				case 2:
					this->printString("\x19\x2E\x0A\x1E");
					
					//cursor
					if ((bool)(this->parameters.display&1)){
						this->qTx.push(0x1B);this->qTx.push(0x5B);this->qTx.push(0x3C);this->qTx.push(0x31);
						if (this->parameters.cursor) this->qTx.push(0x6C);
						else this->qTx.push(0x68);
					}
					else{
						if (this->parameters.cursor) this->qTx.push(0x11);
						else this->qTx.push(0x14);
					}
					//////////////////////////////////
					break;
				case 3:
				case 5:
				case 7:
					{
						std::queue<unsigned char> empty;
						this->qTx.swap(empty);//empty the queue
					}
					this->qRx.clear();
					this->qTx.push(0x1B);this->qTx.push(0x3A);this->qTx.push(0x6B);this->qTx.push(0x7F);
					break;
				case 4:
					this->print(reset_minitel,sizeof(reset_minitel)/sizeof(reset_minitel[0]));
					this->moveCursor(0,1);
					this->qTx.push(0x18);
					this->printString("! Erreur de connexion");
					this->qTx.push(0x1E);
					this->PARAMETERSSendCMD();
					break;
				case 6:
					this->print(reset_minitel,sizeof(reset_minitel)/sizeof(reset_minitel[0]));
					this->moveCursor(0,1);
					this->qTx.push(0x18);
					this->printString("* D\x19\x42""econnexion (serveur)");
					this->qTx.push(0x1E);
					this->UNINIT_MODULESendCMD();
					break;
				case 8:
					this->print(reset_minitel,sizeof(reset_minitel)/sizeof(reset_minitel[0]));
					this->moveCursor(0,1);
					this->qTx.push(0x18);
					this->printString("* D\x19\x42""econnexion (utilisateur)");
					this->qTx.push(0x1E);
					this->UNINIT_MODULESendCMD();
					break;
			}
		}
		
		void CONNECTEDReceiveCMD(unsigned char d){
			constexpr unsigned short P=0b0110100110010110;
			bool closeWS=false;
			unsigned char s=this->NOT_CMD;
			{
				std::lock_guard<std::mutex> lock(this->pMQMutex);//protect qTx+subState
				switch (this->subState){
					case 2:
						if (this->parameters.parity) this->qRx.push_back(d);
						d=((bool)(((P>>(d&0x0F))^(P>>(d>>4)))&0x01))?0x1A:d&0x7F;
						if (!this->parameters.parity) this->qRx.push_back(d);
						resync1:
						this->CMDBuffer.push_back(d);
						s=this->isModuleForceRestCMD();
						if (s==this->CMD_FINISHED){
							this->subState=7;
							closeWS=true;
						}
						if (s==this->NOT_CMD&&this->CMDBuffer.size()>1){
							this->CMDBuffer.clear();
							goto resync1;
						}
						break;
					case 3:
					case 5:
					case 7:
						d=((bool)(((P>>(d&0x0F))^(P>>(d>>4)))&0x01))?0x1A:d&0x7F;
						resync2:
						this->CMDBuffer.push_back(d);
						s=this->isPRO2CMD();
						if (s==this->NOT_CMD&&this->CMDBuffer.size()>1){
							this->CMDBuffer.clear();
							goto resync2;
						}
						if (s==this->CMD_FINISHED&&this->CMDBuffer[2]==0x75){
							this->subState++;
							this->CONNECTEDSendCMD();
						}
						break;
				}
			}
			if (s==this->NOT_CMD||s==this->CMD_FINISHED) this->CMDBuffer.clear();
			
			if (closeWS) this->disconnect();//avoid deadlock
		}
		
		unsigned short connection_poll_div=0;
		constexpr static unsigned short connection_poll_02s=1920;
		unsigned char connection_poll_anim=0;
		void CONNECTEDPollCMD(){
			std::lock_guard<std::mutex> lock(this->pMQMutex);//protect qTx+subState
			
			if (this->subState==0||this->subState==1){
				this->connection_poll_div=(this->connection_poll_div+1)%this->connection_poll_02s;
				if (this->connection_poll_div==0){
					this->subState=1;
					this->CONNECTEDSendCMD();
				}
			}
		}
		
		void CONNECTEDRxWait(){
			if (!this->qRx.empty()){
				if (this->webSocket.getReadyState()==ix::ReadyState::Open) this->webSocket.sendBinary(this->qRx);
				this->qRx.clear();
			}
		}
		
		void connect(const char* url){
			//std::string serv="ws://go.minipavi.fr:8182";//"ws://localhost:8080";
			this->webSocket.setUrl(url);
			if (this->parameters.ping==0) this->webSocket.setPingInterval(-1);
			else this->webSocket.setPingInterval(this->parameters.ping*15);
			this->webSocket.disablePerMessageDeflate();
			this->webSocket.disableAutomaticReconnection();
			this->webSocket.setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg){
				switch (msg->type){
					case ix::WebSocketMessageType::Error:
						{
							std::lock_guard<std::mutex> lock(this->pMQMutex);
							this->subState=3;
							this->CONNECTEDSendCMD();
						}
						break;
					case ix::WebSocketMessageType::Open:
						{
							std::lock_guard<std::mutex> lock(this->pMQMutex);
							this->subState=2;
							this->CONNECTEDSendCMD();
						}
						break;
					case ix::WebSocketMessageType::Close:
						{
							std::lock_guard<std::mutex> lock(this->pMQMutex);
							if (this->subState==2){
								this->subState=5;
								this->CONNECTEDSendCMD();
							}
							if (this->subState==7) this->CONNECTEDSendCMD();
						}
						break;
					case ix::WebSocketMessageType::Message:
						{
							std::lock_guard<std::mutex> lock(this->pMQMutex);
							if (this->subState==2){
								for (unsigned int i=0;i<msg->str.length();i++) this->qTx.push((unsigned char)msg->str[i]);
							}
						}
						break;
					default:
						break;
				}
			});
			this->webSocket.start();
		}
		void disconnect(){
			//if (this->webSocket.getReadyState()==ix::ReadyState::Open||this->webSocket.getReadyState()==ix::ReadyState::Connecting) 
			this->webSocket.stop();
		}
		
		void setURL(const char* url){
			std::vector<unsigned char>* p=utf8_to_videotex_ts9347(url,false,true);
			this->parameters.wsBuffer.clear();
			this->parameters.wsSplit.clear();
			this->parameters.wsBuffer.insert(this->parameters.wsBuffer.end(), p->begin(), p->end());
			//compute the length of each visible characters
			size_t i=0;
			int n=0;
			while (i<p->size()){
				switch ((*p)[i]){
					case 0x0E:
					case 0x0F:
						n++;
						i++;
						break;
					case 0x19://p is considered a valid videotex data
						if (((*p)[i+1]&0xF0)==0x40){
							i+=3;
							n+=3;
						}
						else{
							i+=2;
							n+=2;
						}
						this->parameters.wsSplit.push_back(n);
						n=0;
						break;
					default:
						n++;
						i++;
						this->parameters.wsSplit.push_back(n);
						n=0;
						break;
				}
			}
			this->parameters.wsSplit[this->parameters.wsSplit.size()-1]+=n;//if SI/SO at the end of the videotex data
			delete p;
			if (this->currentState==this->PARAMETERS){
				this->refreshURLLine(this->parameters.currentLine==0);
			}
		}
		
		char* getURL(){//TODO: sanitize url needed
			char* d=videotex_to_utf8(&(this->parameters.wsBuffer));
			return d;
		}
		
		void printLineSeparator(unsigned char row){
			this->moveCursor(row,1);
			this->print(foreground_color_magenta,sizeof(foreground_color_magenta)/sizeof(foreground_color_magenta[0]));
			this->print(line_separator_40c,sizeof(line_separator_40c)/sizeof(line_separator_40c[0]));
		}
		void selectLineDynamic(bool s){
			this->print(foreground_color_black,sizeof(foreground_color_black)/sizeof(foreground_color_black[0]));
			if (s) this->print(background_color_white,sizeof(background_color_white)/sizeof(background_color_white[0]));
			else this->print(background_color_magenta,sizeof(background_color_magenta)/sizeof(background_color_magenta[0]));
			this->print(swap_color,sizeof(swap_color)/sizeof(swap_color[0]));
			this->qTx.push(0x20);
		}
		void selectLineStatic(bool s){
			if (!s) this->print(foreground_color_magenta,sizeof(foreground_color_magenta)/sizeof(foreground_color_magenta[0]));
			else this->print(foreground_color_white,sizeof(foreground_color_white)/sizeof(foreground_color_white[0]));
		}
		void printString(const char* d){
			for (size_t i=0;i<strlen(d);i++) this->qTx.push(d[i]);
		}
		void print(const unsigned char*d, size_t s){
			for (size_t i=0;i<s;i++) this->qTx.push(d[i]);
		}
		
		void moveCursor(unsigned char row,unsigned char column){
			this->qTx.push(0x1F);this->qTx.push(0x40+row);this->qTx.push(0x40+column);
		}
		
		void changeSelection(unsigned char line){
			if (line>8){
				this->print(bell,sizeof(bell)/sizeof(bell[0]));
				return;
			}
			switch (this->parameters.currentLine){
				case 0:
					this->print(disable_cursor,sizeof(disable_cursor)/sizeof(disable_cursor[0]));
					this->print(default_upper_case,sizeof(default_upper_case)/sizeof(default_upper_case[0]));
					this->refreshURLLine(false);
					
					this->moveCursor(23,15);
					this->printString("\x1B\x45"" ligne pr\x19\x42""ec\x19\x42""edente""\x1B\x5A"" ""\x1B\x5D"" Retour ");
					break;
				case 1:
					this->moveCursor(5,2);
					this->selectLineDynamic(false);
					break;
				case 2:
					this->moveCursor(6,4);
					this->selectLineDynamic(false);
					break;
				case 3:
					this->moveCursor(7,6);
					this->selectLineDynamic(false);
					break;
				case 4:
					this->moveCursor(8,1);
					this->selectLineDynamic(false);
					break;
				case 5:
					this->moveCursor(9,6);
					this->selectLineDynamic(false);
					break;
				case 6:
					this->moveCursor(10,3);
					this->selectLineDynamic(false);
					break;
				case 7:
					this->moveCursor(11,5);
					this->selectLineDynamic(false);
					break;
				case 8:
					this->moveCursor(12,3);
					this->selectLineDynamic(false);
					this->moveCursor(22,17);
					this->printString("\x1B\x45"" ligne suivante""\x1B\x5A"" ""\x1B\x5D"" Suite  ");
					break;
			}
			
			this->printContextMenu(line);
			
			switch (line){
				case 0:
					this->moveCursor(23,15);
					this->printString("\x18");
					
					this->refreshURLLine(true);
					//this->moveCursor(3,12);
					this->print(default_lower_case,sizeof(default_lower_case)/sizeof(default_lower_case[0]));
					this->print(enable_cursor,sizeof(enable_cursor)/sizeof(enable_cursor[0]));
					break;
				case 1:
					this->moveCursor(5,2);
					this->selectLineDynamic(true);
					break;
				case 2:
					this->moveCursor(6,4);
					this->selectLineDynamic(true);
					break;
				case 3:
					this->moveCursor(7,6);
					this->selectLineDynamic(true);
					break;
				case 4:
					this->moveCursor(8,1);
					this->selectLineDynamic(true);
					break;
				case 5:
					this->moveCursor(9,6);
					this->selectLineDynamic(true);
					break;
				case 6:
					this->moveCursor(10,3);
					this->selectLineDynamic(true);
					break;
				case 7:
					this->moveCursor(11,5);
					this->selectLineDynamic(true);
					break;
				case 8:
					this->moveCursor(12,3);
					this->selectLineDynamic(true);
					this->moveCursor(22,17);
					this->printString("\x18");
					break;
			}
			this->parameters.currentLine=line;
		}
		
		void refreshURLLine(bool select){
			if (select) this->print(disable_cursor,sizeof(disable_cursor)/sizeof(disable_cursor[0]));
			this->moveCursor(3,8);
			
			this->selectLineStatic(select);
			this->printString("url:");
			if (select){
				this->selectLineStatic(false);
				this->printString(".\x12\x5C");
				this->moveCursor(4,12);
				this->selectLineStatic(false);
				this->printString(".\x12\x5C");
			}
			else{
				this->printString(" \x12\x5C");
				this->moveCursor(4,12);
				this->printString(" \x12\x5C");
			}
			this->moveCursor(3,12);
			this->selectLineStatic(select);
			if (this->parameters.wsSplit.size()<=29){
				this->print(this->parameters.wsBuffer.data(),this->parameters.wsBuffer.size());
				if (this->parameters.wsSplit.size()==29) this->moveCursor(4,12);
			}
			else{
				unsigned char l=0;
				for (unsigned char i=0;i<29;i++) l+=this->parameters.wsSplit[i];
				this->print(this->parameters.wsBuffer.data(),l);
				this->moveCursor(4,12);
				this->selectLineStatic(select);
				unsigned char* p=this->parameters.wsBuffer.data()+l;
				l=0;
				for (unsigned char i=29;i<std::min(2*29,(int)this->parameters.wsSplit.size());i++) l+=this->parameters.wsSplit[i];
				this->print(p,l);
				
			}
			if (select) this->print(enable_cursor,sizeof(enable_cursor)/sizeof(enable_cursor[0]));
		}
		
		void printParametersStaticPage(){
			//char url[]="ws://localhost:8080";
			this->print(reset_minitel,sizeof(reset_minitel)/sizeof(reset_minitel[0]));
			this->print(disable_local_echo,sizeof(disable_local_echo)/sizeof(disable_local_echo[0]));
			//this->print(disable_cursor,sizeof(disable_cursor)/sizeof(disable_cursor[0]));
			this->print(clear_bulk,sizeof(clear_bulk)/sizeof(clear_bulk[0]));
			
			this->moveCursor(1,11);
			this->printString("CONNEXION WEBSOCKET");
			this->printLineSeparator(2);
			
			this->refreshURLLine(false);
			
			this->moveCursor(5,2);
			this->selectLineDynamic(false);
			this->printString("baudrate:");
			switch (this->parameters.baudrate){
				case 0:this->printString("300 Bauds");break;
				case 1:this->printString("1200 Bauds");break;
				case 2:this->printString("4800 Bauds");break;
				case 3:this->printString("9600 Bauds");break;
			}
			
			this->moveCursor(6,4);
			this->selectLineDynamic(false);
			this->printString("format:");
			this->printString(this->parameters.parity?"7bits+parit\x19\x42""e":"7bits");
			
			this->moveCursor(7,6);
			this->selectLineDynamic(false);
			this->printString("ping:");
			switch (this->parameters.ping){
				case 0:this->printString("d\x19\x42""esactiv\x19\x42""e");break;
				case 1:this->printString("15 secondes");break;
				case 2:this->printString("30 secondes");break;
				case 3:this->printString("45 secondes");break;
			}
			
			this->moveCursor(8,1);
			this->selectLineDynamic(false);
			this->printString("affichage:");
			//this->printString(this->parameters.c80?"mixte":"vid\x19\x42""eotex");
			switch (this->parameters.display){
				case 0:this->printString("page,vid\x19\x42""eotex");break;
				case 1:this->printString("page,t\x19\x42""el\x19\x42""einformatique");break;
				case 2:this->printString("rouleau,vid\x19\x42""eotex");break;
				case 3:this->printString("rouleau,t\x19\x42""el\x19\x42""einformatique");break;
			}
			
			this->moveCursor(9,6);
			this->selectLineDynamic(false);
			this->printString("\x19\x42""echo:");
			this->printString(this->parameters.echo?"oui":"non");
			
			this->moveCursor(10,3);
			this->selectLineDynamic(false);
			this->printString("clavier:");
			this->printString(this->parameters.extendedKeyboard?"\x19\x42""etendu":"simple");
			
			this->moveCursor(11,5);
			this->selectLineDynamic(false);
			this->printString("casse:");
			this->printString(this->parameters.upperCase?"majuscule":"minuscule");
			
			this->moveCursor(12,3);
			this->selectLineDynamic(false);
			this->printString("curseur:");
			this->printString(this->parameters.cursor?"visible":"invisible");
			
			this->printLineSeparator(14);
			
			//this->printContextMenu(this->parameters.currentLine);
			
			this->printLineSeparator(21);
			
			this->moveCursor(22,17);
			this->printString("\x1B\x45"" ligne suivante""\x1B\x5A"" ""\x1B\x5D"" Suite  ");
			
			this->moveCursor(23,15);
			this->printString("\x1B\x45"" ligne pr\x19\x42""ec\x19\x42""edente""\x1B\x5A"" ""\x1B\x5D"" Retour ");
			
			this->moveCursor(24,23);
			this->print(foreground_color_magenta,sizeof(foreground_color_magenta)/sizeof(foreground_color_magenta[0]));
			this->printString("connexion");
			this->print(underline,sizeof(underline)/sizeof(underline[0]));
			this->printString(" ");
			this->print(swap_color,sizeof(swap_color)/sizeof(swap_color[0]));
			this->printString(" Envoi  ");
		}
		
		void printContextMenu(unsigned char line){
			this->moveCursor(17,16);
			this->print(foreground_color_black,sizeof(foreground_color_black)/sizeof(foreground_color_black[0]));////TODO: to avoid flashing background when changing context menu ???
			this->print(swap_color,sizeof(swap_color)/sizeof(swap_color[0]));
			this->printString("\x18\x0A\x18\x0A\x18\x0A\x18");
			//this->printString(" \x12\x7F""\x12\x7F""\x12\x7F""\x12\x7F""\x12\x5B");//7*40
			this->moveCursor(15,1);
			if (line==0){
				this->printString("TAPEZ L'URL DU WEBSOCKET\x18");
			}
			else{
				this->printString("DEFILEZ LES CHOIX:\x18");
				this->print(foreground_color_magenta,sizeof(foreground_color_magenta)/sizeof(foreground_color_magenta[0]));
				this->print(swap_color,sizeof(swap_color)/sizeof(swap_color[0]));
				this->printString("   Espace   ");
			}
			switch (line){
				case 1:
					this->moveCursor(17,15);
					this->selectLineDynamic(this->parameters.baudrate==0);
					this->printString("300 Bauds");
					this->moveCursor(18,15);
					this->selectLineDynamic(this->parameters.baudrate==1);
					this->printString("1200 Bauds");
					this->moveCursor(19,15);
					this->selectLineDynamic(this->parameters.baudrate==2);
					this->printString("4800 Bauds");
					this->moveCursor(20,15);
					this->selectLineDynamic(this->parameters.baudrate==3);
					this->printString("9600 Bauds");
					break;
				case 2:
					this->moveCursor(17,15);
					this->selectLineDynamic(!this->parameters.parity);
					this->printString("7bits");
					this->moveCursor(18,15);
					this->selectLineDynamic(this->parameters.parity);
					this->printString("7bits+parit\x19\x42""e");
					break;
				case 3:
					this->moveCursor(17,15);
					this->selectLineDynamic(this->parameters.ping==0);
					this->printString("d\x19\x42""esactiv\x19\x42""e");
					this->moveCursor(18,15);
					this->selectLineDynamic(this->parameters.ping==1);
					this->printString("15 secondes");
					this->moveCursor(19,15);
					this->selectLineDynamic(this->parameters.ping==2);
					this->printString("30 secondes");
					this->moveCursor(20,15);
					this->selectLineDynamic(this->parameters.ping==3);
					this->printString("45 secondes");
					break;
				case 4:
					this->moveCursor(17,15);
					this->selectLineDynamic(this->parameters.display==0);
					this->printString("page,vid\x19\x42""eotex");
					this->moveCursor(18,15);
					this->selectLineDynamic(this->parameters.display==1);
					this->printString("page,t\x19\x42""el\x19\x42""einformatique");
					this->moveCursor(19,15);
					this->selectLineDynamic(this->parameters.display==2);
					this->printString("rouleau,vid\x19\x42""eotex");
					this->moveCursor(20,15);
					this->selectLineDynamic(this->parameters.display==3);
					this->printString("rouleau,t\x19\x42""el\x19\x42""einformatique");
					break;
				case 5:
					this->moveCursor(17,15);
					this->selectLineDynamic(!this->parameters.echo);
					this->printString("non");
					this->moveCursor(18,15);
					this->selectLineDynamic(this->parameters.echo);
					this->printString("oui");
					break;
				case 6:
					this->moveCursor(17,15);
					this->selectLineDynamic(!this->parameters.extendedKeyboard);
					this->printString("simple");
					this->moveCursor(18,15);
					this->selectLineDynamic(this->parameters.extendedKeyboard);
					this->printString("\x19\x42""etendu");
					break;
				case 7:
					this->moveCursor(17,15);
					this->selectLineDynamic(!this->parameters.upperCase);
					this->printString("minuscule");
					this->moveCursor(18,15);
					this->selectLineDynamic(this->parameters.upperCase);
					this->printString("majuscule");
					break;
				case 8:
					this->moveCursor(17,15);
					this->selectLineDynamic(!this->parameters.cursor);
					this->printString("invisible");
					this->moveCursor(18,15);
					this->selectLineDynamic(this->parameters.cursor);
					this->printString("visible");
					break;
			}
			
		}
		void cycleParameterOption(unsigned char line){//TODO
			switch (line){
				case 1://baudrate
					this->moveCursor(17+this->parameters.baudrate,15);
					this->selectLineDynamic(false);
					this->parameters.baudrate=(this->parameters.baudrate+1)%4;
					this->moveCursor(17+this->parameters.baudrate,15);
					this->selectLineDynamic(true);
					this->moveCursor(5,12);
					this->print(foreground_color_black,sizeof(foreground_color_black)/sizeof(foreground_color_black[0]));
					this->print(swap_color,sizeof(swap_color)/sizeof(swap_color[0]));
					switch (this->parameters.baudrate){
						case 0:this->printString("300 Bauds\x18");break;
						case 1:this->printString("1200 Bauds\x18");break;
						case 2:this->printString("4800 Bauds\x18");break;
						case 3:this->printString("9600 Bauds\x18");break;
					}
					break;
				case 2://parity
					this->parameters.parity=!this->parameters.parity;
					this->moveCursor(6,12);
					this->print(foreground_color_black,sizeof(foreground_color_black)/sizeof(foreground_color_black[0]));
					this->print(swap_color,sizeof(swap_color)/sizeof(swap_color[0]));
					this->printString(this->parameters.parity?"7bits+parit\x19\x42""e\x18":"7bits\x18");
					this->moveCursor(17,15);
					this->selectLineDynamic(!this->parameters.parity);
					this->moveCursor(18,15);
					this->selectLineDynamic(this->parameters.parity);
					break;
				case 3://ping
					this->moveCursor(17+this->parameters.ping,15);
					this->selectLineDynamic(false);
					this->parameters.ping=(this->parameters.ping+1)%4;
					this->moveCursor(17+this->parameters.ping,15);
					this->selectLineDynamic(true);
					this->moveCursor(7,12);
					this->print(foreground_color_black,sizeof(foreground_color_black)/sizeof(foreground_color_black[0]));
					this->print(swap_color,sizeof(swap_color)/sizeof(swap_color[0]));
					switch (this->parameters.ping){
						case 0:this->printString("d\x19\x42""esactiv\x19\x42""e\x18");break;
						case 1:this->printString("15 secondes\x18");break;
						case 2:this->printString("30 secondes\x18");break;
						case 3:this->printString("45 secondes\x18");break;
					}
					break;
				case 4://display
					/*this->parameters.c80=!this->parameters.c80;
					this->moveCursor(8,12);
					this->print(foreground_color_black,sizeof(foreground_color_black)/sizeof(foreground_color_black[0]));
					this->print(swap_color,sizeof(swap_color)/sizeof(swap_color[0]));
					this->printString(this->parameters.c80?"mixte\x18":"vid\x19\x42""eotex\x18");
					this->moveCursor(16,15);
					this->selectLineDynamic(!this->parameters.c80);
					this->moveCursor(17,15);
					this->selectLineDynamic(this->parameters.c80);*/
					this->moveCursor(17+this->parameters.display,15);
					this->selectLineDynamic(false);
					this->parameters.display=(this->parameters.display+1)%4;
					this->moveCursor(17+this->parameters.display,15);
					this->selectLineDynamic(true);
					this->moveCursor(8,12);
					this->print(foreground_color_black,sizeof(foreground_color_black)/sizeof(foreground_color_black[0]));
					this->print(swap_color,sizeof(swap_color)/sizeof(swap_color[0]));
					switch (this->parameters.display){
						case 0:this->printString("page,vid\x19\x42""eotex\x18");break;
						case 1:this->printString("page,t\x19\x42""el\x19\x42""einformatique\x18");break;
						case 2:this->printString("rouleau,vid\x19\x42""eotex\x18");break;
						case 3:this->printString("rouleau,t\x19\x42""el\x19\x42""einformatique\x18");break;
					}
					break;
				case 5://echo
					this->parameters.echo=!this->parameters.echo;
					this->moveCursor(9,12);
					this->print(foreground_color_black,sizeof(foreground_color_black)/sizeof(foreground_color_black[0]));
					this->print(swap_color,sizeof(swap_color)/sizeof(swap_color[0]));
					this->printString(this->parameters.echo?"oui\x18":"non\x18");
					this->moveCursor(17,15);
					this->selectLineDynamic(!this->parameters.echo);
					this->moveCursor(18,15);
					this->selectLineDynamic(this->parameters.echo);
					break;
				case 6://keyboard
					this->parameters.extendedKeyboard=!this->parameters.extendedKeyboard;
					this->moveCursor(10,12);
					this->print(foreground_color_black,sizeof(foreground_color_black)/sizeof(foreground_color_black[0]));
					this->print(swap_color,sizeof(swap_color)/sizeof(swap_color[0]));
					this->printString(this->parameters.extendedKeyboard?"\x19\x42""etendu\x18":"simple\x18");
					this->moveCursor(17,15);
					this->selectLineDynamic(!this->parameters.extendedKeyboard);
					this->moveCursor(18,15);
					this->selectLineDynamic(this->parameters.extendedKeyboard);
					break;
				case 7://case
					this->parameters.upperCase=!this->parameters.upperCase;
					this->moveCursor(11,12);
					this->print(foreground_color_black,sizeof(foreground_color_black)/sizeof(foreground_color_black[0]));
					this->print(swap_color,sizeof(swap_color)/sizeof(swap_color[0]));
					this->printString(this->parameters.upperCase?"majuscule\x18":"minuscule\x18");
					this->moveCursor(17,15);
					this->selectLineDynamic(!this->parameters.upperCase);
					this->moveCursor(18,15);
					this->selectLineDynamic(this->parameters.upperCase);
					break;
				case 8://cursor
					this->parameters.cursor=!this->parameters.cursor;
					this->moveCursor(12,12);
					this->print(foreground_color_black,sizeof(foreground_color_black)/sizeof(foreground_color_black[0]));
					this->print(swap_color,sizeof(swap_color)/sizeof(swap_color[0]));
					this->printString(this->parameters.cursor?"visible\x18":"invisible\x18");
					this->moveCursor(17,15);
					this->selectLineDynamic(!this->parameters.cursor);
					this->moveCursor(18,15);
					this->selectLineDynamic(this->parameters.cursor);
					break;
			}
		}
};

class SimplifiedMinitelNetworkAppPrinter: public SimplifiedMinitelNetworkApp{
	public:
		std::atomic_bool fr=true;//ASCII or ISO/CEI 646 FR
		std::atomic_bool activated=true;
		enum Const{
			NOT_CMD=0x00,
			CMD_ONGOING=0x01,
			CMD_FINISHED=0x02
		};
		
		virtual void PWRCallback(bool b) override final{
			if (this->PWR&&!b) this->PTout=true;
			this->PWR=b;
		}
		
		virtual void PTCallback(bool b) override final{
			this->PTin=b;
		}
		virtual void RxCallback(unsigned char d) override final{
			constexpr unsigned short P=0b0110100110010110;
			d=((bool)(((P>>(d&0x0F))^(P>>(d>>4)))&0x01))?0x1A:d&0x7F;
			this->CMDBuffer.push_back(d);
			unsigned char cmd=this->NOT_CMD;
			
			resync:
			if (!this->PTout){
				if (!(bool)(cmd&this->CMD_FINISHED)){
					cmd|=this->isPrintCMD();
					if ((bool)(cmd&this->CMD_FINISHED)){
						this->printPage();
					}
				}
				if (!(bool)(cmd&this->CMD_FINISHED)){
					cmd|=this->isILCPrinterCMD()|this->isCDGCMD();
					if ((bool)(cmd&this->CMD_FINISHED)){
						this->PTout=true;
					}
				}
			}
			else{
				if (!(bool)(cmd&this->CMD_FINISHED)){
					cmd|=this->isDCPrinterCMD();
					if (this->activated.load(std::memory_order_relaxed)&&(bool)(cmd&this->CMD_FINISHED)){
						this->PTout=false;
						this->TxBuffer.push(0x1B);
						this->TxBuffer.push(0x21);
						this->TxBuffer.push(0x39);
					}
				}
			}
			cmd|=this->isACKCMD()|isISO2022CMD();
			
			if (cmd==this->NOT_CMD){
				if (this->CMDBuffer.size()>1){
					this->CMDBuffer.clear();
					this->CMDBuffer.push_back(d);
					goto resync;
				}
				this->CMDBuffer.clear();
			}
			if (cmd==this->CMD_FINISHED) this->CMDBuffer.clear();
			
			if (!(bool)cmd){
				if (d==0x0C) this->PrintBuffer.clear();
				else{
					if (this->PrintBuffer.size()>=2&&d==0x0A&&this->PrintBuffer[this->PrintBuffer.size()-1]==0x0D&&this->PrintBuffer[this->PrintBuffer.size()-2]==0x08){//BS CR LF -> CR LF
						this->PrintBuffer.erase(this->PrintBuffer.end()-2);
					}
					if (this->fr.load(std::memory_order_relaxed)) this->printISO646FRChar(d);
					else this->printASCIIChar(d);
				}
			}
		}
		
		virtual bool TxEmpty() override final{
			return this->TxBuffer.empty();
		}
		
		virtual unsigned char TxPop() override final{
			unsigned char d=this->TxBuffer.front();
			constexpr unsigned short P=0b0110100110010110;
			d^=((P>>(d&0x0F))^(P>>(d>>4)))<<7;
			this->TxBuffer.pop();
			return d;
		}
		
		void subscribePrintFinished(std::function<void(const char*)> f){
			this->printFinished=f;
		}
		
	private:
		bool PTin=false;
		bool PWR=false;
		std::vector<unsigned char> CMDBuffer;
		std::vector<unsigned char> PrintBuffer;
		std::queue<unsigned char> TxBuffer;
		std::function<void(const char*)> printFinished=[](const char* p){};
		
		unsigned char isACKPrintCMD(){
			switch (this->CMDBuffer.size()){
				case 0:break;
				case 1:
					if (this->CMDBuffer[0]!=0x13) return this->NOT_CMD;
					break;
				case 2:
					if (this->CMDBuffer[0]!=0x13||this->CMDBuffer[1]!=0x5C) return this->NOT_CMD;
					else return this->CMD_FINISHED;
				default:
					return this->NOT_CMD;
			}
			return this->CMD_ONGOING;
		}
		
		unsigned char isDCPrinterCMD(){
			switch (this->CMDBuffer.size()){
				case 0:break;
				case 1:
					if (this->CMDBuffer[0]!=0x1B) return this->NOT_CMD;
					break;
				case 2:
					if (this->CMDBuffer[0]!=0x1B||this->CMDBuffer[1]!=0x21) return this->NOT_CMD;
					break;
				default:
					if (this->CMDBuffer[0]==0x1B&&this->CMDBuffer[1]==0x21&&this->CMDBuffer[2]==0x38) return this->CMD_FINISHED;
					return this->NOT_CMD;
					break;
			}
			return this->CMD_ONGOING;
		}
		
		unsigned char isILCPrinterCMD(){
			switch (this->CMDBuffer.size()){
				case 0:break;
				case 1:
					if (this->CMDBuffer[0]!=0x1B) return this->NOT_CMD;
					break;
				case 2:
					if (this->CMDBuffer[0]!=0x1B||this->CMDBuffer[1]!=0x21) return this->NOT_CMD;
					break;
				default:
					if (this->CMDBuffer[0]==0x1B&&this->CMDBuffer[1]==0x21&&this->CMDBuffer[2]==0x3A) return this->CMD_FINISHED;
					return this->NOT_CMD;
					break;
			}
			return this->CMD_ONGOING;
		}
		
		unsigned char isCDGCMD(){
			switch (this->CMDBuffer.size()){
				case 0:break;
				case 1:
					if (this->CMDBuffer[0]!=0x1B) return this->NOT_CMD;
					break;
				case 2:
					if (this->CMDBuffer[0]!=0x1B||this->CMDBuffer[1]!=0x22) return this->NOT_CMD;
					break;
				case 3:
					if (this->CMDBuffer[0]==0x1B&&this->CMDBuffer[1]==0x22&&this->CMDBuffer[2]==0x3C) return this->CMD_FINISHED;
					if (this->CMDBuffer[0]!=0x1B||this->CMDBuffer[1]!=0x22||(this->CMDBuffer[2]&0xF0)!=0x20) return this->NOT_CMD;
					break;
				default:
					if (this->CMDBuffer[0]!=0x1B||this->CMDBuffer[1]!=0x22||(this->CMDBuffer[2]&0xF0)!=0x20||this->CMDBuffer[3]!=0x3C) return this->NOT_CMD;
					else return this->CMD_FINISHED;
					break;
			}
			return this->CMD_ONGOING;
		}
		
		unsigned char isPrintCMD(){
			switch (this->CMDBuffer.size()){
				case 0:break;
				case 1:
					if (this->CMDBuffer[0]!=0x1B) return this->NOT_CMD;
					break;
				case 2:
					if (this->CMDBuffer[0]!=0x1B||this->CMDBuffer[1]!=0x35) return this->NOT_CMD;
					break;
				default:
					if (this->CMDBuffer[0]==0x1B&&this->CMDBuffer[1]==0x35&&this->CMDBuffer[2]==0x40) return this->CMD_FINISHED;
					return this->NOT_CMD;
					break;
			}
			return this->CMD_ONGOING;
		}
		
		unsigned char isACKCMD(){
			if (this->CMDBuffer.size()<2){
				if (this->CMDBuffer[0]==0x13){
					return this->CMD_ONGOING;
				}
			}
			else if (this->CMDBuffer.size()==2){
				if (this->CMDBuffer[0]==0x13){
					return this->CMD_FINISHED;
				}
			}
			return this->NOT_CMD;
		}
		
		unsigned char isISO2022CMD(){
			switch(this->CMDBuffer.size()){
				case 1:
					if (this->CMDBuffer[0]==0x1B) return this->CMD_ONGOING;
					break;
				case 2:
					if (this->CMDBuffer[0]==0x1B&&(this->CMDBuffer[1]&0xF0)==0x20) return this->CMD_ONGOING;
					break;
				default:
					if (this->CMDBuffer[0]==0x1B){
						for (size_t i=1;i<this->CMDBuffer.size()-1;i++){
							if ((this->CMDBuffer[i]&0xF0)!=0x20) return this->NOT_CMD;
						}
						switch (this->CMDBuffer[this->CMDBuffer.size()-1]){
							case 0x20 ... 0x2F:return this->CMD_ONGOING;
							case 0x30 ... 0x7E:return this->CMD_FINISHED;
						}
					}
					break;
			}
			return this->NOT_CMD;
		}
		
		void printPage(){
			this->PrintBuffer.push_back(0);
			this->printFinished((char*)this->PrintBuffer.data());
		}
		
		void printASCIIChar(unsigned char c){
			this->PrintBuffer.push_back(c);//7bit ASCII is compatible with UTF-8
		}
		
		void printISO646FRChar(unsigned char c){
			switch (c){
				case 0x23:
					this->PrintBuffer.push_back(0xC2);
					this->PrintBuffer.push_back(0xA3);
					break;
				case 0x27:
					this->PrintBuffer.push_back(0xE2);
					this->PrintBuffer.push_back(0x80);
					this->PrintBuffer.push_back(0x99);
					break;
				case 0x40:
					this->PrintBuffer.push_back(0xC3);
					this->PrintBuffer.push_back(0xA0);
					break;
				case 0x5B:
					this->PrintBuffer.push_back(0xC2);
					this->PrintBuffer.push_back(0xB0);
					break;
				case 0x5C:
					this->PrintBuffer.push_back(0xC3);
					this->PrintBuffer.push_back(0xA7);
					break;
				case 0x5D:
					this->PrintBuffer.push_back(0xC2);
					this->PrintBuffer.push_back(0xA7);
					break;
				case 0x60:
					this->PrintBuffer.push_back(0xC2);
					this->PrintBuffer.push_back(0xB5);
					break;
				case 0x7B:
					this->PrintBuffer.push_back(0xC3);
					this->PrintBuffer.push_back(0xA9);
					break;
				case 0x7C:
					this->PrintBuffer.push_back(0xC3);
					this->PrintBuffer.push_back(0xB9);
					break;
				case 0x7D:
					this->PrintBuffer.push_back(0xC3);
					this->PrintBuffer.push_back(0xA8);
					break;
				case 0x7E:
					this->PrintBuffer.push_back(0xC2);
					this->PrintBuffer.push_back(0xA8);
					break;
				default:this->PrintBuffer.push_back(c);break;
			}
		}
		
};
class SimplifiedMinitelNetworkAppAutoStart: public SimplifiedMinitelNetworkApp{
	public:
		std::atomic_bool autoStart=false;
	
		virtual void CLKCallback() final override{
			if ((bool)this->stop_delay_cnt){
				this->stop_delay_cnt--;
				if (!(bool)this->stop_delay_cnt){
					this->PTout=true;
				}
			}
		}
		virtual void RxCallback(unsigned char d) final override{
			if (this->starting&&this->PTin){
				if (d==0x72&&this->prevRx==0x93){
					this->transmission_index=0;
					this->starting=false;
					this->PTout=false;
				}
				this->prevRx=d;
			}
		}
		virtual void TxQueueEmptyCallback() final override{
			if (this->TxEmpty()) this->stop_delay_cnt=this->stop_delay_50ms;
		}
		virtual void PTCallback(bool b) final override{
			if (b&&!this->PTin) this->prevRx=0;
			this->PTin=b;
		}
		virtual void PWRCallback(bool b) final override{
			if (b&&!this->PWR&&this->autoStart.load(std::memory_order_relaxed)){
				this->starting=true;
				this->PTout=true;
				this->stop_delay_cnt=0;
			}
			this->PWR=b;
		};
		virtual bool TxEmpty() final override{
			return this->transmission_index>=sizeof(this->cmd)/sizeof(this->cmd[0]);
		}
		virtual unsigned char TxPop() final override{
			if (this->TxEmpty()){
				return 0;
			}
			else{
				return this->cmd[this->transmission_index++];
			}
		}
	private:
		//bool PTout=true;
		bool PTin=false;
		bool PWR=false;
		bool starting=false;
		
		unsigned char prevRx=0;
		
		constexpr static unsigned char cmd[]={0x1B,0xBB,0x6A,0xD8,0x41};
		
		unsigned char transmission_index=0;//ensure don't exit before sending cmd
		
		constexpr static unsigned short stop_delay_50ms=480;
		unsigned short stop_delay_cnt=0;
};

#endif