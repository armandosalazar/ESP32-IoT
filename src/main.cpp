#include <Arduino.h>

#include <WiFi.h>

#include <WebSocketsClient.h>
#include <SocketIOclient.h>

void socketIoEvent();

void setup() {
  Serial.begin(9600);

  // @note wifi connection
  WiFi.begin("salazar", "232005195");

  Serial.print("\nConnecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nConnected to the WiFi network");

  String ip = WiFi.localIP().toString();
  Serial.printf("IP address: %s\n", ip.c_str());
}

void loop() {
}

void socketIoEvent() {
}