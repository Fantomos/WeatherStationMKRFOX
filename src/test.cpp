#include <SigFox.h>
#include <ArduinoLowPower.h>
#include <RTCZero.h>
#include <Wire.h>

RTCZero rtc;

#define MKRFOX_ADDR 0x55

#define REG_TIME 0x01
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

void receiveI2C(int packetSize)
{
  Serial.println("Receiving... ");
  Serial.println(packetSize);

  if (packetSize == 1) // READ OPERATION
  {
    byte reg = Wire.read();
    switch (reg)
    {
    case REG_SLEEP:
      Wire.write(sleep_hour);
      break;

    case REG_WAKE:
      Wire.write(wakeup_hour);
      break;

    case REG_ERROR:
      Wire.write(error_code);
      break;

    case REG_TIME:
      Wire.write((u_int32_t)rtc.getEpoch());
      break;

    case REG_STATE:
      Wire.write(state);
      break;

    case REG_BATTERY:
      Wire.write(battery);
      break;

    default:
      bitSet(error_code, ERROR_I2C_REG_NOT_FOUND);
      break;
    }
  }
  else if (packetSize > 1)  // WRITE OPERATION
  {
    byte reg = Wire.read();
    uint32_t val;
    switch (packetSize)
    {
      case 2:
        val =  Wire.read();
        break;

      case 3:
        val = Wire.read() << 8 | Wire.read();
        break;

      case 5:
        val = Wire.read() << 24 | Wire.read() << 16 | Wire.read() << 8 | Wire.read();
        break;

      default:
        val =  Wire.read();
        break;
    }
    Serial.println(val);
    switch (reg)
    {
      case REG_SLEEP:
        sleep_hour = val;
        break;

      case REG_WAKE:
        wakeup_hour = val;
        break;

      case REG_ERROR:
        error_code = val;
        break;

      case REG_STATE:
        state = val;
        break;

      case REG_TIME:
        if(val == 0){
          setRTCTime(getTimeFromSigfox());
        }else{
          setRTCTime(val);
        }
        break;

      case REG_DATA:
        sendDataToSigfox(val);
        break;

      case REG_BATTERY_THRESHOLD:
        battery_threshold = val;
        break;

      default:
        bitSet(error_code, ERROR_I2C_REG_NOT_FOUND);
        break;
    }
  }
}



void setup() {
    // I2C INIT
  Wire.begin(MKRFOX_ADDR);
  Wire.onReceive(receiveI2C); // register event

  // RTC INIT
  rtc.begin();

  // PIN INIT
  pinMode(PIN_POWER_5V, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  // Uncomment this function if you wish to attach function dummy when RTC wakes up the chip
  // LowPower.attachInterruptWakeup(RTC_ALARM_WAKEUP, dummy, CHANGE);
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
  // Triggers a 2000 ms sleep (the device will be woken up only by the registered wakeup sources and by internal RTC)
  // The power consumption of the chip will drop consistently
  LowPower.sleep(2000);
}

void dummy() {
  // This function will be called once on device wakeup
  // You can do some little operations here (like changing variables which will be used in the loop)
  // Remember to avoid calling delay() and long running functions since this functions executes in interrupt context
}