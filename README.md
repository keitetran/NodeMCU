# Wemos-D1-Toutch-Switch

Project touth switch D1 mini with HASS

- Edit your info on MyConfig.h
- Config HASS with below

```
switch: #(or domain: light)
  - platform: mqtt
    name: 'node1'
    state_topic: 'ChipId/switchName'
    command_topic: 'ChipId/switchName/set'
    payload_on: "1"
    payload_off: "0"
    qos: 0
    retain: true
```
