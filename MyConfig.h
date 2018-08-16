
/*
  Config.h - Configuration file
  Created by Keite Trần, 2018/07/02.
  Released into the public domain.
  ====================================================
  Config Home Assistant (domain: light or switch is ok)
  switch: #(or domain: light)
  - platform: mqtt
    name: 'node1'
    state_topic: 'ChipId/switchName'
    command_topic: 'ChipId/switchName/set'
    payload_on: "1"
    payload_off: "0"
    qos: 0
    retain: true
*/

// Debug setting
#define DEBUG_LOG true

// Debug logger
#ifdef DEBUG_LOG
#define Loggerln(x) Serial.println(x)
#define Logger(x) Serial.print(x)
#else
#define Loggerln(x)
#define Logger(x)
#endif

// Wifi info
#define WIFI_SSID "832947893274988"
#define WIFI_PASSWORD "xxx"

// MQTT info
#define MQTT_SERVER "220.144.20.xxx"
#define MQTT_PORT 1883
#define MQTT_USERNAME "homeassistant"
#define MQTT_PASSWORD "xxx"

// Global VAR
#define _ON "1"
#define _OFF "0"

// Switch 1
#define SW_1_NAME "Switch1"  // Name (Not have space)
#define SW_1_RELAY_PIN D1    // Relay pin
#define SW_1_BUTTON_PIN D5   // Touth module pin
#define SW_1_LED_PIN D0      // Led pin

// Switch 2
#define SW_2_NAME "Switch2"  // Name (Not have space)
#define SW_2_RELAY_PIN D2    // Relay pin
#define SW_2_BUTTON_PIN D6   // Touth module pin
#define SW_2_LED_PIN D4      // Led pin

// Switch 3
#define SW_3_NAME "Switch3"  // Name (Not have space)
#define SW_3_RELAY_PIN D3    // Relay pin // High run | low flash
#define SW_3_BUTTON_PIN D7   // Touth module pin
#define SW_3_LED_PIN D8      // Led pin