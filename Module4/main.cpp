#include <Wire.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <WiFiServer.h>
#include <U8g2lib.h>
#include <PubSubClient.h>

// ✅ WiFi Credentials
const char *ssid = "Mango_WiFi";
const char *password = "iotempire";

const char *mqtt_server = "192.168.14.1"; // Remplacez par l'adresse IP de votre broker MQTT
const int mqtt_port = 1883;

WiFiClient espClient;
WiFiServer telnetServer(23); // Serveur Telnet sur le port 23
WiFiClient telnetClient;

PubSubClient client(espClient);

#define RST_PIN 26
#define SS_PIN 5

MFRC522 mfrc522(SS_PIN, RST_PIN);

// ✅ Fonction pour imprimer les logs via Serial et Telnet
void print_log(const char *message)
{
  // Logs via Serial
  Serial.println(message);

  // Logs via Telnet
  if (telnetClient && telnetClient.connected())
  {
    telnetClient.println(message);
  }
}

// ✅ MQTT Credentials
void connectToMQTT()
{
  while (!client.connected())
  {
    print_log("Connecting to MQTT...");
    if (client.connect("ESP32-MiniKit"))
    {
      print_log("Connected to MQTT!");
    }
    else
    {
      print_log("Failed to connect to MQTT. Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

// ✅ WiFi Connection Function
void connectToWiFi()
{
  WiFi.mode(WIFI_STA);
  WiFi.setHostname("ESP32-MiniKit");
  WiFi.begin(ssid, password);

  print_log("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    print_log(".");
  }
  print_log("\nWiFi Connected!");
  print_log("IP Address: ");
  print_log(WiFi.localIP().toString().c_str());
}

// ✅ OTA Setup Function
void setupOTA()
{
  ArduinoOTA.setHostname("ESP32-MiniKit");
  ArduinoOTA.setPort(3232);

  ArduinoOTA.onStart([]()
                     { print_log("OTA Update Starting..."); });

  ArduinoOTA.onEnd([]()
                   { print_log("\nOTA Update Complete!"); });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                        { char buffer[50];
                          snprintf(buffer, sizeof(buffer), "OTA Progress: %u%%", (progress / (total / 100)));
                          print_log(buffer); });

  ArduinoOTA.onError([](ota_error_t error)
                     {
        char buffer[50];
        snprintf(buffer, sizeof(buffer), "OTA Error [%u]: ", error);
        print_log(buffer);
        if (error == OTA_AUTH_ERROR) print_log("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) print_log("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) print_log("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) print_log("Receive Failed");
        else if (error == OTA_END_ERROR) print_log("End Failed"); });

  ArduinoOTA.begin();
  print_log("OTA Ready!");
}

// ✅ Fonction pour lire les cartes RFID
void readRFID()
{
  // Vérifie si une nouvelle carte est présente
  if (!mfrc522.PICC_IsNewCardPresent())
    return;

  // Tente de lire le contenu de la carte
  if (!mfrc522.PICC_ReadCardSerial())
    return;

  // Affiche l'UID de la carte
  String uid = "UID tag :";
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    if (mfrc522.uid.uidByte[i] < 0x10)
      uid += " 0";
    else
      uid += " ";
    uid += String(mfrc522.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();

  // Log l'UID via Serial et Telnet
  print_log(uid.c_str());

  // Publier l'UID au topic MQTT
  if (client.connected())
  {
    client.publish(">>", uid.c_str());
    print_log("UID published to MQTT topic: rfid/read");
  }
  else
  {
    print_log("MQTT not connected. Unable to publish UID.");
  }

  // Arrête la communication avec la carte
  mfrc522.PICC_HaltA();
}

// ✅ Setup Function
void setup()
{
  Serial.begin(115200);
  delay(5000);

  SPI.begin();
  mfrc522.PCD_Init();

  connectToWiFi();
  setupOTA();

  client.setServer(mqtt_server, mqtt_port);
  connectToMQTT();

  // Démarrage du serveur Telnet
  telnetServer.begin();
  telnetServer.setNoDelay(true);
  print_log("Telnet Server Started!");

  byte version = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  Serial.print("RC522 Version: 0x");
  Serial.println(version, HEX);
  if (version == 0x00 || version == 0xFF)
  {
    Serial.println("❌ RC522 non détecté !");
  }
  else
  {
    Serial.println("✅ RC522 détecté !");
  }
}

// ✅ Main Loop
void loop()
{
  ArduinoOTA.handle(); // Keep OTA service running

  // Gestion des connexions Telnet
  if (telnetServer.hasClient())
  {
    if (!telnetClient || !telnetClient.connected())
    {
      if (telnetClient)
        telnetClient.stop();
      telnetClient = telnetServer.available();
      print_log("Telnet Client Connected!");
    }
    else
    {
      telnetServer.available().stop(); // Refuse une nouvelle connexion si un client est déjà connecté
    }
  }
  // Maintenir la connexion MQTT
  if (!client.connected())
  {
    connectToMQTT();
  }
  client.loop();
  // Lire les cartes RFID
  readRFID();
}
