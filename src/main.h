#include <Arduino.h>
#include <SigFox.h>
#include <ArduinoLowPower.h>
#include <RTCZero.h>
#include <Wire.h>
#include <TimeLib.h>

#define MKRFOX_ADDR 0x55

#define REG_TIME 0x01
#define REG_STATE 0x07
#define REG_BATTERY 0x08
#define REG_SLEEP 0x09
#define REG_WAKE 0x10
#define REG_ERROR 0x11
#define REG_DATA 0x12


#define ERROR_SIGFOX_BEGIN 0
#define ERROR_SIGFOX_TRANSMIT 1
#define ERROR_I2C_REG_NOT_FOUND 2

#define FLAG_RPI_POWER 0
#define FLAG_PIC_TIME_REFRESHED 2
#define FLAG_FIRST_CYCLE 3
#define FLAG_RPI_DATA_READY 4
#define FLAG_SIGFOX_TRANSMITTED 5
#define FLAG_REQUEST_SHUTDOWN 6

#define PIN_POWER_RPI 1
#define PIN_BATTERY 1