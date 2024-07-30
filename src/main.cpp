#include <DHTesp.h>
#include <WiFi.h>
#include <ThingsBoard.h>
#include <Arduino_MQTT_Client.h>

#define pinDht 15
DHTesp dhtSensor;

#include "secrets.h"

const char *WIFI_SSID = secret_wifi_ssid;
const char *WIFI_PASSWORD = secret_wifi_password;

// See https://thingsboard.io/docs/getting-started-guides/helloworld/
// to understand how to obtain an access token
const char *TOKEN = secret_thingsboard_token;

// Thingsboard we want to establish a connection too
const char *THINGSBOARD_SERVER = secret_thingsboard_server;

constexpr uint16_t MAX_MESSAGE_SIZE = 128U;

WiFiClient espClient;
Arduino_MQTT_Client mqttClient(espClient);
ThingsBoard tb(mqttClient, MAX_MESSAGE_SIZE);

void connectToWiFi() {
  Serial.println("Connecting to WiFi...");
  int attempts = 0;
  
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD, 6);
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nFailed to connect to WiFi.");
  } else {
    Serial.println("\nConnected to WiFi");
  }
}

void connectToThingsBoard() {
  if (!tb.connected()) {
    Serial.println("Connecting to ThingsBoard server");
    
    if (!tb.connect(THINGSBOARD_SERVER, TOKEN)) {
      Serial.println("Failed to connect to ThingsBoard");
    } else {
      Serial.println("Connected to ThingsBoard");
    }
  }
}

void sendDataToThingsBoard(float temp, int hum) {
  String jsonData = "{\"temperature\":" + String(temp) + ", \"humidity\":" + String(hum) + "}";
  tb.sendTelemetryJson(jsonData.c_str());
  Serial.println("Data sent");
}

void setup() {
  Serial.begin(9600);
  dhtSensor.setup(pinDht,DHTesp::DHT11);
  connectToWiFi();
  connectToThingsBoard();
}

void loop() {
  connectToWiFi();

  TempAndHumidity data =dhtSensor.getTempAndHumidity();
  float temp = data.temperature;
  int hum = data.humidity;

  Serial.println(temp);
  Serial.println(hum);

  if (!tb.connected()) {
    connectToThingsBoard();
  }

  sendDataToThingsBoard(temp, hum);

  delay(3000);

  tb.loop();
}