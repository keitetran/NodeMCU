
/*
  Config.h - Configuration file
  Created by Keite Trần, 2018/07/02.
  Released into the public domain.
  ====================================================
  Config Home Assistant (domain: light or switch is ok)
  switch: #(or domain: light)
  - platform: mqtt
    name: 'node1'
    state_topic: 'MQTT_NAME/switch1'
    command_topic: 'MQTT_NAME/switch1/set'
    payload_on: "1"
    payload_off: "0"
    qos: 0
    retain: true
*/

// Debug setting
#define DEBUG_LOG true

// Debug logger
#ifdef DEBUG_LOG
#define Logger(x) Serial.println(x)
#define Logger2(x) Serial.print(x)
#else
#define Logger(x)
#define Logger2(x)
#endif

// Wifi info
#define WIFI_SSID "832947893274988" 
#define WIFI_PASSWORD "xxxx"

// MQTT info
#define MQTT_SERVER "xxxx.144.20.xxxx" 
#define MQTT_SERVER_PORT 1883
#define MQTT_SERVER_USERNAME "homeassistant"
#define MQTT_SERVER_PASSWORD "xxxx"

// Global VAR
#define _ON "1"
#define _OFF "0"
