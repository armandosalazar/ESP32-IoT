#include <Arduino.h>

#include <WiFi.h>

#include <ArduinoJson.h>

#include <WebSocketsClient.h>
#include <SocketIOclient.h>

SocketIOclient socketIO;

void socketIOEvent(socketIOmessageType_t type, uint8_t *payload, size_t length);

void setup()
{
  Serial.begin(9600);

  WiFi.begin("salazar", "232005195");
  Serial.print("\n\n\n[*] Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\n[*] Connected to the WiFi network");

  String ip = WiFi.localIP().toString();
  Serial.printf("[*] IP address: %s\n", ip.c_str());

  // @note socket.io connection
  socketIO.begin("192.168.0.25", 3000, "/socket.io/?EIO=4");
  socketIO.onEvent(socketIOEvent);
}

unsigned long messageTimestamp = 0;
void loop()
{
  socketIO.loop();

  uint64_t now = millis();
  if (now - messageTimestamp > 2000)
  {
    messageTimestamp = now;
    JsonDocument doc;
    JsonArray events = doc.to<JsonArray>();

    events.add("sensor");

    JsonObject data = events.add<JsonObject>();
    data["humidity"] = 50;
    data["temperature"] = 25;

    String output;
    serializeJson(doc, output);
    socketIO.sendEVENT(output);
    Serial.println(output);
  }
}

void socketIOEvent(socketIOmessageType_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  case sIOtype_DISCONNECT:
    Serial.printf("[IOc] Disconnected!\n");
    break;
  case sIOtype_CONNECT:
  {
    Serial.printf("[IOc] Connected to url: %s\n", payload);
    socketIO.send(sIOtype_CONNECT, "/");
  }
  break;
  case sIOtype_EVENT:
    Serial.printf("[IOc] get event: %s\n", payload);
    break;
  case sIOtype_ACK:
    Serial.printf("[IOc] get ack: %u\n", length);
    socketIO.send(sIOtype_ACK, "message");
    break;
  case sIOtype_ERROR:
    Serial.printf("[IOc] get error: %u\n", length);
    break;
  case sIOtype_BINARY_EVENT:
    Serial.printf("[IOc] get binary: %u\n", length);
    break;
  case sIOtype_BINARY_ACK:
    Serial.printf("[IOc] get binary ack: %u\n", length);
    break;
  }
}