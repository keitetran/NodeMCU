/*
  EspBathRoomSwitch.ino - Toilet switch
  Created by Keite Trần, 2018/07/02.
  Released into the public domain.
*/

// Config file
#include "./Extra/Config.h";

// Library
#include <Arduino.h>            // Arduino
#include <ESP8266WiFi.h>        // https://www.arduino.cc/en/Reference/WiFiClient
#include <PubSubClient.h>       // https://github.com/knolleary/pubsubclient
#include "./Extra/MySwitch.h"   // Custom class MySwitch
#include "./Extra/UpdateOTA.h"  // Custom class update OTA

// MQTT name
#define MQTT_NAME "EspBathRoom"

// Switch 1
#define SW_1_NAME "Switch 1"  // Name
#define SW_1_RELAY_PIN D1     // Relay pin
#define SW_1_BUTTON_PIN D5    // Touth module pin
#define SW_1_STATE_TOPIC "EspBathRoom/switch1"
#define SW_1_COMMAND_TOPIC "EspBathRoom/switch1/set"

// Switch 2
#define SW_2_NAME "Switch 2"  // Name
#define SW_2_RELAY_PIN D2     // Relay pin
#define SW_2_BUTTON_PIN D6    // Touth module pin
#define SW_2_STATE_TOPIC "EspBathRoom/switch2"
#define SW_2_COMMAND_TOPIC "EspBathRoom/switch2/set"

// Switch 3
#define SW_3_NAME "Switch 3"  // Name
#define SW_3_RELAY_PIN D3     // Relay pin // High run | low flash
#define SW_3_BUTTON_PIN D7    // Touth module pin
#define SW_3_STATE_TOPIC "EspBathRoom/switch3"
#define SW_3_COMMAND_TOPIC "EspBathRoom/switch3/set"

// Global var
WiFiClient espClient;          // Creates a client that can connect to to a specified internet IP address and port as defined in
PubSubClient mqtt(espClient);  // Khởi tạo MQTT
UpdateOTA updateOTA;           // Update OTA
MySwitch switchLight;          // Custom class => khởi tạo đối tượng công tắc đèn
MySwitch switchFan;            // Custom class => Khởi tạo đối tượng công tắc quạt
MySwitch switchFrontLight;     // Custom class => Khởi tạo đối tượng công tắc quạt

// Setup board
// ====================================================
void setup() {
  Serial.begin(115200);  // Initialize serial communication
  Serial.println();

  // Connect to wifi
  Logger("Connecting to wifi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // Debug local IP
  Logger("Wifi connected.. IP address: ");
  Logger(WiFi.localIP());

  // Setup update OTA
  Logger("Start setup OTA update...");
  updateOTA.init(WIFI_PASSWORD);

  // Setup MQTT server
  Logger("Start setup MQTT...");
  mqtt.setServer(MQTT_SERVER, MQTT_SERVER_PORT);
  mqtt.setCallback(mqttCallback);

  // Setup đối tượng công tắc
  // Param: Name, stateTopic, commandTopic, relayPin, touchModulePin, ledPin
  Logger("Start setup Switch object...");
  switchLight.init(SW_1_NAME, SW_1_STATE_TOPIC, SW_1_COMMAND_TOPIC, SW_1_RELAY_PIN, SW_1_BUTTON_PIN);
  switchFan.init(SW_2_NAME, SW_2_STATE_TOPIC, SW_2_COMMAND_TOPIC, SW_2_RELAY_PIN, SW_2_BUTTON_PIN);
  switchFrontLight.init(SW_3_NAME, SW_3_STATE_TOPIC, SW_3_COMMAND_TOPIC, SW_3_RELAY_PIN, SW_3_BUTTON_PIN);

  Logger("Setup finished...");
}

// Control relay by press on button
// ====================================================
void loop() {
  // Update OTA handle
  updateOTA.handle();

  // Set state by button
  switchLight.setStateManual(mqtt);
  switchFan.setStateManual(mqtt);
  switchFrontLight.setStateManual(mqtt);

  // Auto reconnect to MQTT server
  if (!mqtt.connected()) reconnectMQTT();

  // Loop mqtt
  mqtt.loop();
}

// Control relay by Home Assistant (mqtt.publish)
// ====================================================
void mqttCallback(char *_topic, byte *_payload, unsigned int _length) {
  int i;
  char messageStr[100];
  for (i = 0; i < _length; i++) messageStr[i] = _payload[i];
  messageStr[i] = '\0';

  // Set state by MQTT
  // Gọi tất cả các công tắc, cái nào trùng vs topic thì mới thay đổi trạng thái
  switchLight.setStateMQTT(mqtt, String(messageStr), _topic);
  switchFan.setStateMQTT(mqtt, String(messageStr), _topic);
  switchFrontLight.setStateMQTT(mqtt, String(messageStr), _topic);
}

// Reconnect to MQTT server
// ====================================================
void reconnectMQTT() {
  Logger("Attempting MQTT connection...");
  while (!mqtt.connected()) {
    if (mqtt.connect(MQTT_NAME, MQTT_SERVER_USERNAME, MQTT_SERVER_PASSWORD)) {
      Logger("MQTT connected!!!");
      switchLight.subscribeMQTT(mqtt);
      switchFan.subscribeMQTT(mqtt);
      switchFrontLight.subscribeMQTT(mqtt);
    } else {
      Logger("Failed, rc=" + String(mqtt.state()) + " try again in 5 seconds...");
      // delay(5000);  // Wait 5 seconds before retrying
    }
  }
}
