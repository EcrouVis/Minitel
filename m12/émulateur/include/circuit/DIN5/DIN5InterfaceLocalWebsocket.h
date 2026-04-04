#ifndef DIN5INTERFACELOCALWEBSOCKET_H
#define DIN5INTERFACELOCALWEBSOCKET_H
#include "circuit/DIN5/DIN5InterfaceBase.h"
#include <cstdio>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "ixwebsocket/IXWebSocketServer.h"
#include <queue>
#include <mutex>

class DIN5InterfaceLocalWebsocket: public DIN5InterfaceBase{
	public:
		std::atomic<int> baudrate_selection_rx=1;
		std::atomic<int> baudrate_selection_tx=1;
		const int baudrate_div[4] = { 32, 8, 2, 1};
		std::atomic_bool PWR=false;
		
		~DIN5InterfaceLocalWebsocket() override{
			if (this->p_server!=NULL){
				this->p_server->stop();
				delete this->p_server;
			}
		}
		
	private:
		ix::WebSocketServer* p_server=NULL;
		std::mutex pWSMutex;
		ix::WebSocket* pWebSocket;
		std::mutex pMQMutex;
		std::queue<unsigned char> messageQueue;
		
		bool Rx=false;
		bool Rx_prev=false;
		unsigned char Rx_step=0;
		unsigned char Rx_buf;
		unsigned char Tx_step=0;
		unsigned char Tx_buf;
		unsigned char Rx_tick=0;
		unsigned char Tx_tick=0;
		
		void _PWRChangeIn(bool b) override{
			printf("PWR state: %i\n",b);
			this->PWR.store(b,std::memory_order_release);
			if (b&&this->p_server==NULL){
				this->p_server=new ix::WebSocketServer(8080,"0.0.0.0", ix::WebSocketServer::kDefaultTcpBacklog, 1);
				this->p_server->setOnClientMessageCallback([this](std::shared_ptr<ix::ConnectionState> connectionState, ix::WebSocket & webSocket, const ix::WebSocketMessagePtr & msg) {
					if (msg->type == ix::WebSocketMessageType::Open){
						{
							std::lock_guard<std::mutex> lock(this->pWSMutex);
							this->pWebSocket=&webSocket;
						}
						printf("Serial connected\n");
					}
					else if (msg->type == ix::WebSocketMessageType::Close)
					{
						printf("Serial disconnected\n");
						{
							std::lock_guard<std::mutex> lock(this->pWSMutex);
							this->pWebSocket=NULL;
						}
					}
					else if (msg->type == ix::WebSocketMessageType::Message)
					{
						{
							std::lock_guard<std::mutex> lock(this->pMQMutex);
							for (unsigned int i=0;i<msg->str.length();i++) this->messageQueue.push((unsigned char)msg->str[i]);
						}
					}
				});
				auto res = this->p_server->listen();
				if (!res.first)
				{
					delete this->p_server;
					this->p_server=NULL;
					printf("websocket failed to init\n");
					this->plugged.store(false,std::memory_order_release);//unplug if the server failed to initialize
					return ;
				}

				this->p_server->disablePerMessageDeflate();
				this->p_server->start();
				printf("websocket listening at address localhost:8080\n");
			}
			if ((!b)&&this->p_server!=NULL){
				this->p_server->stop();
				delete this->p_server;
				this->p_server=NULL;
				this->messageQueue={};
				this->Rx_step=0;
				this->Tx_step=0;
				/////////////////////////////////////////////////////
			}
		}
		void _RxChangeIn(bool b) override{
			this->Rx=b;
		}
		void _CLKTickIn9600Hz() override{
			if (this->PWR.load(std::memory_order_relaxed)){
				this->Rx_tick=(this->Rx_tick+1)%(this->baudrate_div[this->baudrate_selection_rx.load(std::memory_order_acquire)]);
				if (this->Rx_tick==0) {
					this->_RxClockTick();
				}
				this->Tx_tick=(this->Tx_tick+1)%(this->baudrate_div[this->baudrate_selection_tx.load(std::memory_order_acquire)]);
				if (this->Tx_tick==0) {
					this->_TxClockTick();
				}
			}
		}
		void _RxClockTick(){
			switch (this->Rx_step){
				case 0:
					if (this->Rx_prev&&(!this->Rx)) this->Rx_step++;
					this->Rx_prev=this->Rx;
					break;
				default:
					this->Rx_buf=(this->Rx_buf>>1)|(this->Rx?0x80:0);
					this->Rx_step++;
					break;
				case 9:
					//this->DIN5server.getClients() => uninitializated memory or thread related bug????
					{
						std::lock_guard<std::mutex> lock(this->pWSMutex);
						if (this->pWebSocket!=NULL && this->pWebSocket->getReadyState()==ix::ReadyState::Open) this->pWebSocket->sendBinary(std::string(1,(char)this->Rx_buf));
					}
					this->Rx_prev=this->Rx;
					this->Rx_step=0;
					break;
			}
		}
		void _TxClockTick(){
			switch (this->Tx_step){
				case 0:
					{
						std::lock_guard<std::mutex> lock(this->pMQMutex);
						if (!this->messageQueue.empty()){
							this->Tx_buf=this->messageQueue.front();
							this->messageQueue.pop();
							this->sendTx(false);
							this->Tx_step++;
						}
					}
					break;
				default:
					this->sendTx((bool)(this->Tx_buf&1));
					this->Tx_buf=this->Tx_buf>>1;
					this->Tx_step++;
					break;
				case 9:
					this->sendTx(true);
					this->Tx_step=0;
					break;
			}
		}
};

#endif