#include <Arduino.h>
#include "bcbaws.h"
#include "bcbsdcard.h"
#include "time.h"
#include "WiFiCred1.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include "State.h"

#define ARDUINO_RUNNING_CORE 1

// internal rtc variables
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -18000;
const int daylightOffset_sec = 3600;


// localtime from internal rtc
String printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return ("");
  }
  return (asctime(&timeinfo));
}

String printLocalHour() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return ("");
  }
  return (String(asctime(&timeinfo)).substring(11, 16) + " ");
}

// define functions
void TaskRelay(void *pvParameters); // maintains the websocket display
void initWiFi();
void initTime();

void setup() {
  // start the serial interface
  Serial.begin(115200);
  delay(500);
  initWiFi();
  initTime();
  initWebServer();
  initWebSocket();
  initSDCard();
  checkForIndex();
  setState();
  state.reload = true;

  xTaskCreatePinnedToCore(TaskRelay, "TaskRelay" // A name just for humans
                          ,
                          4096 // This stack size can be checked & adjusted by
                          // reading the Stack Highwater
                          ,
                          NULL, 2 // Priority, with 3 (configMAX_PRIORITIES - 1)
                          // being the highest, and 0 being the lowest.
                          ,
                          NULL, ARDUINO_RUNNING_CORE);


  ArduinoOTA
  .onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS
    SPIFFS.end();
 // Disable client connections    
    ws.enable(false);

    // Advertise connected clients what's going on
    ws.textAll("OTA Update Started");

    // Close them
    ws.closeAll();
    state.ota = true;

    
    Serial.println("Start updating " + type);
  })
  .onEnd([]() {
    Serial.println("\nEnd");
  })
  .onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  })
  .onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR)
      Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR)
      Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR)
      Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR)
      Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR)
      Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  ArduinoOTA.handle(); // check if an update is available
  if (WiFi.status() != WL_CONNECTED) // check if wifi is still connected, if not, reconnect
    initWiFi();
  vTaskDelay(60);
}

void TaskRelay(void *pvParameters) { // handle websocket and oled displays
  (void)pvParameters;
  for (;;) {
    if(!state.ota) {notifyInitialClients(getJson(true));} // send state to the client as a json string
    vTaskDelay(1000);
  }
}


void initWiFi() {
  Serial.println("connecting to wifi");
  // connect to wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
  Serial.println("connecting to wifi failed");
  delay(500);
  initWiFi();
  }
  Serial.println("wifi connected");
  wifiavail = true;
}

void initTime() {
  // set the clock from ntp server
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
}
