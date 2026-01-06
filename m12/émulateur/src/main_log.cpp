#include "thread_messaging.h"

#include <iostream>

#include <ixwebsocket/IXNetSystem.h>

#include <ixwebsocket/IXWebSocketServer.h>

// 8080:din / 8081:tel in / 8082:tel out

void thread_log_main(Mailbox* p_mb_log,GlobalState* p_gState){
	/*ix::initNetSystem();
	
	int port = 8080;
	std::string host("127.0.0.1");
	ix::WebSocketServer DIN5server(port, host, ix::WebSocketServer::kDefaultTcpBacklog, 1);
	
	DIN5server.setOnClientMessageCallback([](std::shared_ptr<ix::ConnectionState> connectionState, ix::WebSocket & webSocket, const ix::WebSocketMessagePtr & msg) {

		if (msg->type == ix::WebSocketMessageType::Open)
		{
			printf("New connection\n");
			webSocket.close();
		}
		else if (msg->type == ix::WebSocketMessageType::Close)
		{
			printf("End connection\n");
		}
		else if (msg->type == ix::WebSocketMessageType::Message)
		{
			printf("Received: %s\n",msg->str.c_str());

			webSocket.send(msg->str, msg->binary);
		}
	});

	auto res = DIN5server.listen();
	if (!res.first)
	{
		return ;
	}

	DIN5server.disablePerMessageDeflate();
	DIN5server.start();

	printf("websocket listening at address 127.0.0.1:8080\n");
	while (!p_gState->shutdown.load(std::memory_order_relaxed)){
		
	}
	DIN5server.stop();
	
	ix::uninitNetSystem();*/
}