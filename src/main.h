#define VERY_LOW_POWER 1
#include <Arduino.h>
#include <SigFox.h>
#include <ArduinoLowPower.h>
#include <RTCZero.h>
#include <Wire.h>
#include <TimeLib.h>
#include <Timezone.h>

#define MKRFOX_ADDR 0x55

#define REG_TIME 0x00
#define REG_STATE 0x07
#define REG_BATTERY 0x08
#define REG_BATTERY_THRESHOLD 0x08
#define REG_SLEEP 0x09
#define REG_WAKE 0x10
#define REG_ERROR 0x11
#define REG_DATA 0x12

#define ERROR_SIGFOX_BEGIN 0
#define ERROR_SIGFOX_TRANSMIT 1
#define ERROR_I2C_REG_NOT_FOUND 2

#define FLAG_RPI_POWER 0
#define FLAG_TIME_REFRESHED 1
#define FLAG_FIRST_CYCLE 2
#define FLAG_RPI_DATA_READY 3
#define FLAG_SIGFOX_TRANSMITTED 4

#define PIN_POWER_5V 1
#define PIN_BATTERY 0

#define DEFAULT_SLEEP_HOUR 19
#define DEFAULT_WAKEUP_HOUR 8
#define DEFAULT_BATTERY_THRESHOLD 11

#define CYCLE_TIME 10
#define BATTERY_CONSTANT 0.0149560117

void receiveI2C(int packetSize);
void sendDataToSigfox(uint32_t data);
uint32_t getTimeFromSigfox();
void setRTCTime(uint32_t unix_time);
void setAlarmForNextCycle();
void setAlarmForNextDay();
void powerUpRPI();
void powerDownRPI();
void alarmFirstCycle();
void alarmNextCycle();