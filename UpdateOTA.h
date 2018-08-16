
/*
  UpdateOTA.h - Class UpdateOTA
  Created by Keite Trần, 2018/07/02.
  Released into the public domain.
*/

#include <Arduino.h>
#include <ArduinoOTA.h>
class UpdateOTA {
 public:
  void init(char* _password);
  void handle();
};

void UpdateOTA::init(char* _password) {
  // Port defaults to 8266
  // ArduinoOTA.setPort(port);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  if (_password != "") ArduinoOTA.setPassword(_password);

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else {  // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Logger("Start updating " + type);
  });

  ArduinoOTA.onEnd([]() {
    Logger("\nEnd");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Logger("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Logger("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Logger("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Logger("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Logger("End Failed");
    }
  });

  ArduinoOTA.begin();
};

void UpdateOTA::handle() {
  ArduinoOTA.handle();
};