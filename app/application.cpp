#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include "wifi-pass.h"
#include "simpleWebSocketClient.h"

#define LED_PIN 13 // GPIO2

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

	pinMode(LED_PIN, OUTPUT);
	procTimer.initializeMs(5000, blink).start();
}
