/*
  MySwitch.cpp - Class Myswitch
  Created by Keite Trần, 2018/07/02.
  Released into the public domain.
*/
#include <Arduino.h>
#include <PubSubClient.h>
#include "./Config.h";

class MySwitch {
 public:
  MySwitch();
  char *switchName;
  char *stateTopic;                                                       // State topic MQTT
  char *cmdTopic;                                                         // Command topic MQTT
  int relayModulePin;                                                     // Chân PIN của relay
  int touchModulePin;                                                     // Chân PIN của touch module
  int switchOldState;                                                     // Trạng thái hiện tại của touth module
  void init(char *a, char *b, char *c, int d, int e);                     // Cài đặt đối tượng MySwitch
  void setStateMQTT(PubSubClient mqtt, String _state, String _cmdTopic);  // Gán state cho relay vs touch module từ MQTT
  void setStateManual(PubSubClient mqtt);                                 // Gán trạng thái từ nút ấn vật lý
  void subscribeMQTT(PubSubClient mqtt);                                  // Subscribe mqtt

 private:
  void setState(PubSubClient mqtt, String _state);  // Gán trạng thái cho relay và led
  int touchModuleOldState;                          // Touth module old state
};

MySwitch::MySwitch(){};

// Setup PIN mode
// ------------------------------------------------
void MySwitch::init(char *a, char *b, char *c, int d, int e) {
  switchName = a;
  stateTopic = b;
  cmdTopic = c;
  relayModulePin = d;
  touchModulePin = e;

  // Default value
  switchOldState = 0;
  touchModuleOldState = 0;

  // Set pin mode
  pinMode(touchModulePin, INPUT_PULLUP);
  pinMode(relayModulePin, OUTPUT);

  // Default pin value
  digitalWrite(relayModulePin, LOW);
};

// Subscribe MQTT topic
// ------------------------------------------------
void MySwitch::subscribeMQTT(PubSubClient mqtt) {
  mqtt.subscribe(stateTopic);
  mqtt.subscribe(cmdTopic);
};

// Gán state cho relay vs touch module
// ------------------------------------------------
void MySwitch::setStateMQTT(PubSubClient mqtt, String _state, String _cmdTopic) {
  // Khác topic MQTT => bỏ qua
  if (_cmdTopic != String(cmdTopic)) return;

  // Set relay state
  setState(mqtt, _state);
};

// Đồng bộ trạng thái qua hàm loop
// ------------------------------------------------
void MySwitch::setStateManual(PubSubClient mqtt) {
  // Lấy trạng thái của chân pin
  int touchModuleState = digitalRead(touchModulePin);
  int relayModuleState = digitalRead(relayModulePin);

  // Kiểm tra trạng thái hiện tại => giống thì bỏ qua
  if (touchModuleState == touchModuleOldState) return;

  // Thay đổi trạng thái relay
  setState(mqtt, String(relayModuleState));

  // Cập nhật trạng thái button
  touchModuleOldState = touchModuleState;
};

// Gán state cho relay vs touch module
// ------------------------------------------------
void MySwitch::setState(PubSubClient mqtt, String _state) {
  Logger("MQTT set state... " + String(switchName) + ": " + _state);

  // Gửi dữ liệu qua MQTT server
  mqtt.publish(stateTopic, _state.c_str(), true);

  // Kiểm tra trạng thái hiện tại => giống thì bỏ qua
  if (switchOldState == _state.toInt()) return;

  // Cập nhật trạng thái của switch
  switchOldState = _state.toInt();

  // Relay state
  if (_state == _OFF) {
    // Chuyển trạng thái sang ON
    digitalWrite(relayModulePin, HIGH);
  } else {
    // Chuyển trạng thái sang OFF
    digitalWrite(relayModulePin, LOW);
  }
};