#ifndef BCBAWS
#define BCBAWS
#include <Arduino.h>
#include "ArduinoJson.h"
#include "AsyncJson.h"
#include <ESPAsyncWebServer.h>
#include "bcbsdcard.h"
#include <SPIFFS.h>
#include "State.h"

// handles state and communication with the client


void parseCommand(String command);



// define functions
String getJson(bool b);


// web server variables
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
bool wifiavail = false;

void notifyClients() {
  ws.textAll(getJson(false));  // send non-slider updating data
}

void notifyInitialClients(const String msg) {
  ws.textAll(msg);  // send slider updating data
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) { // receive the comm from the client and convert it into a string
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len &&
      info->opcode == WS_TEXT) {
    data[len] = 0;
    parseCommand((char *)data);
  }
}


// handle websocket connections
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
             AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      notifyInitialClients(getJson(true));
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

// init websockets
void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

// init webserver
void initWebServer() {
  // Route for root / web page served from spiffs
  // TODO add routines for dynamically updating index.htm
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/index.htm", "text/html");
  });
  // start the web server
  server.begin();
}

// convert state to json
String getJson(bool b) {
  StaticJsonDocument<250> data;
  // struct mp3Status {
  // int slider1;
  // int slider2;
  // };

  data["slider1"] = state.slider1;
  data["slider2"] = state.slider2;
  if (b) {
    data["initial"] = "true";
  }
  if (state.reload) {
    data["reload"] = "true";
    state.reload = false;
  }

  String response;
  serializeJson(data, response);
  state.json = response;
  writeFile(SPIFFS, "/data.json", state.json.c_str());
  return response;
}

// convert json to state
void setState() {
  StaticJsonDocument<250> data;
  // struct mp3Status {
  // int slider1;
  // int slider2;
  // };
  deserializeJson(data, state.json);
  state.slider1 = data["slider1"];
  state.slider2 = data["slider2"];
}

//parse the command coming from the client(s)
void parseCommand(String command) {
  Serial.println(command);

  if (command.substring(0, 2) == "s1") { // slider 1 value
    String val1 = command.substring(2);
    int val2 = atoi((char *)&val1);
    state.slider1 = val2;
    notifyClients();
  }

  if (command.substring(0, 2) == "s2") { // slider 2 value
    String val1 = command.substring(2);
    int val2 = atoi((char *)&val1);
    state.slider2 = val2;
    notifyClients();
  }

  // file upload handler

  if (command.substring(0, 4) == "upld") {
    state.filename = command.substring(5);
    deleteFile(SPIFFS, "/temp.txt");
  }

  if (command.substring(0, 4) == "comp") {
    deleteFile(SPIFFS, ("/" + state.filename).c_str());
    renameFile(SPIFFS, "/temp.txt", ("/" + state.filename).c_str());
    state.filename = "";
  }

  if (command.substring(0, 4) == "file") {
    String message = command.substring(5);
    appendFile(SPIFFS, "/temp.txt", message.c_str());
  }

  if (command == "reload") {
    state.reload = true;
    notifyClients();
    state.reload = false;
  }


}


#endif
