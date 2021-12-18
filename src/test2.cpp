// #include <Arduino.h>
// #include <main.h>
// RTCZero rtc;

// uint32_t getTimeFromSigfox(){
//   if(!SigFox.begin()){
    
//   }
//   delay(100);
//   SigFox.status();
//   delay(1);
//   SigFox.beginPacket();
//   SigFox.write(0);
//   int ret = SigFox.endPacket(true); 
//   if (ret > 0) {

//   }

//   uint8_t time_buf[8] = {0,0,0,0,0,0,0,0};
//   uint8_t i = 0;
//   if (SigFox.parsePacket()) {
//     while (SigFox.available()) {
//       time_buf[i] = SigFox.read(); 
//       i++;
//     }
//   }
//   SigFox.end();
//   uint32_t time = (uint32_t) (time_buf[0] << 24 | time_buf[1] << 16 | time_buf[2] << 8 | time_buf[3]);
//   Serial.println(time);
//   // time = (uint32_t) Convert_to_France.toLocal(time);
//   return  time;
// }

// void setRTCTime(uint32_t unix_time){
//   time_t t = unix_time;
//   Serial.println(t);
//   rtc.setTime(hour(t), minute(t), second(t));
//   rtc.setDate(day(t), month(t), year(t)-2000);
// }

// void setup(){
//     pinMode(LED_BUILTIN,OUTPUT);
//      // SigFox INIT
//   SigFox.begin();
//   delay(200);
//   SigFox.debug();
//   SigFox.end();
//   delay(200);

//   // RTC INIT
//   rtc.begin(false);
//     Serial.begin(9600);
//     while(!Serial.available());
//     Serial.println("Go");
    
//     delay(10000);

//     setRTCTime(getTimeFromSigfox());

//     Serial.println(rtc.getEpoch());
    
// }

// void loop(){
//     digitalWrite(LED_BUILTIN, LOW);
//     delay(1000);
//     digitalWrite(LED_BUILTIN, HIGH);
//     delay(1000);
// }
