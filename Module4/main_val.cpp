#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <Arduino.h>
#include <PubSubClient.h> // MQTT Library
#include <WiFiServer.h>   // For Telnet Logging

#ifndef STASSID
#define STASSID "Mango_WiFi"
#define STAPSK "iotempire"
#endif

const char *ssid = STASSID;
const char *password = STAPSK;
const char *hostname = "Esp_LED";
const char *mqtt_server = "192.168.14.1"; // Modify as needed

const int relayPin = 5; // Broche de contrôle du relais

WiFiClient espClient;
PubSubClient client(espClient);

bool flashing = false;
unsigned long startTime = 0;

// Telnet Server
WiFiServer telnetServer(23);
WiFiClient telnetClient;

// Function to send logs to Serial and Telnet
void sendLog(String message)
{
  Serial.println(message); // Print to Serial Monitor (if connected)
  if (telnetClient && telnetClient.connected())
  {
    telnetClient.println(message); // Send to Telnet Client
  }
}

// MQTT Callback function
void callback(char *topic, byte *payload, unsigned int length)
{
  String message;
  for (unsigned int i = 0; i < length; i++)
  {
    message += (char)payload[i];
  }

  sendLog("Message received [" + String(topic) + "] : " + message);

  if (String(topic) == "rfid/read")
  {
    if (message == "UID TAG : B3 68 4D 08")
    {
      sendLog("Correct tag detected! Unlocking...");
      digitalWrite(relayPin, HIGH); // Active le relais (ouvre le circuit en mode NC)
      delay(5000);                  // Maintient le relais activé pendant 5 secondes
      digitalWrite(relayPin, LOW);  // Désactive le relais (ferme le circuit en mode NC)
      sendLog("Lock re-engaged.");
    }
    else
    {
      sendLog("Incorrect tag. Lock remains engaged.");
    }
  }
}

// Function to reconnect to MQTT
void reconnectMQTT()
{
  while (!client.connected())
  {
    sendLog("Connecting to MQTT server...");
    if (client.connect(hostname))
    {
      sendLog("Connected to MQTT!");
      client.subscribe("rfid/read"); // Subscribe to "rfid/read" topic
    }
    else
    {
      sendLog("Failed to connect, error: " + String(client.state()) + " Retrying in 5s...");
      delay(5000);
    }
  }
}

// Function to handle WiFi connection
void connectWiFi()
{
  sendLog("Connecting to WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.setHostname(hostname);

  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    sendLog("WiFi connection failed! Restarting...");
    delay(5000);
    ESP.restart();
  }

  sendLog("WiFi connected.");
  sendLog("IP Address: " + WiFi.localIP().toString());
}

// Function to configure OTA updates
void configureOTA()
{
  ArduinoOTA.setHostname(hostname);
  ArduinoOTA.onStart([]()
                     { sendLog("OTA update in progress..."); });
  ArduinoOTA.onEnd([]()
                   { sendLog("OTA update completed!"); });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                        { sendLog("OTA Progress: " + String(progress / (total / 100)) + "%"); });
  ArduinoOTA.onError([](ota_error_t error)
                     {
        String errMsg = "OTA Error [" + String(error) + "] : ";
        if (error == OTA_AUTH_ERROR) errMsg += "Authentication failed";
        else if (error == OTA_BEGIN_ERROR) errMsg += "Start failed";
        else if (error == OTA_CONNECT_ERROR) errMsg += "Connection failed";
        else if (error == OTA_RECEIVE_ERROR) errMsg += "Reception failed";
        else if (error == OTA_END_ERROR) errMsg += "Finalization failed";
        sendLog(errMsg); });
  ArduinoOTA.begin();
}

// Function to handle Telnet client connections
void checkTelnetClient()
{
  if (telnetServer.hasClient())
  {
    if (telnetClient)
    {
      telnetClient.stop();
      sendLog("Previous Telnet client disconnected.");
    }
    telnetClient = telnetServer.available();
    sendLog("New Telnet client connected.");
  }

  if (telnetClient && telnetClient.connected())
  {
    while (telnetClient.available())
    {
      Serial.write(telnetClient.read());
    }
  }
}

void setup()
{
  Serial.begin(115200);
  sendLog("Booting...");

  // Configure relay pin as output
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW); // Désactive le relais au démarrage (ferme le circuit en mode NC)

  // Connect to WiFi
  connectWiFi();

  // Start Telnet Server
  telnetServer.begin();
  telnetServer.setNoDelay(true);
  sendLog("Telnet server started on port 23.");

  // Initialize MQTT
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  // Configure OTA
  configureOTA();
}

void loop()
{
  ArduinoOTA.handle();

  if (!client.connected())
  {
    reconnectMQTT();
  }
  client.loop();

  checkTelnetClient(); // Check for new Telnet connections
}
