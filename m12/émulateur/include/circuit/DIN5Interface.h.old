#ifndef DIN5INTERFACE_H
#define DIN5INTERFACE_H

#include <atomic>
#include <mutex>
#include <queue>
#include <functional>

#include "ixwebsocket/IXWebSocketServer.h"

class SimpleDIN5Interface{
	public:
		std::function<void(int)> debug_connection_change=[](int s){};
		
		SimpleDIN5Interface(int port): 
			DIN5server(port, "127.0.0.1", ix::WebSocketServer::kDefaultTcpBacklog, 1)
		{
			//this->DIN5server=ix::WebSocketServer(port, "127.0.0.1", ix::WebSocketServer::kDefaultTcpBacklog, 1);
			
			this->DIN5server.setOnClientMessageCallback([this](std::shared_ptr<ix::ConnectionState> connectionState, ix::WebSocket & webSocket, const ix::WebSocketMessagePtr & msg) {

				if (msg->type == ix::WebSocketMessageType::Open)
				{
					if (!msg->openInfo.uri.compare("/300")){
						this->RxTx_div.store(32,std::memory_order_release);
						printf("Serial connected => 300 Baud\n");
						this->debug_connection_change(300);
					}
					else if (!msg->openInfo.uri.compare("/4800")){
						this->RxTx_div.store(2,std::memory_order_release);
						printf("Serial connected => 4800 Baud\n");
						this->debug_connection_change(4800);
					}
					else if (!msg->openInfo.uri.compare("/9600")){
						this->RxTx_div.store(1,std::memory_order_release);
						printf("Serial connected => 9600 Baud\n");
						this->debug_connection_change(9600);
					}
					else{//1200 by default
						this->RxTx_div.store(8,std::memory_order_release);
						printf("Serial connected => 1200 Baud (default)\n");
						this->debug_connection_change(1200);
					}
					{
						std::lock_guard<std::mutex> lock(this->p_mutex);
						this->pWebSocket=&webSocket;
					}
				}
				else if (msg->type == ix::WebSocketMessageType::Close)
				{
					printf("Serial disconnected\n");
					{
						std::lock_guard<std::mutex> lock(this->p_mutex);
						this->pWebSocket=NULL;
					}
					this->debug_connection_change(0);
				}
				else if (msg->type == ix::WebSocketMessageType::Message)
				{
					
					{
						std::lock_guard<std::mutex> lock(this->q_mutex);
						for (unsigned int i=0;i<msg->str.length();i++) this->q.push((unsigned char)msg->str[i]);
					}
				}
			});

			auto res = this->DIN5server.listen();
			if (!res.first)
			{
				printf("websocket failed to init\n");
				return ;
			}

			this->DIN5server.disablePerMessageDeflate();
			this->DIN5server.start();

			printf("websocket listening at address 127.0.0.1:%i\n",port);
			
		}
		~SimpleDIN5Interface(){
			this->DIN5server.stop();
		}
		void CLKTickIn(){//9600Hz
			this->RxTx_tick=(this->RxTx_tick+1)%(this->RxTx_div.load(std::memory_order_acquire));
			if (this->RxTx_tick==0) {
				this->RxClockTick();
				this->TxClockTick();
			}
		}
		void RxChangeIn(bool b){
			if (b!=this->Rx) this->Rx=b;
		}
		void subscribeTx(std::function<void(bool)> f){
			this->sendTx=f;
		}
	private:
		bool Rx=false;
		bool Rx_prev=false;
		unsigned char RxTx_tick=0;
		std::atomic<unsigned char> RxTx_div=8;
		unsigned char Rx_step=0;
		unsigned char Tx_step=0;
		unsigned char Tx_buf;
		unsigned char Rx_buf;
		ix::WebSocketServer DIN5server;
		std::mutex q_mutex;
		std::queue<unsigned char> q;
		std::function<void(bool)> sendTx=[](bool b){};
		ix::WebSocket* pWebSocket;
		std::mutex p_mutex;
		
		
		void RxClockTick(){
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
						std::lock_guard<std::mutex> lock(this->p_mutex);
						if (this->pWebSocket!=NULL && this->pWebSocket->getReadyState()==ix::ReadyState::Open) this->pWebSocket->sendBinary(std::string(1,(char)this->Rx_buf));
					}
					this->Rx_prev=this->Rx;
					this->Rx_step=0;
					break;
			}
		}
		void TxClockTick(){
			switch (this->Tx_step){
				case 0:
					{
						std::lock_guard<std::mutex> lock(this->q_mutex);
						if (!this->q.empty()){
							this->Tx_buf=this->q.front();
							this->q.pop();
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