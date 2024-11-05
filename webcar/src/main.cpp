#include <Arduino.h>
#include <WiFi.h>
#include <iostream>
#include <sstream>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

#define UP1
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

void setUpPinModes()
{
ledcSetup(PWMSpeedChannel,PWMFreq,PWMResolution);
 for (int i=0; i< motorPins.size(); i++);{
  pinMode(motorPins[i].pinEn,OUTPUT);
  pinMode(motorPins[i].pinIN1,OUTPUT);
  pinMode(motorPins[i].pinIN2, OUTPUT);

  ldecAttachPin(motorPins[i].pinEn, PWMSpeedChannel);
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
  // put your main code here, to run repeatedly:
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}