#ifndef __SIMPLEWEBSOCKETCLIENT_H__
#define __SIMPLEWEBSOCKETCLIENT_H__

#include <user_config.h>
#include <SmingCore/SmingCore.h>

void websocket_connect(char *url,char *port);
int  websocket_hasMessage();
char *websocket_getMessage();
void websocket_sendMessage(char *message);

void onCompleted(TcpClient& client, bool successful);
void onReadyToSend(TcpClient& client, TcpConnectionEvent sourceEvent);
bool onReceive(TcpClient& client, char *buf, int size);

#endif
