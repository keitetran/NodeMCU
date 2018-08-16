/*
  EspBathRoomSwitch.ino - Toilet switch
  Created by Keite Trần, 2018/07/02.
  Released into the public domain.
*/

// Config file (Keep it above)
#include "./MyConfig.h";

// Library
#include <Arduino.h>       // Arduino
#include <ESP8266WiFi.h>   // https://www.arduino.cc/en/Reference/WiFiClient
#include <PubSubClient.h>  // https://github.com/knolleary/pubsubclient
#include "./MySwitch.h"    // Custom class MySwitch
#include "./UpdateOTA.h"   // Custom class update OTA

// Global var
WiFiClient espClient;          // Creates a client that can connect to to a specified internet IP address and port as defined in
PubSubClient mqtt(espClient);  // Khởi tạo MQTT
UpdateOTA updateOTA;           // Update OTA
MySwitch switchLight;          // Custom class light
MySwitch switchFan;            // Custom class fan
MySwitch switchFrontLight;     // Custom class front light
uint32_t chipId;               // ESP chip ID

// Setup board
// ====================================================
void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  Loggerln("");

  // Get chip ID
  chipId = ESP.getChipId();
  Loggerln("Chip ID: " + String(chipId));

  // Connect to wifi
  delay(3000);
  Logger("Connecting to wifi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Logger(".");
  }

  // Debug local IP
  Loggerln("");
  Logger("WiFi connected, IP address: ");
  Loggerln(WiFi.localIP());

  // Setup update OTA
  Loggerln("Start setup OTA update...");
  updateOTA.init(WIFI_PASSWORD);

  // Setup đối tượng công tắc
  // Param: Name, relayPin, touchModulePin
  Loggerln("Start setup Switch object...");
  switchLight.init(SW_1_NAME, SW_1_RELAY_PIN, SW_1_BUTTON_PIN);
  switchFan.init(SW_2_NAME, SW_2_RELAY_PIN, SW_2_BUTTON_PIN);
  switchFrontLight.init(SW_3_NAME, SW_3_RELAY_PIN, SW_3_BUTTON_PIN);

  // Setup MQTT server
  Loggerln("Start setup MQTT...");
  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.setCallback(MqttCallback);
  Loggerln("Setup finished...");
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
  static uint8_t startIndex = 0;
  startIndex = startIndex + 1; /* motion speed */
  if (!mqtt.connected()) ConnectToMQTT();

  // Loop mqtt
  mqtt.loop();

  // Hold button for restart ESP
  // HoldButtonForRestart();
}

// Control relay by Home Assistant (mqtt.publish)
// ====================================================
void MqttCallback(char *_topic, byte *_payload, unsigned int _length) {
  char messageStr[100];
  for (int i = 0; i < _length; i++) {
    messageStr[i] = _payload[i];
    // Replace last item
    if (i == (_length - 1)) messageStr[i] = '\0';
  }

  // Set state by MQTT
  // Gọi tất cả các công tắc, cái nào trùng vs topic thì mới thay đổi trạng thái
  switchLight.setStateMQTT(mqtt, String(messageStr), _topic);
  switchFan.setStateMQTT(mqtt, String(messageStr), _topic);
  switchFrontLight.setStateMQTT(mqtt, String(messageStr), _topic);
}

// Reset ESP via button hold
// ====================================================
int restartFlag = 0;
void HoldButtonForRestart() {
  int btnState = digitalRead(SW_1_BUTTON_PIN);
  if (btnState == HIGH)
    restartFlag++;
  else
    restartFlag = 0;

  if (restartFlag == 5000) {
    ESP.reset();
    Loggerln("ESP reset...");
  }
}

// Reconnect to MQTT server
// ====================================================
void ConnectToMQTT() {
  Loggerln("Attempting MQTT connection...");
  while (!mqtt.connected()) {
    if (mqtt.connect(String(chipId).c_str(), MQTT_USERNAME, MQTT_PASSWORD)) {
      Loggerln("MQTT connected!!! - " + String(MQTT_USERNAME) + "@" + String(MQTT_SERVER) + ":" + String(MQTT_PORT));
      switchLight.subscribeMQTT(mqtt);
      switchFan.subscribeMQTT(mqtt);
      switchFrontLight.subscribeMQTT(mqtt);
    } else {
      Loggerln("Failed, rc=" + String(mqtt.state()) + " try again in 5 seconds...");
      // delay(5000);  // Wait 5 seconds before retrying
    }
  }
}