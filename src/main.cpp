#include <main.h>


uint32_t state = 0;
uint32_t battery = 0;
uint32_t sleep_hour = 19;
uint32_t wakeup_hour = 7;
uint32_t error_code = 0;
uint32_t battery_threshold = 0;
RTCZero rtc;


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


void sendDataToSigfox(uint32_t data){
  if(!SigFox.begin()){
      bitSet(error_code, ERROR_SIGFOX_BEGIN);
  }
  delay(100);
  SigFox.beginPacket();
  SigFox.write(data);
  int ret = SigFox.endPacket(); 
  if (ret > 0) {
    bitSet(error_code, ERROR_SIGFOX_TRANSMIT);
  }
  SigFox.end();
  bitSet(state, FLAG_SIGFOX_TRANSMITTED);
}

uint32_t getTimeFromSigfox(){
  if(!SigFox.begin()){
    bitSet(error_code, ERROR_SIGFOX_BEGIN);
  }
  delay(100);
  SigFox.beginPacket();
  SigFox.write(0);
  int ret = SigFox.endPacket(true); 
  if (ret > 0) {
    bitSet(error_code, ERROR_SIGFOX_TRANSMIT);
  }

  uint8_t time_buf[8] = {0,0,0,0,0,0,0,0};
  uint8_t i = 0;
  if (SigFox.parsePacket()) {
    while (SigFox.available()) {
      time_buf[i] = SigFox.read(); 
      i++;
    }
  }
  SigFox.end();
  return (uint32_t) (time_buf[0] << 24 | time_buf[1] << 16 | time_buf[2] << 8 | time_buf[3]);
}

void setRTCTime(uint32_t unix_time){
  time_t t = unix_time;
  rtc.setTime(hour(t), minute(t), second(t));
  rtc.setDate(day(t), month(t), year(t)-2000);
  bitSet(state, FLAG_TIME_REFRESHED);
}

void setAlarmForNextCycle(){
  rtc.detachInterrupt();
  rtc.attachInterrupt(alarmNextCycle);
  rtc.setAlarmTime(00, (rtc.getMinutes()+10)%60, 00);
  rtc.enableAlarm(rtc.MATCH_MMSS);
  rtc.standbyMode();
}

void setAlarmForNextDay(){
  rtc.detachInterrupt();
  rtc.attachInterrupt(alarmFirstCycle);
  rtc.setAlarmTime(wakeup_hour, 00, 00);
  rtc.enableAlarm(rtc.MATCH_HHMMSS);
  rtc.standbyMode();
}

void powerUpRPI(){
  digitalWrite(PIN_POWER_RPI, HIGH);
  while(bitRead(state, FLAG_RPI_POWER) == 0){};
}

void powerDownRPI(){
  while(bitRead(state, FLAG_RPI_POWER) == 1){};
  delay(5000);
  digitalWrite(PIN_POWER_RPI, LOW);
}

void alarmFirstCycle(){
    bitSet(state, FLAG_FIRST_CYCLE);
    cycle();
}

void alarmNextCycle(){
    bitClear(state, FLAG_FIRST_CYCLE);
    cycle();
}

void cycle(){
  rtc.disableAlarm();
  if(rtc.getHours() < sleep_hour && rtc.getHours() > wakeup_hour){
    battery = analogRead(PIN_BATTERY);
    if(battery > battery_threshold){
      powerUpRPI();
      powerDownRPI();
    }
    setAlarmForNextCycle();
  }else{
    setAlarmForNextDay();
  }

}


// FIRST STARTUP
void setup()
{
  // SERIAL INIT
  Serial.begin(9600);
  while (!Serial){};

  // I2C INIT
  Wire.begin(MKRFOX_ADDR);
  Wire.onReceive(receiveI2C); // register event

  // RTC INIT
  rtc.begin();

  // PIN INIT
  pinMode(PIN_POWER_RPI, OUTPUT);

  // REGISTER INIT
  state = 0; 
  battery = 0;
  battery_threshold = 11;
  sleep_hour = 19;
  wakeup_hour = 7;
  error_code = 0;

  setRTCTime(1634293107);

  // RTC INIT
  rtc.attachInterrupt(alarmFirstCycle);
  setAlarmForNextCycle();
  rtc.standbyMode();
}

void loop()
{


}