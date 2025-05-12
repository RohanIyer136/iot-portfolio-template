#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>  // Use <WiFi.h> for ESP32
#include <PubSubClient.h>

// WiFi Credentials
const char* ssid = "router12";
const char* password = "iot123456789";

// MQTT Broker Settings
const char* mqtt_server = "192.168.14.1";
const char* mqtt_topic = "home/temperature";

// DS18B20 Sensor Setup
#define ONE_WIRE_BUS 4  // GPIO4 (D2 on ESP8266)
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// MQTT Client
WiFiClient espClient;
PubSubClient client(espClient);

// Function to connect to WiFi
void setup_wifi() {
    Serial.print("Connecting to WiFi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi Connected, IP: " + WiFi.localIP().toString());
}

// Function to reconnect to MQTT Broker
void reconnect() {
    while (!client.connected()) {
        Serial.print("Connecting to MQTT...");
        if (client.connect("ESP_Temperature_Sensor")) {
            Serial.println("Connected!");
        } else {
            Serial.print("Failed, retrying in 5s...");
            delay(5000);
        }
    }
}

// Setup function
void setup() {
    Serial.begin(115200);
    setup_wifi();
    client.setServer(mqtt_server, 1883);
    sensors.begin();
}

// Loop function
void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    sensors.requestTemperatures();
    float tempC = sensors.getTempCByIndex(0);

    if (tempC != DEVICE_DISCONNECTED_C) {
        Serial.print("Temperature: ");
        Serial.println(tempC);

        // Convert temperature to a string and publish to MQTT
        char tempStr[10];
        dtostrf(tempC, 5, 2, tempStr);
        client.publish(mqtt_topic, tempStr);
    } else {
        Serial.println("Failed to read from sensor.");
    }

    delay(5000);  // Publish every 5 seconds
}
