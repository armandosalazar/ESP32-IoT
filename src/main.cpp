#include <Arduino.h>

#include <WiFi.h>

#include <ArduinoJson.h>

#include <WebSocketsClient.h>
#include <SocketIOclient.h>

#include <DHT.h>

#define DHTPIN 4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

SocketIOclient socketIO;
const char *ssid = "CFE2";
const char *password = "12345678";

void socketIOEvent(socketIOmessageType_t type, uint8_t *payload, size_t length);

void setup()
{
  Serial.begin(9600);

  WiFi.begin(ssid, password);
  Serial.print("\n\n[*] Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\n[*] Connected to the WiFi network");

  String ip = WiFi.localIP().toString();
  Serial.printf("[*] IP address: %s\n", ip.c_str());

  // @note socket.io connection
  socketIO.begin("192.168.137.166", 3000, "/socket.io/?EIO=4");
  socketIO.onEvent(socketIOEvent);

  dht.begin();
}

unsigned long messageTimestamp = 0;
void loop()
{
  socketIO.loop();

  uint64_t now = millis();
  if (now - messageTimestamp > 15000)
  {
    messageTimestamp = now;

    float humidity = dht.readHumidity();
    float temperatureCelsius = dht.readTemperature();
    float temperatureFahrenheit = dht.readTemperature(true);
    float heatIndexCelsius = dht.computeHeatIndex(temperatureCelsius, humidity);
    float heatIndexFahrenheit = dht.computeHeatIndex(temperatureFahrenheit, humidity, true);

    JsonDocument doc;
    JsonArray events = doc.to<JsonArray>();
    events.add("sensor");
    JsonObject data = events.add<JsonObject>();
    data["humidity"] = humidity;
    data["temperatureCelsius"] = temperatureCelsius;
    data["temperatureFahrenheit"] = temperatureFahrenheit;
    data["heatIndexCelsius"] = heatIndexCelsius;
    data["heatIndexFahrenheit"] = heatIndexFahrenheit;

    String output;
    serializeJson(doc, output);
    socketIO.sendEVENT(output);
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
  {
    JsonDocument doc;
    deserializeJson(doc, payload);
    String event = doc[0];
    if (event != "sensor")
    {
      Serial.printf("[IOc] event name: %s\n", event.c_str());
      Serial.printf("[IOc] get event: %s\n", payload);
    }
  }
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