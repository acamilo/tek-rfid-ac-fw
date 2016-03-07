#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include "wifi-pass.h"

#define LED_PIN 13 // GPIO2
#define WIEG_ZERO	14
#define WIEG_ONE	4

Timer procTimer;
bool state = true;

// Will be called when WiFi station was connected to AP
void connectOk()
{
	debugf("I'm CONNECTED");
	Serial.println(WifiStation.getIP().toString());


}

// Will be called when WiFi station timeout was reached
bool aptoggle = false;
void connectFail()
{
	debugf("I'm NOT CONNECTED!");
	if (aptoggle){
		aptoggle = false;
		WifiStation.config(WIFI_SSID, WIFI_PWD); // Put you SSID and Password here
		WifiStation.waitConnection(connectOk, 10, connectFail); // Repeat and check again
	} else {
		aptoggle = true;
		WifiStation.config(WIFI_SSID2, WIFI_PWD2); // Put you SSID and Password here
		WifiStation.waitConnection(connectOk, 10, connectFail); // Repeat and check again
	}
}

// Will be called when WiFi hardware and software initialization was finished
// And system initialization was completed
void ready()
{
	debugf("READY!");

	// If AP is enabled:
	debugf("AP. ip: %s mac: %s", WifiAccessPoint.getIP().toString().c_str(), WifiAccessPoint.getMAC().c_str());
}


typedef enum {OPEN, VERIFYING, REACTIVATE, IDLE} readerState;
readerState myState;
int bitcnt = 0;
char bitdata[100];
int timeout=0;

bool zeroskp = true;
void readerZeroAsserted(){
	if (zeroskp){
	bitdata[bitcnt] = '0';
	bitcnt++;
	timeout=0;
	zeroskp=false;
	} else {
		zeroskp = true;
	}
	delayMicroseconds(101);
}

bool oneskp = true;
void readerOneAsserted(){
	if (oneskp){
	bitdata[bitcnt] = '1';
	bitcnt++;
	timeout=0;
	oneskp=false;
	} else {
		oneskp=true;
	}
	delayMicroseconds(101);
}

bool dbm=false;

void readerLogic(){
	//digitalWrite(WIEG_ONE,state);
	//state = !state;
	switch(myState){
			// turn interrupts back on and reset.
		case REACTIVATE:
			if (dbm) {  dbm=false; debugf("[Reader]\tACTIVATING"); }
			ETS_GPIO_INTR_ENABLE();
			bitcnt = 0;
			timeout = 0;
			for (int i=0; i<sizeof(bitdata); i++) bitdata[i]=0;
			myState = IDLE; dbm =true;
			break;

		case IDLE:
			if (dbm) {  dbm=false; debugf("[Reader]\tIDLE"); }
			if (bitcnt>0 && timeout>10){
				myState = VERIFYING;  dbm =true;
				ETS_GPIO_INTR_DISABLE();
				break;
			}
			timeout++;
			break;

		case VERIFYING:
			if (dbm) {  dbm=false; debugf("[Reader]\tVERIFYING"); }
			Serial.print("\tCardID:\t");
			Serial.print(bitdata);
			Serial.print(" (");
			Serial.print(bitcnt);
			Serial.println(")");

			timeout = 0;
			myState = OPEN;  dbm =true;

			break;
		case OPEN:
			if (dbm) {  dbm=false; debugf("[Reader]\tOPEN"); }
			timeout++;
			if (timeout > 100){
				myState = REACTIVATE;  dbm =true;
				timeout = 0;
			}
			break;
	}
}

void init()
{	// 000100010110110011111011000010100
	// 0001000101101100111110110000101100
	// 0001000101101100111110110000101100
	// 0001000101101100111110011000010100
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

	pinMode(WIEG_ZERO,INPUT);
	pinMode(WIEG_ONE ,INPUT);

	attachInterrupt(WIEG_ZERO, readerZeroAsserted, LOW);
	attachInterrupt(WIEG_ONE,  readerOneAsserted, LOW);

	myState = REACTIVATE;
	procTimer.initializeMs(100, readerLogic).start();

}
