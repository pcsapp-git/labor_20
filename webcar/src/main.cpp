#include <Arduino.h>
#include <WiFi.h>
#include <iostream>
#include <sstream>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4
#define STOP 0

#define RIGHT_MOTOR 0
#define LEFT_MOTOR 1

#define FORWARD 1
#define BACKWARD -1

const int PWMFreq = 1000; //1KHZ
const int PWMResolution = 8;
const int PWMSpeedChannel = 4;


const char* ssid = "pcsapp-car";
const char* password = "12345678";

AsyncWebServer server(80);
AsyncWebSocket wsCarInput("/CarInput");

struct MOTOR_PINS
{
  int pinEn;
  int pinIN1;
  int pinIN2;
};

std::vector<MOTOR_PINS> motorPins=
{
  {22,16,17},
  {23,18,19},
};
const char * htmlHomePage PROGMEM = R"HTMLHOMEPAGE(
<!DOCTYPE HTML>
<html>
 <head>
  <style>
       .arrows {
      font-size:40px;
      color:red;
    }
    td.button {
      background-color:black;
      border-radius:25%;
      box-shadow: 5px 5px #888888;
    }
    td.button:active {
      transform: translate(5px,5px);
      box-shadow: none; 
    }

    .noselect {
      -webkit-touch-callout: none; /* iOS Safari */
        -webkit-user-select: none; /* Safari */
         -khtml-user-select: none; /* Konqueror HTML */
           -moz-user-select: none; /* Firefox */
            -ms-user-select: none; /* Internet Explorer/Edge */
                user-select: none; /* Non-prefixed version, currently
                                      supported by Chrome and Opera */
    }

    .slidecontainer {
      width: 100%;
    }

    .slider {
      -webkit-appearance: none;
      width: 100%;
      height: 20px;
      border-radius: 5px;
      background: #d3d3d3;
      outline: none;
      opacity: 0.7;
      -webkit-transition: .2s;
      transition: opacity .2s;
    }

    .slider:hover {
      opacity: 1;
    }
  
    .slider::-webkit-slider-thumb {
      -webkit-appearance: none;
      appearance: none;
      width: 40px;
      height: 40px;
      border-radius: 50%;
      background: red;
      cursor: pointer;
    }

    .slider::-moz-range-thumb {
      width: 40px;
      height: 40px;
      border-radius: 50%;
      background: red;
      cursor: pointer;
    }
  </style>
 </head>
 <body>
<h1 style="color: teal; text-align: center;">ESP32 Web Control Car</h1>
<h2 style="text-align:center">WiFi Controller</h2>
<table id="mainTable" border="1" style="width:400px; margin:auto;table-layout:fixed;text-align:center;"CELLSPACING=10>
  <tr> 
    <td></td>
    <td class="button" ontouchstart='sendButtonInput("MoveCar","1")' ontouchend='sendButtonInput("MoveCar","0")'><span class="arrows">&#8679;</span></td>
    <td></td>
  </tr>
    <tr> 
  <td class="button" ontouchstart='sendButtonInput("MoveCar","3")' ontouchend='sendButtonInput("MoveCar","0")'><span class="arrows">&#8678;</span></td>
    <td class="button"></td>
    <td class="button" ontouchstart='sendButtonInput("MoveCar","4")' ontouchend='sendButtonInput("MoveCar","0")'><span class="arrows">&#8680;</span></td>
  </tr>
   <tr> 
    <td></td>
    <td class="button" ontouchstart='sendButtonInput("MoveCar","2")' ontouchend='sendButtonInput("MoveCar","0")'><span class="arrows">&#8681;</span></td>
    <td></td>
  </tr>
  <tr>
  <tr></tr>
  <tr></tr>
  <tr>
    <td style="text-align:center;font-size:24px"><b>Speed:</b></td>
    <td colspan=2>
      <div class="slidecontainer">
        <input id="speed"type="range" min="8" max="255" value="125"class="slider" oninput='sendButtonInput("Speed","value")'>
      </div>
    </td>
  </tr>
</table>
<script>
var webSocketCarInputUrl="ws:\/\/"+window.location.hostname+"/CarInput";
var webSocketCarInput;

function sendButtonInput(key,value){
  var data= key+","+value; 
  webSocketCarInput.send(data);
}
function initCarInputWebSocket(){
  webSocketCarInput = new WebSocket(webSocketCarInputUrl);
  webSocketCarInput.onopen = function(event){
    var speedButton = document.getElementById("Speed");
    senButtonInput("speed", speedButton.value);
  };
  webSocketCarInput.onclose = function(event){setTimeout(iniCarInputWebSocket,2000)};
  webSocketCarInput.onmessage = function(event){};
}
window.onload=initCarInputWebSocket;
document.getElementById("mainTable").addEventListender("touchend",function(event){
                                                       event.prentDefault()
                                                       });
</script>
 </body>
</html>
)HTMLHOMEPAGE";
void setUpPinModes()
{
ledcSetup(PWMSpeedChannel,PWMFreq,PWMResolution);
 for (int i=0; i< motorPins.size(); i++);{
  pinMode(motorPins[i].pinEn,OUTPUT);
  pinMode(motorPins[i].pinIN1,OUTPUT);
  pinMode(motorPins[i].pinIN2, OUTPUT);

  ledcAttachPin(motorPins[i].pinEn, PWMSpeedChannel);
 }

}
void handleRoot(AsyncWebServerRequest *request)
{
request->send(200, "text/plain",htmlHomePage);
}
void handleNotFound(AsyncWebServerRequest *request)
{
request->send(404, "text/plain","File Not Found");
}
void rotateMotor(int motorNumber,int motorDirection){
  if (motorDirection == FORWARD){
    digitalWrite(motorPins[motorNumber].pinIN1, HIGH);
    digitalWrite(motorPins[motorNumber].pinIN2, LOW);
  }else if (motorDirection == BACKWARD){
      digitalWrite(motorPins[motorNumber].pinIN1, LOW);
      digitalWrite(motorPins[motorNumber].pinIN2, HIGH);
  }
  else{
      digitalWrite(motorPins[motorNumber].pinIN1, LOW);
      digitalWrite(motorPins[motorNumber].pinIN2, LOW);
  }
}
void moveCar(int valueInt){
  Serial.printf("Got value as %d\n", valueInt);
  switch(valueInt){
    case UP:rotateMotor(RIGHT_MOTOR,FORWARD);rotateMotor(LEFT_MOTOR,FORWARD);
    break;
    case DOWN:rotateMotor(RIGHT_MOTOR,BACKWARD);rotateMotor(LEFT_MOTOR,BACKWARD);
    break;
    case LEFT:rotateMotor(RIGHT_MOTOR,FORWARD);rotateMotor(LEFT_MOTOR,BACKWARD);
    break;
    case RIGHT:rotateMotor(RIGHT_MOTOR,BACKWARD);rotateMotor(LEFT_MOTOR,FORWARD);
    break;
    case STOP:rotateMotor(RIGHT_MOTOR,STOP);rotateMotor(LEFT_MOTOR,STOP);
    break;
    default: ;break;  
    }
}
void onCarInputWebSocketEvent(AsyncWebSocket *server,AsyncWebSocketClient *client, AwsEventType type,void *arg, uint8_t *data, size_t len){
switch(type){
  case WS_EVT_CONNECT:
  Serial.printf("webSocket client#%u connected from %s\n",client->id(),client->remoteIP().toString().c_str()); break;
  case WS_EVT_DISCONNECT: Serial.printf("WebSocket client #%u disconnected\n",client->id());break;
  case WS_EVT_DATA:AwsFrameInfo *info;info=(AwsFrameInfo*)arg;if(info->final && info->index ==0 && info->len ==len && info->opcode==WS_TEXT)
  {
    std::string myData = "";
    myData.assign((char *)data,len);
    std::istringstream ss(myData);
    std::string key,value;
    std::getline(ss,key,',');
    std::getline(ss,value,',');
    Serial.printf("key[%s] value[%s]\n",key,value);
    int valueInt = atoi(value.c_str());
    if (key =="MoveCar"){
      moveCar(valueInt);
    }
    else if (key == "Speed")
    {
      ledcWrite(PWMSpeedChannel,valueInt);
    }
    
  }
  break;
  case WS_EVT_PONG:
  case WS_EVT_ERROR:break;
  deafault:break
}
}
void setup() {
  // put your setup code here, to run once:
  setUpPinModes();
  Serial.begin(115200);

  WiFi.softAP(ssid, password);
  IPAddress IP =WiFi.softAPIP();
  Serial.print("AP IP address:");
  Serial.println(IP);

  server.on("/", HTTP_GET,handleRoot);
  server.onNotFound(handleNotFound);
  wsCarInput.onEvent(onCarInputWebSocketEvent);
  server.addHandler(&wsCarInput);
  server.begin();
  Serial.println("HTTP Server started");

}

void loop() {
 wsCarInput.cleanupClients();
}

// put function definitions here:
