#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include "wifi-pass.h"

#define LED_PIN 13 // GPIO2
#define WIEG_ZERO	10
#define WIEG_ONE	11

Timer procTimer;
bool state = true;

// Will be called when WiFi station was connected to AP
void connectOk()
{
	debugf("I'm CONNECTED");
	Serial.println(WifiStation.getIP().toString());


}

// Will be called when WiFi station timeout was reached
void connectFail()
{
	debugf("I'm NOT CONNECTED!");
	WifiStation.config(WIFI_SSID2, WIFI_PWD2); // Put you SSID and Password here
	WifiStation.waitConnection(connectOk, 10, connectFail); // Repeat and check again
}

// Will be called when WiFi hardware and software initialization was finished
// And system initialization was completed
void ready()
{
	debugf("READY!");

	// If AP is enabled:
	debugf("AP. ip: %s mac: %s", WifiAccessPoint.getIP().toString().c_str(), WifiAccessPoint.getMAC().c_str());
}

void blink()
{
	digitalWrite(LED_PIN, state);
	state = !state;
}


typedef enum {OPEN, VERIFYING, REACTIVATE, IDLE} readerState;
readerState myState;
int bitcnt = 0;
char bitdata[100];
int timeout=0;

void readerZeroAsserted(){
	bitdata[bitcnt] = '0';
	bitcnt++;
	timeout=0;
}

void readerOneAsserted(){
	bitdata[bitcnt] = '1';
	bitcnt++;
	timeout=0;
}

void readerLogic(){
	switch(myState){
			// turn interrupts back on and reset.
		case REACTIVATE:
			debugf("[Reader]\tACTIVATING");
			ETS_GPIO_INTR_ENABLE();
			bitcnt = 0;
			timeout = 0;
			for (int i=0; i<sizeof(bitdata); i++) bitdata[0]=0;
			myState = IDLE;
			break;

		case IDLE:
			debugf("[Reader]\tIDLE");
			if (bitcnt>0 && timeout>5){
				myState = VERIFYING;
				ETS_GPIO_INTR_DISABLE();
				break;
			}
			timeout++;
			break;

		case VERIFYING:
			debugf("[Reader]\tVERIFYING");

			timeout = 0;
			myState = OPEN;

			break;
		case OPEN:
			debugf("[Reader]\tOPEN");
			timeout++;
			if (timeout > 10){
				myState = REACTIVATE;
			}
			break;
	}
}

void init()
{
	// Set up serial
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true); // Allow debug print to serial
	Serial.println("Sming. Let's do smart things!");

	// Set system ready callback method
	System.onReady(ready);

	// Station - WiFi client
	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD); // Put you SSID and Password here
	WifiStation.waitConnection(connectOk, 30, connectFail);


	attachInterrupt(WIEG_ZERO, readerOneAsserted, FALLING);
	attachInterrupt(WIEG_ONE, readerZeroAsserted, FALLING);

	myState = REACTIVATE;
	procTimer.initializeMs(500, readerLogic).start();

}
