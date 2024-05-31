#include <ESP8266WiFi.h>
#include <WebSocketClient.h>
#include <ArduinoJson.h>
#include "config.h"

/*
all settings made in config.h 
const char* SSID = "";
const char* PSK = "";
char* host = "";
*/

//Radumfang: 2,19 m = 2190 mm

boolean handshakeFailed = 0;
String data = "";
StaticJsonDocument<200> doc;
String jsonString;

char path[] = "/";
const int espport = 8088;

WebSocketClient webSocketClient;

unsigned long interval = 300;  //interval for sending data to the websocket server in ms

//a comment

// Use WiFiClient class to create TCP connections
WiFiClient client;

const byte cadencePin = D1;
const uint16_t bounceTime = 150;  // in ms

const byte speedPin = D3;

volatile uint32_t cadence_t0 = 0;
volatile uint32_t cadence_t1 = 0;
volatile uint32_t cadenceCounter = 0;
int rpm = 0;
bool cadenceChanged = false;


volatile uint32_t speed_t0 = 0;
volatile uint32_t speed_t1 = 0;
volatile uint32_t speedCounter = 0;
int rpm_wheel = 0;
bool speedChanged = false;
float speed = 0;

ICACHE_RAM_ATTR void cadenceISR() {
  if (millis() - cadence_t1 >= bounceTime) {
    cadence_t0 = cadence_t1;
    cadence_t1 = millis();
    cadenceCounter++;
    cadenceChanged = true;
  }
}

ICACHE_RAM_ATTR void speedISR() {
  //Serial.println("SpeedISR");
  if (millis() - speed_t1 >= bounceTime) {
    speed_t0 = speed_t1;
    speed_t1 = millis();
    speedCounter++;
    speedChanged = true;
    //Serial.println("Speed changed");
  }
  //Serial.println("Speed hasn't changed");
}


void setup() {
  Serial.begin(9600);
  Serial.println("Start");
  pinMode(cadencePin, INPUT_PULLUP);
  pinMode(speedPin, INPUT_PULLUP);

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(SSID);

  WiFi.begin(SSID, PSK);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  delay(1000);
  doc["rpm"] = 0;
  doc["speed"] = 0;

  attachInterrupt(digitalPinToInterrupt(cadencePin), cadenceISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(speedPin), speedISR, CHANGE);

  wsconnect();
  
}

void getSpeed() {
  //Umfang: 218cm
  cli();
  rpm_wheel = 60000 / (speed_t1 - speed_t0);
  Serial.print("RPM_wheel: ");
  Serial.println(rpm_wheel);
  speed = rpm_wheel * 2.18;  //rounds per minute * umfang = strecke/minute
  speed = speed / 16.66667;
  Serial.print("Speed: ");
  Serial.println(speed);
  sei();
}

void getCadence() {
  cli();
  rpm = 60000 / (cadence_t1 - cadence_t0);
  Serial.print("RPM: ");
  Serial.println(rpm);
  //doc["rpm"] = rpm;
  sei();
}



void loop() {
  if (cadenceChanged) {
    getCadence();
    doc["rpm"] = rpm;
    serializeJson(doc, jsonString);
    webSocketClient.sendData(jsonString);
    cadenceChanged = false;
    }
  if (speedChanged) {
    getSpeed();
    doc["speed"] = speed;    
    serializeJson(doc, jsonString);
    webSocketClient.sendData(jsonString);
    speedChanged = false;
  }
}

void wsconnect() {
  // Connect to the websocket server
  if (client.connect(host, espport)) {
    Serial.println("Connected");
  } else {
    Serial.println("Connection failed.");
    delay(1000);

    if (handshakeFailed) {
      handshakeFailed = 0;
      ESP.restart();
    }
    handshakeFailed = 1;
  }
  // Handshake with the server
  webSocketClient.path = path;
  webSocketClient.host = host;
  if (webSocketClient.handshake(client)) {
    Serial.println("Handshake successful");
  } else {

    Serial.println("Handshake failed.");
    delay(4000);

    if (handshakeFailed) {
      handshakeFailed = 0;
      ESP.restart();
    }
    handshakeFailed = 1;
  }
}
