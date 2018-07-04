/*
  MyWifiManager.cpp - Class MyWifiManager
  Created by Keite Trần, 2018/07/02.
  Released into the public domain.
*/

#include "MyWifiManager.h"  // https://github.com/tzapu/WiFiManager
#include <Arduino.h>
#include <ArduinoJson.h>  // https://github.com/bblanchon/ArduinoJson
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <FS.h>           // this needs to be first, or it all crashes and burns...
#include <WiFiManager.h>  // https://github.com/tzapu/WiFiManager

// Set flag về false
// ------------------------------------------------
bool MyWifiManager::shouldSaveConfig = false;

// Save config file callback
// ------------------------------------------------
void MyWifiManager::saveConfigCallback() {
  shouldSaveConfig = true;
  Serial.println("Should save config");
};

/**
 * Setup wifimanager 
 * @param: char* a - Client id
 * @param: char* b - MQTT server ip
 * @param: char* c - MQTT server port 
 * @param: char* d - MQTT username
 * @param: char* e - MQTT password
 * @return: bool: wifi conection state
*/
bool MyWifiManager::init(char *a, char *b, char *c, char *d, char *e) {
  // Set flag về false
  shouldSaveConfig = false;

  // Chuyển đổi kiểu dữ liệu
  String(a).toCharArray(mqttClientId, 20);
  String(b).toCharArray(mqttServer, 40);
  String(c).toCharArray(mqttPort, 6);
  String(d).toCharArray(mqttUserName, 20);
  String(e).toCharArray(mqttPassword, 20);

  // Read configuration from FS json
  Serial.println("Mounting FS...");

  // Xóa hết phân vùng filesystem
  // SPIFFS.format();

  // Bắt đầu đọc duex liệu từ file config
  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject &json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");

          strcpy(mqttServer, json["mqttServer"]);
          strcpy(mqttPort, json["mqttPort"]);
          strcpy(mqttClientId, json["mqttClientId"]);
          strcpy(mqttUserName, json["mqttUserName"]);
          strcpy(mqttPassword, json["mqttPassword"]);
        } else {
          Serial.println("failed to load json config");
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  //end read

  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter customMqttServer("server", "mqtt server", mqttServer, 40);
  WiFiManagerParameter customMqttPort("port", "mqtt port", mqttPort, 6);
  WiFiManagerParameter customMqttClientId("clientId", "mqtt client id", mqttClientId, 20);
  WiFiManagerParameter customMqttUserName("username", "mqtt username", mqttUserName, 20);
  WiFiManagerParameter customMqttPassword("password", "mqtt password", mqttPassword, 20);

  // WiFiManager
  // Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  // set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  // set static ip
  // wifiManager.setSTAStaticIPConfig(IPAddress(10, 0, 1, 99), IPAddress(10, 0, 1, 1), IPAddress(255, 255, 255, 0));

  // add all your parameters here
  wifiManager.addParameter(&customMqttServer);
  wifiManager.addParameter(&customMqttPort);
  wifiManager.addParameter(&customMqttClientId);
  wifiManager.addParameter(&customMqttUserName);
  wifiManager.addParameter(&customMqttPassword);

  // reset settings - for testing
  // wifiManager.resetSettings();

  // set minimu quality of signal so it ignores AP's under that quality
  // defaults to 8%
  // wifiManager.setMinimumSignalQuality();

  // sets timeout until configuration portal gets turned off
  // useful to make it all retry or go to sleep
  // in seconds
  // wifiManager.setTimeout(120);

  // fetches ssid and pass and tries to connect
  // if it does not connect it starts an access point with the specified name
  // here  "AutoConnectAP"
  // and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("AutoConnectAP", "password")) {
    Serial.println("Failed to connect and hit timeout");
    delay(3000);

    // reset and try again, or maybe put it to deep sleep
    // ESP.reset();
    // delay(5000);
  }

  // if you get here you have connected to the WiFi
  Serial.println("Connected...yeey :)");

  // read updated parameters
  strcpy(mqttServer, customMqttServer.getValue());
  strcpy(mqttPort, customMqttPort.getValue());
  strcpy(mqttClientId, customMqttClientId.getValue());
  strcpy(mqttUserName, customMqttUserName.getValue());
  strcpy(mqttPassword, customMqttPassword.getValue());

  /// Chuyển dữ liệu từ String sang uint16_t cho phù hợp với kiểu dữ liệu của MQTT
  mqttPortNum = strtol(String(mqttPort).c_str(), NULL, 0);

  // save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("Saving config..");
    DynamicJsonBuffer jsonBuffer;
    JsonObject &json = jsonBuffer.createObject();
    json["mqttServer"] = mqttServer;
    json["mqttPort"] = mqttPort;
    json["mqttClientId"] = mqttClientId;
    json["mqttUserName"] = mqttUserName;
    json["mqttPassword"] = mqttPassword;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("Failed to open config file for writing");
      return false;
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    // end save
  }

  Serial.println("Local ip: ");
  Serial.println(WiFi.localIP());
  return true;
};
