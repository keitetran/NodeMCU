/**
 * MySwitch.cpp - Class Myswitch
 * Created by Keite Trần, 2018/07/02.
 * Released into the public domain.
**/
#include "MySwitch.h"
#include <Arduino.h>
#include <PubSubClient.h>

// Setup PIN mode
// ------------------------------------------------
void MySwitch::init(PubSubClient psc, char *a, char *b, int c, int d, int e) {
  pinMode(touchModulePin, INPUT);
  pinMode(relayModulePin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  client = psc;
};

void MySwitch::subscribeMQTT() {
  client.subscribe(stateTopic);
  client.subscribe(cmdTopic);
};

// Lấy trạng thái hiện tại của công tắc
// ------------------------------------------------
String MySwitch::getState() {
  return String(switchState);
};

// Gán state cho relay vs touch module
// ------------------------------------------------
void MySwitch::sendMQTTMessage(String _state, String _topicStr) {
  // Khác topic MQTT => bỏ qua
  if (_topicStr != String(stateTopic))
    return;

  // Lấy trạng thái của chân pin
  switchState = digitalRead(relayModulePin);

  // Kiểm tra trạng thái hiện tại => giống thì bỏ qua
  if (String(switchState) == _state)
    return;

  setState(_state);
};

// Đồng bộ trạng thái qua hàm loop
// ------------------------------------------------
void MySwitch::setStateSync() {
  // Lấy trạng thái của chân pin
  switchState = digitalRead(touchModulePin);

  // Kiểm tra trạng thái hiện tại => giống thì bỏ qua
  if (switchState == switchOldState)
    return;

  setState(String(switchState));
};

// Gán state cho relay vs touch module
// ------------------------------------------------
void MySwitch::setState(String _state) {
  // Gán state cũ bằng state mới
  switchOldState = _state.toInt();

  // Nếu ON
  if (_state == "1") {
    // Chuyển trạng thái sang ON
    digitalWrite(relayModulePin, HIGH);
    digitalWrite(ledPin, HIGH);

    // Gửi dữ liệu qua MQTT
    client.publish(stateTopic, payloadOn, true);
    Serial.println("Switch ON");
    return;  // Bỏ qua không chạy tiếp code nếu phía dưới
  }

  // Nếu OFF
  if (_state == "0") {
    // Chuyển trạng thái sang OFF
    digitalWrite(relayModulePin, LOW);
    digitalWrite(ledPin, LOW);

    // Gửi dữ liệu qua MQTT
    client.publish(stateTopic, payloadOff, true);
    Serial.println("Switch OFF");
  }
};

// Gán trạng thái mặc định
// Đồng bộ trạng thái giữa module touth vs relay khi kết nối lại wifi
// ------------------------------------------------
void MySwitch::setDefaultState() {
  // Lấy trạng thái của chân toutch module
  switchState = digitalRead(touchModulePin);

  // Trạng thái hiện tại giống trạng thái cũ => bỏ qua
  if (switchState == switchOldState)
    return;

  // Nếu đang mở => chuyển hết về tắt
  if (switchState == 1) {
    digitalWrite(relayModulePin, LOW);
    digitalWrite(ledPin, LOW);
  }

  // Nếu đang tắt => chuyển hết về mở
  else if (switchState == 0) {
    digitalWrite(relayModulePin, HIGH);
    digitalWrite(ledPin, HIGH);
  }

  // Reset lại trạng thái
  switchOldState = switchState;
};
