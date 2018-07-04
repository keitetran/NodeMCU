
/*
  MySwitch.cpp - Class Myswitch
  Created by Keite Trần, 2018/07/02.
  Released into the public domain.
*/
#ifndef MySwitch_h
#define MySwitch_h

#include <Arduino.h>
#include <PubSubClient.h>

class MySwitch {
 public:
  char *stateTopic;                                                    // state topic MQTT
  char *cmdTopic;                                                      // command topic MQTT
  int relayModulePin;                                                  // Chân PIN của relay
  int touchModulePin;                                                  // Chân PIN của touch module
  int ledPin;                                                          // Chân đèn led
  PubSubClient client;                                                 // MQTT client object
  void sendMQTTMessage(String _state, String _topicStr);               // Gán state cho relay vs touch module từ MQTT
  void setStateSync();                                                 // Gán trạng thái từ nút ấn vật lý
  void setDefaultState();                                              // Gán trạng thái mặc định
  void subscribeMQTT();                                                // subscribe mqtt
  void init(PubSubClient psc, char *a, char *b, int c, int d, int e);  // Cài đặt đối tượng mýwitch
  String getState();                                                   // Lấy trạng thái hiện tại

 private:
  int switchState;               // Trạng thái hiện tại của touth module
  int switchOldState;            // trạng thái cũ
  char *payloadOn;               // MQTT payload ON
  char *payloadOff;              // MQTT payload OFF
  void setState(String _state);  // Gán trạng thái cho relay và led
};
#endif
