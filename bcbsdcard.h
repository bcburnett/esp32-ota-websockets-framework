#include "FS.h"
#include "SPIFFS.h"
#include "ArduinoJson.h"
#include "State.h"

#ifndef BCBSD
#define BCBSD

//default html page if not in SPIFFS

// beginning of default web page
const char htmlCode[]PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <!  define meta data >
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <! define the style CSS of your page >
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h1 {font-size: 2.9rem;}
    h2 {font-size: 2.1rem;}
    p {font-size: 1.9rem;}
    body {max-width: 400px; margin:0px auto; padding-bottom: 30px;}
    .slider { -webkit-appearance: none; margin: 14px; width: 400px; height: 15px; border-radius: 5px; background: #39a6de; outline: none; -webkit-transition: .2s; transition: opacity .2s;}
    .slider::-webkit-slider-thumb {-webkit-appearance: none; appearance: none; width: 25px; height: 25px; border-radius: 12px; background: #f74d4d; cursor: pointer;}
    .slider::-moz-range-thumb { width: 25px; height: 25px; border-radius: 12px; background: #F74D4D; cursor: pointer; } 
  </style>
</head>

<body>
 
  <! Edit the pages your heading 2 >
  <h2>ESP32 Multi Slider rev3</h2>
  
  <! Displays the value of slider1 >
  <p><span id="textSliderValue">0</span> &#37</p>
  
  <! displays the range of the slider 0 - 100 in steps of 1 >
  <p><input type="range" oninput="sendMessage('s1'+this.value)" id="pwmSlider" min="0" max="100" value=0 step="1" class="slider"></p>
  
  <! Displays the value of slider2 >
  <p><span id="textSliderValue1">0</span> &#37</p>
  
  <! displays the range of the slider 0 - 100 in steps of 1 >
  <p><input type="range" oninput="sendMessage('s2'+this.value)" id="pwmSlider1" min="0" max="100" value=0 step="1" class="slider"></p>

  <input type='file' name='upload' id='upload' value=''><span id="percent"></span>
<script>
    var gateway = `ws://${window.location.hostname}/ws`;
    var websocket;
    var state = {};
    var json = {};

    window.addEventListener('load', onLoad);

    function initWebSocket() {
      console.log('Trying to open a WebSocket connection...');
      websocket = new WebSocket(gateway);
      websocket.onopen = onOpen;
      websocket.onclose = onClose;
      websocket.onmessage = onMessage; 
    }
    
    function onOpen(event) {
      console.log('Connection opened');
    }

    function onClose(event) {
      console.log('Connection closed');
      setTimeout(initWebSocket, 2000);
    }

    function onMessage(event) {
      json = JSON.parse(event.data);
      console.log(json);
      document.getElementById("textSliderValue").innerHTML = json.slider1;
      document.getElementById("textSliderValue1").innerHTML = json.slider2;
      if(json.initial){
        document.getElementById("pwmSlider").value = json.slider1;
        document.getElementById("pwmSlider1").value = json.slider2;
      }

      if(json.reload) location.reload();
    }

    // on page load
    function onLoad(event) {
    initWebSocket();
    document.getElementById("upload").addEventListener("change", (e) => processFile(e));
    }

  function processFile(e) {
      const reader = new FileReader();
      reader.readAsText(e.path[0].files[0]);
      var myfilename = e.path[0].files[0].name;

      reader.onload = (e) => {
        var myfile = e.target.result;
        var myarray = myfile.split('');
        var mylength = myfile.length;
        i = 0;
        k = 1;
        var sendArray = [];
        sendArray[0] = ("upld:" + myfilename);

        function getChunk() {
          var sendstring = '';
          for (var j = 0; j < 512; j++) {
            if (i < mylength) {
              sendstring += myarray[i];
            }
            i++
          }
          return sendstring;
        }

        function loopThroughSplittedText(splittedText) {
          splittedText.forEach(function (text, i) {
            setTimeout(function () {
              sendMessage(text, i);
            }, i * 500)
          })
        }

        while (i < mylength) {

          sendArray[k] = ("file:" + getChunk());
          k++;
        }
        sendArray[k] = ("comp");
        sendstring = '';
        loopThroughSplittedText(sendArray);
      };
    }

    function sendMessage(data, index) {
      if (index) var percent = index / k * 100;
      if (data.substring(0, 4) == 'file') document.getElementById('percent').innerHTML = percent + ' %';
      if (data.substring(0, 4) == 'comp') {
        document.getElementById('percent').innerHTML = "completed";
        setTimeout(() => {
          sendMessage("reload");
        }, 2000);
      }
      websocket.send(data);
    }


</script>
</body>
</html> 

)rawliteral";

// end of default web page

// variables
xSemaphoreHandle semFile = xSemaphoreCreateMutex(); // file operation lock
#define FORMAT_SPIFFS_IF_FAILED true
void initSDCard() {
  if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
    return;
  }
}

void appendFile(fs::FS &fs, const char * path, const char * message) {

  if (xSemaphoreTake(semFile, 500)) {
    File file = fs.open(path, FILE_APPEND);

    if (!file) {
      return;
    }
    file.print(message);
    file.close();

    xSemaphoreGive(semFile);
  }
}

void writeFile(fs::FS &fs, const char * path, const char * message) {

  if (xSemaphoreTake(semFile, 500)) {
    File file = fs.open(path, FILE_WRITE);

    if (!file) {
      return;
    }
    file.print(message);
    file.close();

    xSemaphoreGive(semFile);
  }
}
String readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\r\n", path);

    File file = fs.open(path);
    if(!file || file.isDirectory()){
        Serial.println("- failed to open file for reading");
        return "";
    }
    String result;
    Serial.println("- read from file:");
    while(file.available()){
        result += String(char(file.read()));
    }
    file.close();
    return result;
}

void renameFile(fs::FS &fs, const char * path1, const char * path2) {
  if (xSemaphoreTake(semFile, 500)) {
    fs.rename(path1, path2);
    xSemaphoreGive(semFile);
  }
}

void deleteFile(fs::FS &fs, const char * path) {
  if (xSemaphoreTake(semFile, 500)) {
    fs.remove(path);
    xSemaphoreGive(semFile);
  }
}

// write the default index.htm to SPIFFS
//  check if index exists and only update if it doesn't
void checkForIndex(){
  if(SPIFFS.exists("/data.json")) {
    state.json = readFile(SPIFFS, "/data.json");
    Serial.println(state.json);
  }
  if(SPIFFS.exists("/index.htm")) return; // 
  deleteFile(SPIFFS,"/index.htm");
  delay(500);
  File file = SPIFFS.open("/index.htm", FILE_APPEND);
    file.print(htmlCode);
    file.close();
    }

#endif
