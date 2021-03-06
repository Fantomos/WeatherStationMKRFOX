// #define VERY_LOW_POWER 1 // Permet lors de l'initialisation du framework Arduino (wiring.c) de ne pas initialiser les PIN par défaut pour diminuer la consomation en sleep mode
#include <Arduino.h>
#include <SigFox.h>
#include <ArduinoLowPower.h>
#include <RTCZero.h>
#include <Wire.h>
#include <TimeLib.h>
#include <Timezone.h>

#define MKRFOX_ADDR 0x55 // Définition de l'addresse I2C du MKRFOX

// Définition des adresses d'accées des variables par I2C
#define REG_TIME 0x00
#define REG_STATE 0x01
#define REG_ERROR 0x02
#define REG_SLEEP 0x03
#define REG_WAKE 0x04
#define REG_DATA 0x05
#define REG_BATTERY 0x06
#define REG_BATTERY_THRESHOLD 0x07

// Définition des bits d'erreur
#define ERROR_SIGFOX_BEGIN 0
#define ERROR_SIGFOX_TRANSMIT 1
#define ERROR_I2C_REG_NOT_FOUND 2

// Définition des bits de flag
#define FLAG_RPI_POWER 0 
#define FLAG_TIME_REFRESHED 1
#define FLAG_FIRST_CYCLE 2
#define FLAG_SHUTDOWN_RPI 3

// Définition des numéros de PIN
#define PIN_POWER_5V 0
#define PIN_BATTERY PIN_A0
#define PIN_POWER_ATTINY 13


// Définition des valeurs par défauts
#define DEFAULT_SLEEP_HOUR 19
#define DEFAULT_WAKEUP_HOUR 8
#define DEFAULT_BATTERY_THRESHOLD 11000

// Définition de constante
#define CYCLE_TIME 10
#define BATTERY_CONSTANT 14.978485

// Définition des timeouts
#define TIMEOUT_WAKEUP 120000
#define TIMEOUT_CYCLE 120000
#define TIMEOUT_SHUTDOWN 4000

// Flag definissant si le Raspberry doit être éteint après avoir fini son cycle
#define SHUTDOWN_RPI true

// Prototype des fonctions
void sendI2C();
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

/**
 * \struct sigfox_message
 * \brief Message transmis via Sigfox.
 *
 * sigfox_message est une structure composé de toutes les données 
 * à transmettre et formaté pour l'envoie via Sigfox.
 */
typedef struct __attribute__ ((packed)) sigfox_message {
  uint8_t speed;
  uint8_t speed_max;
  uint16_t direction;
  uint16_t direction_max;
  uint8_t humidity;
  uint8_t temperature;
  uint16_t pressure;
  uint16_t voltage;
} SigfoxMessage;