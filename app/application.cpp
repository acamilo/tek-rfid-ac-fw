#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include "wifi-pass.h"

#define LED_PIN 13 // GPIO2
#define WIEG_ZERO	14
#define WIEG_ONE	4
#define BEEPER 		5

Timer procTimer;
HttpClient database;

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


typedef enum {OPEN, VERIFYING, REACTIVATE, IDLE, WAITINGFORDB, DENIED} readerState;
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

void dbResponseCallback(HttpClient& client, bool successful)
{
	if (successful){
		Serial.println("Success sent");
		myState = OPEN;  dbm =true;
	}
	else {
		Serial.println("Failed");
		myState = DENIED;  dbm =true;
	}

	String response = client.getResponseString();
	Serial.println("Server response: '" + response + "'");


}

void readerLogic(){
	//digitalWrite(WIEG_ONE,state);
	//state = !state;
	switch(myState){
			// turn interrupts back on and reset.
		case REACTIVATE:
			if (dbm) {  dbm=false; Serial.println("[Reader]\tACTIVATING"); }
			ETS_GPIO_INTR_ENABLE();
			bitcnt = 0;
			timeout = 0;
			for (int i=0; i<sizeof(bitdata); i++) bitdata[i]=0;
			myState = IDLE; dbm =true;
			break;

		case IDLE:
			if (dbm) {  dbm=false; Serial.println("[Reader]\tIDLE"); }
			if (bitcnt>0 && timeout>10){
				myState = VERIFYING;  dbm =true;
				ETS_GPIO_INTR_DISABLE();
				break;
			}
			timeout++;
			break;

		case VERIFYING:
			if (dbm) {  dbm=false; Serial.println("[Reader]\tVERIFYING"); }
			Serial.print("\tCardID:\t");
			Serial.print(bitdata);
			Serial.print(" (");
			Serial.print(bitcnt);
			Serial.println(")");

			database.downloadString( \
					"http://192.168.1.100:8080/"+ \
					String(bitdata) + "/front", dbResponseCallback);

			timeout = 0;
			myState = WAITINGFORDB;  dbm =true;


			break;
		case WAITINGFORDB:
			if (dbm) {  dbm=false; Serial.println("[Reader]\tWAITINGFORDB"); }

			break;
		case OPEN:
			if (dbm) {  dbm=false; Serial.println("[Reader]\tOPEN"); }
			digitalWrite(BEEPER,0);
			timeout++;
			if (timeout>20) digitalWrite(BEEPER,1);
			if (timeout > 100){
				myState = REACTIVATE;  dbm =true;
				timeout = 0;
			}
			break;

		case DENIED:
			if (dbm) {  dbm=false; Serial.println("[Reader]\tDENIED"); }
			state = !state;
			digitalWrite(BEEPER,state);
			timeout++;
			if (timeout > 100){
				digitalWrite(BEEPER,1);
				myState = REACTIVATE;  dbm =true;
				timeout = 0;
			}
			break;
	}
}


void init()
{
	// Set up serial
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(false); // Allow debug print to serial
	Serial.println("Sming. Let's do smart things!");

	// Set system ready callback method
	System.onReady(ready);

	// Station - WiFi client
	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD); // Put you SSID and Password here
	WifiStation.waitConnection(connectOk, 30, connectFail);

	pinMode(WIEG_ZERO,INPUT);
	pinMode(WIEG_ONE ,INPUT);
	pinMode(BEEPER ,OUTPUT);

	digitalWrite(BEEPER,1);

	attachInterrupt(WIEG_ZERO, readerZeroAsserted, LOW);
	attachInterrupt(WIEG_ONE,  readerOneAsserted, LOW);

	myState = REACTIVATE;
	procTimer.initializeMs(100, readerLogic).start();

}
