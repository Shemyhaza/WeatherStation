#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"
#include "secrets.h"

#define DHTPIN D2
#define DHTTYPE DHT11
#define MQTT_SERVER "192.168.1.106" 

int sensor_A0 = A0;
int sensor_D1 = D1;
int value_A0;
int value_D1;

DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  pinMode(D1, INPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());

  client.setServer(MQTT_SERVER, 1883); 

  dht.begin();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  value_A0 = analogRead(sensor_A0);
  value_D1 = digitalRead(sensor_D1);
  
  delay(1000);

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  float hic = dht.computeHeatIndex(t, h, false);

  String payload = String(F("Humidity: ")) + h + F("%  Temperature: ") + t + F("°C Heat index: ") + hic + F("°C D0: ") + value_D1 + F(" -- A0: ") + value_A0;
  Serial.println(payload);

  client.publish("esp8266/measurements", payload.c_str());
}

void reconnect() {
  // Reconnection loop
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Try to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 sec for next attemp
      delay(5000);
    }
  }
}
