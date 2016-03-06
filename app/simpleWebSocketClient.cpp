#include "simpleWebSocketClient.h"


void onCompleted(TcpClient& client, bool successful)
{

}

void onReadyToSend(TcpClient& client, TcpConnectionEvent sourceEvent)
{
	debugf("onReadyToSend");
	debugf("sourceEvent: %d", sourceEvent);

	if(sourceEvent == eTCE_Connected)
	{
		client.sendString("foo", true);
	}
}

bool onReceive(TcpClient& client, char *buf, int size)
{
	// debug msg
	debugf("onReceive");
	debugf("%s", buf);
}
TcpClient wsTCP(onCompleted, onReadyToSend, onReceive);

void websocket_connect(char *url,char *port){

}

int websocket_hasMessage(){

}

char *websocket_getMessage(){

}

void websocket_sendMessage(char *message){

}
