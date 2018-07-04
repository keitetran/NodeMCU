/*
  Config Home Assistant (domain: light or switch is ok)
  light: #(or domain: switch)
  - platform: mqtt
    name: 'node1'
    state_topic: 'ESP_LIGHT/state/light01'
    command_topic: 'ESP_LIGHT/cmd/light01'
    payload_on: "1"
    payload_off: "0"
    qos: 0
    retain: true
  ====================================================

  Lần chạy đầu tiên sau khi nạp code.
  - Kết nối wifi, server MQTT theo cấu hình mặc định
  - Không thể kết nối đến wifi sẽ hiện potal để cấu hình
*/

#include <Arduino.h>        // Arduino
#include <ESP8266WiFi.h>    // ESP 8266
#include <PubSubClient.h>   // MQTT
#include <Wire.h>           // MQTT
#include "MySwitch.h"       // Custom class
#include "MyWifiManager.h"  // Custom class
WiFiClient espClient;
PubSubClient client(espClient);
MySwitch switchLight;
MySwitch switchFan;
MyWifiManager wifiManager;

// Setup board
// ------------------------------------------------
void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  Serial.println();
  delay(10);

  // setup wifi manager
  // Param: clientId, server ip, port, username, password => all string
  wifiManager.init("ESP1", "192.168.1.105", "1883", "abc", "xyz");

  // Setup MQTT server
  client.setServer(wifiManager.mqttServer, wifiManager.mqttPortNum);
  client.setCallback(mqttCallback);

  // Setup đối tượng công tắc
  // Param: stateTopic, commandTopic, relayPin, touchModulePin, ledPin
  switchLight.init(client, "ESP_LIGHT/state/light01", "ESP_LIGHT/cmd/light01",
                   12, 5, 3);
  switchFan.init(client, "ESP_LIGHT/state/light02", "ESP_LIGHT/cmd/light02", 11,
                 6, 1);

  // TODO: Chỗ này chưa hiểu rõ, đợi có mạch kiểm tra lại
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    switchLight.setDefaultState();
    switchFan.setDefaultState();
    digitalWrite(LED_BUILTIN, LOW);
  }

  // Nháy 3 lần => kết nối WIFI thành công.
  blinkLedBuildIn(3);

  // Setup PIN led build in
  pinMode(LED_BUILTIN, OUTPUT);
}

// Control relay by press on button
// ------------------------------------------------
void loop() {
  // Set state
  switchLight.setStateSync();
  switchFan.setStateSync();

  // Auto reconnect to MQTT server
  if (!client.connected()) reconnect();

  // Loop client
  client.loop();
}

// Control relay by Home Assistant (mqtt.publish)
// ------------------------------------------------
void mqttCallback(char *topic, byte *payload, unsigned int length) {
  int i;
  char messageBuff[100];
  for (i = 0; i < length; i++) {
    messageBuff[i] = payload[i];
  }
  messageBuff[i] = '\0';

  // Gọi tất cả các công tắc, cái nào trùng vs topic thì mới thay đổi trạng thái
  switchLight.sendMQTTMessage(String(messageBuff), topic);
  switchFan.sendMQTTMessage(String(messageBuff), topic);
}

// Reconnect to MQTT server
// ------------------------------------------------
int reconnectCounter = 0;
void reconnect() {
  Serial.print("Attempting MQTT connection...");
  while (!client.connected()) {
    reconnectCounter++;
    delay(100);

    if (reconnectCounter == 150) {
      if (client.connect(wifiManager.mqttClientId, wifiManager.mqttUserName, wifiManager.mqttPassword)) {
        Serial.println("MQTT connected!!!");
        switchLight.subscribeMQTT();
        switchFan.subscribeMQTT();
      }
      reconnectCounter = 0;
    } else {
      switchLight.setDefaultState();
      switchFan.setDefaultState();
    }
  }
}

// Blink led on board
// @param: (int) num - Number blick led
// ------------------------------------------------
void blinkLedBuildIn(int num) {
  for (int i = 0; i < num; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
  }
}
