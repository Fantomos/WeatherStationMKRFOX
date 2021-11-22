// #include <main.h>


// uint32_t state = 0;
// uint32_t battery = 0;
// uint8_t sleep_hour = 19;
// uint8_t wakeup_hour = 7;
// uint32_t error_code = 0;
// uint32_t battery_threshold = -1;
// bool request_sigfox_time = false;
// bool request_sigfox_data = false;
// uint8_t data_sigfox[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
// RTCZero rtc;
// uint8_t read_reg = 0;
// TimeChangeRule Rule_France_summer = {"RHEE", Last, Sun, Mar, 2, 120}; // Règle de passage à l'heure d'été pour la France
// TimeChangeRule Rule_France_winter = {"RHHE", Last, Sun, Oct, 3, 60}; // Règle de passage à l'heure d'hiver la France
// Timezone Convert_to_France(Rule_France_summer, Rule_France_winter); // Objet de conversion d'heure avec les caractéristiques de la métropole française

// void sendI2C(){
//   switch (read_reg)
//   {
//     case REG_SLEEP:
//       Wire.write(sleep_hour);
//       break;

//     case REG_WAKE:
//       Wire.write(wakeup_hour);
//       break;

//     case REG_ERROR:
//       Wire.write(error_code);
//       break;

//     case REG_TIME:
//       {
//       u_int32_t time = rtc.getEpoch();
//       uint8_t data[] = {time >> 24, time >> 16, time >> 8, time};
//       Wire.write(data,(uint8_t)4);
//       }
//       break;

//     case REG_STATE:
//       Wire.write(state);
//       break;

//     case REG_BATTERY:
//       Wire.write(battery);
//       break;

//     default:
//       bitSet(error_code, ERROR_I2C_REG_NOT_FOUND);
//       break;
//   }
// }

// void receiveI2C(int packetSize)
// {
//   if (packetSize == 1) // READ OPERATION
//   {
//     read_reg = Wire.read();
//   }
//   else if (packetSize > 1)  // WRITE OPERATION
//   {
//     uint8_t reg = Wire.read();
//     if(reg == REG_DATA){
//       for(uint8_t i = 0; i<12;i++){
//          data_sigfox[i] = Wire.read();
//       }
//       request_sigfox_data = true;
//     }
//     else{
//       uint32_t val = 0;
//       int8_t i = packetSize - 2;
//       while(i >= 0){
//         val = val | Wire.read() << 8*i;
//         i--;
//       }
//       switch (reg)
//       {
//         case REG_SLEEP:
//           sleep_hour = val;
//           break;

//         case REG_WAKE:
//           wakeup_hour = val;
//           break;

//         case REG_ERROR:
//           error_code = val;
//           break;

//         case REG_STATE:
//           state = val;
//           break;

//         case REG_TIME:
//           if(val == 0){
//             request_sigfox_time = true;
//           }else{
//             setRTCTime(val);
//           }
//           break;

//         case REG_BATTERY_THRESHOLD:
//           battery_threshold = val;
//           break;

//         default:
//           bitSet(error_code, ERROR_I2C_REG_NOT_FOUND);
//           break;
//       }
//     }
//   }
// }


// void sendDataToSigfox(uint8_t data[12]){
//   if(!SigFox.begin()){
//       bitSet(error_code, ERROR_SIGFOX_BEGIN);
//   }
//   delay(100);
//   SigFox.beginPacket();
//   SigFox.write(data);
//   int ret = SigFox.endPacket(); 
//   if (ret > 0) {
//     bitSet(error_code, ERROR_SIGFOX_TRANSMIT);
//   }
//   SigFox.end();
//   bitSet(state, FLAG_SIGFOX_TRANSMITTED);
// }

// uint32_t getTimeFromSigfox(){
//   if(!SigFox.begin()){
//     bitSet(error_code, ERROR_SIGFOX_BEGIN);
//   }
//   delay(100);
//   SigFox.beginPacket();
//   SigFox.write(0);
//   int ret = SigFox.endPacket(true); 
//   if (ret > 0) {
//     bitSet(error_code, ERROR_SIGFOX_TRANSMIT);
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
//   time = (uint32_t) Convert_to_France.toLocal(time);
//   return  time;
// }

// void setRTCTime(uint32_t unix_time){
//   time_t t = unix_time;
//   rtc.setTime(hour(t), minute(t), second(t));
//   rtc.setDate(day(t), month(t), year(t)-2000);
//   bitSet(state, FLAG_TIME_REFRESHED);
// }

// void setAlarmForNextCycle(){
//   SigFox.begin();
//   delay(200);
//   SigFox.end();
//   delay(200);
//   rtc.detachInterrupt();
//   rtc.attachInterrupt(alarmNextCycle);
//   rtc.setAlarmTime(00, (rtc.getMinutes()+CYCLE_TIME)%60, 00);
//   rtc.enableAlarm(rtc.MATCH_MMSS);
//   rtc.standbyMode();
// }

// void setAlarmForNextDay(){
//   SigFox.begin();
//   delay(200);
//   SigFox.end();
//   delay(200);
//   rtc.detachInterrupt();
//   rtc.attachInterrupt(alarmFirstCycle);
//   rtc.setAlarmTime(wakeup_hour, 00, 00);
//   rtc.enableAlarm(rtc.MATCH_HHMMSS);
//   rtc.standbyMode();
// }

// void powerUpRPI(){
//   digitalWrite(PIN_POWER_5V, HIGH); // On active l'alimentation du RPI
//   while(bitRead(state, FLAG_RPI_POWER) == 0){}; // On attends que le RPI s'initialise
// }

// void powerDownRPI(){
//   delay(5000);
//   digitalWrite(PIN_POWER_5V, LOW);
// }

// void alarmFirstCycle(){
//     bitSet(state, FLAG_FIRST_CYCLE);
// }

// void alarmNextCycle(){
//     bitClear(state, FLAG_FIRST_CYCLE);
// }


// // FIRST STARTUP
// void setup()
// {
//   // I2C INIT
//   Wire.begin(MKRFOX_ADDR); // Démarre l'I2C en mode esclave avec l'adresse MKRFOX_ADDR
//   Wire.onReceive(receiveI2C); // On déclare l'interruption pour la réception d'un message I2C
//   Wire.onRequest(sendI2C);  // On déclare l'interruption pour la requete d'une réponse I2C

//   // RTC INIT
//   rtc.begin(false); // Démarre l'horloce RTC

//   // PIN INIT
//   pinMode(PIN_POWER_5V, OUTPUT);
//   pinMode(PIN_BATTERY, INPUT);

//   // REGISTER INIT
//   state = 0; 
//   error_code = 0;
//   battery = 0;
//   sleep_hour = DEFAULT_SLEEP_HOUR;
//   wakeup_hour = DEFAULT_WAKEUP_HOUR;
//   battery_threshold = DEFAULT_BATTERY_THRESHOLD;
//   request_sigfox_time = false;
//   request_sigfox_data = false;
 

//   // RTC INIT
//   rtc.setHours(12); // Lors du 1er démarrage on règle l'heure dans la plage de fonctionnement
// }

// void loop()
// {
//     rtc.disableAlarm(); // On désactive l'alarme
//     if(rtc.getHours() < sleep_hour && rtc.getHours() > wakeup_hour){  // Si on est dans la plage horaire de fonctionnement 
//       battery = analogRead(PIN_BATTERY)*BATTERY_CONSTANT; // On mesure la tension de la batterie pour vérifier l'état de charge
//       if(battery > battery_threshold){ // Si la tension est suffisament grande (batterie suffisament chargée)
//         powerUpRPI(); // On allume le RPI et on attend son message nous indiquant qu'il est prêt à fonctionner
//         while(bitRead(state, FLAG_RPI_POWER) == 1){ // Tant que le RPI n'a pas terminé, on continue à répondre aux commandes I2C
//             if(request_sigfox_time){ // Si le RPI nous demande d'obtenir l'heure par le module SigFox
//               setRTCTime(getTimeFromSigfox()); // On récupere l'heure par SigFox et met à jour le registre et le module RTC
//               request_sigfox_time = false; // On clear la demande une fois qu'elle a été satisfaite
//             }
//             if(request_sigfox_data){ // Si le RPI nous demande d'envoyer les données par SigFox
//               sendDataToSigfox(data_sigfox); // On envoie les données par SigFox
//               request_sigfox_data = false; // On clear la demande une fois qu'elle a été satisfaite
//             }
//         };
//         powerDownRPI(); // On éteints le RPI
//       }
//       setAlarmForNextCycle(); // On prépare le prochain révéil pour le prochain cycle
//     }else{
//       setAlarmForNextDay(); // On prépare le prochain révéil pour le jour suivant
//     }

// }