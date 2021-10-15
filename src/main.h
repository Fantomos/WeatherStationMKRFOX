#include <Arduino.h>
#include <SigFox.h>
#include <ArduinoLowPower.h>
#include <RTCZero.h>
#include <Wire.h>

#define MKRFOX_ADDR 0x55

#define REG_TIME 0x01
#define REG_ALARM 0x02
#define REG_ALARM_CON 0x03
#define REG_WIND_DIR 0x04
#define REG_WIND_SPEED 0x05
#define REG_STATE_RPI 0x06
#define REG_STATE_PIC 0x07
#define REG_BATTERY 0x08
#define REG_SLEEP 0x09
#define REG_EVEIL 0x10

