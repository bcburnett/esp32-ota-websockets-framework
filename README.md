# ESP32 Async web server / websocket framework example.

 - connects to wifi
 - retrieves time from ntp servers
 - if index.htm is not present in spiffs, it loads a default webpage to spiffs.
 - provides real time bi-directional communication between the clients and the ESP-32
 - a new index.htm can be uploaded from the client interface, and once uploaded, all the clients will refresh to the new page.
 - programming can be updated using the arduino OTA interface.
 - it will maintain state between reboots.

# Files

 - slider-demo2.ino
 - WiFiCred.h
 - bcbaws.h
 - bcbsdcard.h

## slider-demo2.ino

### internal rtc variables
const char *ntpServer = "pool.ntp.org";

const long gmtOffset_sec = -18000;

const int daylightOffset_sec = 3600;

### define functions

void TaskRelay(void *pvParameters); // maintains the websocket display

void initWiFi(); initializes wifi

void initTime(); initializes the real-time clock.

String printLocalHour(); retrieves the time

String printLocalTime(); retrieves the date and time


## WiFiCred.h

put your wifi credentials here

//wifi connection credentials

const char* ssid     = "*****";

const char* password = "*********";

## bcbaws.h

### variables
//set initial state

struct State { 

  int slider1;
  
  int slider2;
  
  String filename;
  
  bool reload;
  
};

State state = {0,0}; 

// web server variables

AsyncWebServer server(80);

AsyncWebSocket ws("/ws");

bool wifiavail = false;

### define functions
String getJson(bool b); creates the json object for sending to the client

void notifyClients(); sends data to clients

void notifyInitialClients; sends data to clients with a flag

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len); handles websocket messages

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
             AwsEventType type, void *arg, uint8_t *data, size_t len);
             
void initWebSocket(); initializes websockets

void initWebServer(); initializes webserver

void parseCommand(String command); parses commands from the clients



## bcbsdcard.h

### variables

const char htmlCode[]PROGMEM default webpage

xSemaphoreHandle semFile = xSemaphoreCreateMutex(); // file 
operation lock

#define FORMAT_SPIFFS_IF_FAILED true


### functions
void initSDCard();

void appendFile(fs::FS &fs, const char * path, const char * message);

void renameFile(fs::FS &fs, const char * path1, const char * path2);

void deleteFile(fs::FS &fs, const char * path);

// write the default index.htm to SPIFFS 

//  check if index exists and only update if it doesn't

void checkForIndex();

## index.htm

### variables
    var gateway = `ws://${window.location.hostname}/ws`;
    
    var websocket;
    
    var state = {};
    
    var json = {};

### functions
    function initWebSocket() opens a connection to the server
    
    function onOpen(event) connection opened
    
    function onClose(event) connection closed
    
    function onMessage(event) message handler
    
    function onLoad(event) page is loaded
    
    function processFile(e) read a text file, break it into 512k chunks and send it to the server
    
    function sendMessage(data, index) send a message to the server.
