
/*
  MyWifiManager.cpp - Class MyWifiManager
  Created by Keite Trần, 2018/07/02.
  Released into the public domain.
*/
#ifndef MyWifiManager_h
#define MyWifiManager_h
#include <Arduino.h>
#include <ArduinoJson.h>  // https:// github.com/bblanchon/ArduinoJson
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <FS.h>           // this needs to be first, or it all crashes and burns...
#include <WiFiManager.h>  // https:// github.com/tzapu/WiFiManager

class MyWifiManager {
 public:
  char mqttServer[40];                                     // MQTT server
  char mqttPort[6];                                        // MQTT port
  char mqttClientId[20];                                   // MQTT client id
  char mqttUserName[20];                                   // MQTT username
  char mqttPassword[20];                                   // MQTT password
  uint16_t mqttPortNum;                                    // MQTT port type uint16_t
  bool init(char *a, char *b, char *c, char *d, char *e);  // Init function

 private:
  static bool shouldSaveConfig;      // flag save config file
  static void saveConfigCallback();  // save config file callback
};
#endif