#include <ESP8266WiFi.h>
#include <WebSocketClient.h>
#include "config.h"

/*
all settings made in config.h 
const char* SSID = "";
const char* PSK = "";
char* host = "";
*/

//Radumfang: 2,19 m = 2190 mm

boolean handshakeFailed=0;
String data= "";

char path[] = "/";
const int espport= 3000;

WebSocketClient webSocketClient;

unsigned long interval=300; //interval for sending data to the websocket server in ms

// Use WiFiClient class to create TCP connections
WiFiClient client;

const byte cadencePin = D1; 
const uint16_t bounceTime = 5; // in ms

const byte speedPin = D2; 

volatile uint32_t cadence_t0 = 0;
volatile uint32_t cadence_t1 = 0;
volatile uint32_t cadenceCounter = 0;

volatile uint32_t speed_t0 = 0;
volatile uint32_t speed_t1 = 0;
volatile uint32_t speedCounter = 0;

ICACHE_RAM_ATTR void cadenceISR() {
  if (millis() - cadence_t1 >= bounceTime) {
    cadence_t0 = cadence_t1;
    cadence_t1 = millis(); 
    cadenceCounter++;  
  } 
}

ICACHE_RAM_ATTR void speedISR() {
  if (millis() - speed_t1 >= bounceTime) {
    speed_t0 = speed_t1;
    speed_t1 = millis(); 
    speedCounter++;  
  } 
}

void setup() {
  Serial.begin(9600);
  Serial.println("Start");
  pinMode(cadencePin,INPUT_PULLUP);

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
  
  attachInterrupt(digitalPinToInterrupt(cadencePin),cadenceISR,RISING);
  attachInterrupt(digitalPinToInterrupt(speedPin),speedISR,RISING);

  wsconnect();

}

void getCadence() {
  static uint16_t cadenceCounterOld = 0;
  uint32_t cadenceCounterLocal;
  uint32_t t1Local;
  uint32_t t0Local;

  cli();
  cadenceCounterLocal = cadenceCounter;
  t1Local = t1;
  t0Local = t0;
  sei();
  if (cadenceCounterLocal != cadenceCounterOld) {
    cadenceCounterOld = cadenceCounterLocal;
    uint32_t cadenceInterval = t1Local - t0Local;
    uint32_t cadence = 60000/cadenceInterval;
    Serial.print("Counter: ");Serial.print(cadenceCounterLocal);
    Serial.print(" Millis: ");Serial.println(t1Local);
    Serial.print(" took: ");Serial.println(cadenceInterval);
    Serial.print(" Cadence: ");Serial.println(cadence);
    webSocketClient.sendData((String) cadence);
  }
}

void getValues() {
  static uint16_t cadenceCounterOld = 0;
  uint32_t cadenceCounterLocal;
  uint32_t cadence_t1Local;
  uint32_t cadence_t0Local;

  static uint16_t speedCounterOld = 0;
  uint32_t speedCounterLocal;
  uint32_t speed_t1Local;
  uint32_t speed_t0Local;
  

  cli();
  
  cadenceCounterLocal = cadenceCounter;
  cadence_t1Local = cadence_t1;
  cadence_t0Local = cadence_t0;

  speedCounterLocal = speedCounter;
  speed_t1Local = speed_t1;
  speed_t0Local = speed_t0;
  
  sei();
  
  if (cadenceCounterLocal != cadenceCounterOld) {
    cadenceCounterOld = cadenceCounterLocal;
    uint32_t cadenceInterval = t1Local - t0Local;
    uint32_t cadence = 60000/cadenceInterval;
    Serial.print("Counter: ");Serial.print(cadenceCounterLocal);
    Serial.print(" Millis: ");Serial.println(t1Local);
    Serial.print(" took: ");Serial.println(cadenceInterval);
    Serial.print(" Cadence: ");Serial.println(cadence);
    webSocketClient.sendData((String) cadence);
  }

  if (speedCounterLocal != speedCounterOld) {
    speedCounterOld = speedCounterLocal;
    
  }
  
}


void loop() {
  getCadence();

}

void wsconnect(){
  // Connect to the websocket server
  if (client.connect(host, espport)) {
    Serial.println("Connected");
  } else {
    Serial.println("Connection failed.");
      delay(1000);  
   
   if(handshakeFailed){
    handshakeFailed=0;
    ESP.restart();
    }
    handshakeFailed=1;
  }
  // Handshake with the server
  webSocketClient.path = path;
  webSocketClient.host = host;
  if (webSocketClient.handshake(client)) {
    Serial.println("Handshake successful");
  } else {
    
    Serial.println("Handshake failed.");
   delay(4000);  
   
   if(handshakeFailed){
    handshakeFailed=0;
    ESP.restart();
    }
    handshakeFailed=1;
  }
}
