
/*
  MyWifiManager.cpp - Class MyWifiManager
  Created by Keite Trần, 2018/07/02.
  Released into the public domain.
*/
#include <Arduino.h>
#include <ArduinoJson.h>  // https://github.com/bblanchon/ArduinoJson
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <FS.h>           // this needs to be first, or it all crashes and burns...
#include <WiFiManager.h>  // https://github.com/tzapu/WiFiManager
class MyWifiManager {
 public:
  MyWifiManager();
  char mqttServer[40];                                     // MQTT server
  char mqttPort[6];                                        // MQTT port
  char mqttClientId[20];                                   // MQTT client id
  char mqttUserName[20];                                   // MQTT username
  char mqttPassword[20];                                   // MQTT password
  char OTAPassword[20];                                    // OTA update password
  uint16_t mqttPortNum;                                    // MQTT port type uint16_t
  bool init(char *a, char *b, char *c, char *d, char *e);  // Init function

  static bool shouldSaveConfig;      // flag save config file
  static void saveConfigCallback();  // save config file callback
};

MyWifiManager::MyWifiManager(){};

// Set flag về false
// ------------------------------------------------
bool MyWifiManager::shouldSaveConfig = false;

// Save config file callback
// ------------------------------------------------
void MyWifiManager::saveConfigCallback() {
  shouldSaveConfig = true;
  Logger("Should save config");
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
  String(e).toCharArray(OTAPassword, 20);

  // Read configuration from FS json
  Logger("Mounting FS...");

  // Xóa hết phân vùng filesystem
  // SPIFFS.format();

  // Bắt đầu đọc duex liệu từ file config
  if (SPIFFS.begin()) {
    Logger("Mounted file system...");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Logger("Reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Logger("Opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject &json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Logger("\nParsed json");

          strcpy(mqttServer, json["mqttServer"]);
          strcpy(mqttPort, json["mqttPort"]);
          strcpy(mqttClientId, json["mqttClientId"]);
          strcpy(mqttUserName, json["mqttUserName"]);
          strcpy(mqttPassword, json["mqttPassword"]);
          strcpy(OTAPassword, json["OTAPassword"]);
        } else {
          Logger("Failed to load json config");
        }
      }
    }
  } else {
    Logger("Failed to mount FS");
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
  WiFiManagerParameter customOTAPassword("OTAPassword", "OTA update password", OTAPassword, 20);

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
  wifiManager.addParameter(&customOTAPassword);

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
  if (!wifiManager.autoConnect()) {
    Logger("Failed to connect and hit timeout");
    delay(3000);

    // reset and try again, or maybe put it to deep sleep
    // ESP.reset();
    // delay(5000);
  }

  // if you get here you have connected to the WiFi
  Logger("Connected...yeey :)");

  // read updated parameters
  strcpy(mqttServer, customMqttServer.getValue());
  strcpy(mqttPort, customMqttPort.getValue());
  strcpy(mqttClientId, customMqttClientId.getValue());
  strcpy(mqttUserName, customMqttUserName.getValue());
  strcpy(mqttPassword, customMqttPassword.getValue());
  strcpy(OTAPassword, customOTAPassword.getValue());

  /// Chuyển dữ liệu từ String sang uint16_t cho phù hợp với kiểu dữ liệu của MQTT
  mqttPortNum = strtol(String(mqttPort).c_str(), NULL, 0);

  // save the custom parameters to FS
  if (shouldSaveConfig) {
    Logger("Saving config..");
    DynamicJsonBuffer jsonBuffer;
    JsonObject &json = jsonBuffer.createObject();
    json["mqttServer"] = mqttServer;
    json["mqttPort"] = mqttPort;
    json["mqttClientId"] = mqttClientId;
    json["mqttUserName"] = mqttUserName;
    json["mqttPassword"] = mqttPassword;
    json["OTAPassword"] = OTAPassword;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Logger("Failed to open config file for writing");
      return false;
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
  }  // end save
  return true;
};
