/*
  MySwitch.cpp - Class Myswitch
  Created by Keite Trần, 2018/07/02.
  Released into the public domain.
*/

// Config file (Keep it above)
#include "./MyConfig.h";

// Libraries
#include <Arduino.h>
#include <PubSubClient.h>

class MySwitch {
 public:
  MySwitch();
  String switchName;                                                      // Switch name
  uint32_t chipId;                                                        // Chip ID
  char *stateTopic;                                                       // State topic MQTT
  char *cmdTopic;                                                         // Command topic MQTT
  int relayModulePin;                                                     // Chân PIN của relay
  int touchModulePin;                                                     // Chân PIN của touch module
  void init(String a, int b, int c);                                      // Cài đặt đối tượng MySwitch
  void setStateMQTT(PubSubClient mqtt, String _state, String _cmdTopic);  // Gán state cho relay vs touch module từ MQTT
  void setStateManual(PubSubClient mqtt);                                 // Gán trạng thái từ nút ấn vật lý
  void subscribeMQTT(PubSubClient mqtt);                                  // Subscribe mqtt

 private:
  void setState(PubSubClient mqtt, String _state);  // Gán trạng thái cho relay
  int touchModuleOldState;                          // Touth module old state
  int switchOldState;                               // Trạng thái hiện tại của touth module
};

// Constructor
// ------------------------------------------------
MySwitch::MySwitch(){};

// Setup init
// ------------------------------------------------
void MySwitch::init(String a, int b, int c) {
  // Switch Name
  switchName = a;

  // ESP chip ID
  chipId = ESP.getChipId();

  // State topic MQTT -> chipID/switchName
  String stateTopic = String(chipId) + "/" + switchName;
  char __stateTopic[sizeof(stateTopic)];                       // Bufer
  stateTopic.toCharArray(__stateTopic, sizeof(__stateTopic));  // Convert to const char *
  Loggerln("State topic: " + String(stateTopic));

  // Command topic MQTT -> chipID/switchName/set
  String cmdTopic = String(chipId) + "/" + switchName + "/set";
  char __cmdTopic[sizeof(cmdTopic)];                     // Bufer
  cmdTopic.toCharArray(__cmdTopic, sizeof(__cmdTopic));  // Convert to const char *
  Loggerln("Command topic: " + String(cmdTopic));

  // Pin define
  relayModulePin = b;
  touchModulePin = c;

  // Default value
  switchOldState = 0;
  touchModuleOldState = 0;

  // Set pin mode
  pinMode(touchModulePin, INPUT_PULLUP);
  pinMode(relayModulePin, OUTPUT);

  // Default pin value => Cần để low vì khi khởi động lên thì
  // ESP sẽ reset lại trạng thái của nút vs relay về OFF -> rồi kết publish MQTT
  // Khi đó HASS sẽ tưởng là trạng thái thay đổi và set off toàn bộ các switch
  // -> Đến khi kết nối MQTT thành công thì ESP sẽ nhận toàn bộ trạng thái mới nhất từ HASS
  // Lúc này thì trạng thái của RELAY sẽ được đồng bộ từ HASS về.
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
  Loggerln("MQTT set state... " + switchName + ": " + _state);
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
  Loggerln("Manual set state... " + switchName + ": " + String(relayModuleState));
};

// Gán state cho relay vs touch module
// ------------------------------------------------
void MySwitch::setState(PubSubClient mqtt, String _state) {
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