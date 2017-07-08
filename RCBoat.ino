#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Servo.h>
#include <WiFiUdp.h>

static const uint8_t GPIO12   = 12;
static const uint8_t GPIO13   = 13;
static const uint8_t GPIO14   = 14;
static const uint8_t GPIO16   = 16;

// Wiring on the ESP8266 board
uint8_t pinEngineP1 = GPIO16;
uint8_t pinEngineP2 = GPIO14;
uint8_t pinThrottle = GPIO12;
uint8_t pinRudder = GPIO13;

// Initial values
static unsigned int valEngineP1 = 0;
static unsigned int valEngineP2 = 0;
static unsigned int valThrottle = 0;
static unsigned int valRudder = 90;

// Hardcoded configuration
const char* ssid     = "RCHub";
const char* password = "Stprtp121";

void startWiFi() {  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

Servo rudder;

WiFiUDP Udp;
unsigned int localUdpPort = 8080;

void setup() {
  // Configure pins on board
  pinMode(pinEngineP1, OUTPUT);
  pinMode(pinEngineP2, OUTPUT);
  pinMode(pinThrottle, OUTPUT);
  rudder.attach(pinRudder);

  // Initialize pins
  digitalWrite(pinEngineP1, valEngineP1 == 0 ? LOW : HIGH);
  digitalWrite(pinEngineP2, valEngineP2 == 0 ? LOW : HIGH);
  digitalWrite(pinThrottle, valThrottle == 0 ? LOW : HIGH);
  rudder.write(valRudder);

  // Try to connect to WiFi
  startWiFi();

  // Start listening on local UDP port for incoming control
  Udp.begin(localUdpPort);
}

unsigned long lastUpdate = 0;
char incomingPacket[10];

void loop() {
  int packetSize = Udp.parsePacket();
  if (packetSize == 10) {
    int len = Udp.read(incomingPacket, 10);
    if (len > 0) { 
      valThrottle = 0x0000FFFF & ((incomingPacket[1] << 8) | incomingPacket[0]);
      valRudder = 0x0000FFFF & ((incomingPacket[3] << 8) | incomingPacket[2]);
      valEngineP1 = 0x0000FFFF & ((incomingPacket[5] << 8) | incomingPacket[4]);
      valEngineP2 = 0x0000FFFF & ((incomingPacket[7] << 8) | incomingPacket[6]);
      unsigned int valChecksum = 0x0000FFFF & ((incomingPacket[9] << 8) | incomingPacket[8]);

      // if ((valThrottle + valRudder + valEngineP1 + valEngineP2) % 171 == valChecksum) {
        
        digitalWrite(pinEngineP1, valEngineP1 == 0 ? LOW : HIGH);
        digitalWrite(pinEngineP2, valEngineP2 == 0 ? LOW : HIGH);
        digitalWrite(pinThrottle, valThrottle == 0 ? LOW : HIGH);
        rudder.write(valRudder);
        
        lastUpdate = millis();
        
      // }
    }
  } else {
     // Out of range protection
    if (millis() - lastUpdate > 1000) {
      // Kill the throttle
      digitalWrite(pinThrottle, LOW);
      // Reset ESP; will cause to try to recconect to the WiFi
      if (WiFi.status() != WL_CONNECTED) {
        ESP.reset();
      }
    }
  }
}
