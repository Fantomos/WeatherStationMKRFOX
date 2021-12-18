/** @file main.cpp
 * Programme du MKRFOX1200 pour la station météo.
 * 
*/
#include <main.h>


uint8_t state = 0;
uint32_t battery = 0;
uint8_t sleep_hour = DEFAULT_SLEEP_HOUR;
uint8_t wakeup_hour = DEFAULT_WAKEUP_HOUR;
uint8_t error_code = 0;
uint32_t battery_threshold = DEFAULT_BATTERY_THRESHOLD;
bool request_sigfox_time = false;
bool request_sigfox_data = false;
RTCZero rtc;
uint8_t read_reg = 0;
SigfoxMessage msg;
TimeChangeRule Rule_France_summer = {"RHEE", Last, Sun, Mar, 2, 120}; // Règle de passage à l'heure d'été pour la France
TimeChangeRule Rule_France_winter = {"RHHE", Last, Sun, Oct, 3, 60}; // Règle de passage à l'heure d'hiver la France
Timezone Convert_to_France(Rule_France_summer, Rule_France_winter); // Objet de conversion d'heure avec les caractéristiques de la métropole française

/**
 * \brief Interruption I2C lors de l'envoi de données.
 */
void sendI2C(){
  switch (read_reg)
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
      {
      u_int32_t time = rtc.getEpoch();
      uint8_t time_array[] = {(time >> 24) & 0xFF, (time >> 16) & 0xFF, (time >> 8) & 0xFF, time & 0xFF};
      Wire.write(time_array,4);
      }
      break;

    case REG_STATE:
      Wire.write(state);
      break;

    case REG_BATTERY:
    {
      uint8_t battery_array[] = {(battery >> 24) & 0xFF, (battery >> 16) & 0xFF, (battery >> 8) & 0xFF, battery & 0xFF};
      Wire.write(battery_array, 4);
    }
      break;
    case REG_BATTERY_THRESHOLD:
      {
      uint8_t battery_threshold_array[] = {(battery_threshold >> 24) & 0xFF, (battery_threshold >> 16) & 0xFF, (battery_threshold >> 8) & 0xFF, battery_threshold & 0xFF};
      Wire.write(battery_threshold_array, 4);
      }
      break;

    default:
      bitSet(error_code, ERROR_I2C_REG_NOT_FOUND);
      break;
  }
}

/**
 * \brief Interruption I2C lors de la réception de données.
 * \param packetSize Nombre d'octets reçus.
 */
void receiveI2C(int packetSize)
{
  if (packetSize == 1) // READ OPERATION
  {
    read_reg = Wire.read();
  }
  else if (packetSize > 1)  // WRITE OPERATION
  {
    uint8_t reg = Wire.read();
    if(reg == REG_DATA){
        msg.speed = Wire.read();
        msg.speed_max = Wire.read();
        msg.direction = Wire.read() << 8 | Wire.read();
        msg.direction_max = Wire.read() << 8 | Wire.read();
        msg.humidity = Wire.read();
        msg.temperature = Wire.read();
        msg.pressure = Wire.read() << 8 | Wire.read();
        msg.voltage = Wire.read() << 8 | Wire.read();
        request_sigfox_data = true;
    }
    else{
      uint32_t val = 0;
      int8_t i = packetSize - 2;
      while(i >= 0){
        val = val | Wire.read() << 8*i;
        i--;
      }
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
            request_sigfox_time = true;
          }else{
            setRTCTime(val);
          }
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
}

/**
 * \brief Transmet les données au réseau Sigfox.
 */
void sendDataToSigfox(){
  if(!SigFox.begin()){
      bitSet(error_code, ERROR_SIGFOX_BEGIN);
  }
  delay(100);
  SigFox.status();
  delay(1);
  SigFox.beginPacket();
  SigFox.write((uint8_t*)&msg,12);
  int ret = SigFox.endPacket(); 
  if(ret > 0) {
    bitSet(error_code, ERROR_SIGFOX_TRANSMIT);
  }
  SigFox.end();
}

/**
 * \brief Obtiens l'heure actuelle à partir du réseau Sigfox.
 * \return Heure actuelle au format UNIX.
 */
uint32_t getTimeFromSigfox(){
  if(!SigFox.begin()){
    bitSet(error_code, ERROR_SIGFOX_BEGIN);
  }
  delay(100);
  SigFox.status();
  delay(1);
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
  uint32_t time = (uint32_t) (time_buf[0] << 24 | time_buf[1] << 16 | time_buf[2] << 8 | time_buf[3]);
// time = (uint32_t) Convert_to_France.toLocal(time);
  return  time;
}

/**
 * \brief Met à jour l'horloge RTC du microcontrolleur.
 * \param unix_time Heure actuelle au format UNIX.
 */
void setRTCTime(uint32_t unix_time){
  time_t t = unix_time;
  rtc.setEpoch(t);
  bitSet(state, FLAG_TIME_REFRESHED);
}

/**
 * \brief Initialise le bus I2C en mode esclave et enregistre les fonctions d'interruption.
 */
void initI2C(){
    Wire.begin(MKRFOX_ADDR); // Démarre l'I2C en mode esclave avec l'adresse MKRFOX_ADDR
    Wire.onReceive(receiveI2C); // On déclare l'interruption pour la réception d'un message I2C
    Wire.onRequest(sendI2C);  // On déclare l'interruption pour la requete d'une réponse I2C
}

/**
 * \brief Désactive le bus I2C et configure les pin en sortie à l'état bas pour éviter d'alimenter les capteurs.
 */
void endI2C(){
    Wire.end();
    pinMode(12, INPUT);
    pinMode(11, INPUT);
}

/**
 * \brief Active l'alarme pour le cycle suivant dans les x minutes programmées.
 */
void setAlarmForNextCycle(){
  // On désactive la ligne I2C
  endI2C();

  // On s'assure que le module Sigfox est bien désactivé avant de rentrer en sommeil (bug de la librairie Sigfox)
  SigFox.begin();
  delay(200);
  SigFox.debug();
  SigFox.end();
  delay(200); 
  rtc.setAlarmTime(00, (rtc.getMinutes()+CYCLE_TIME)%60, 00);
  rtc.enableAlarm(rtc.MATCH_MMSS);
  rtc.standbyMode();
}

/**
 * \brief Active l'alimentation du Raspberry Pi et attends la fin de son démaragge.
 */
void powerUpRPI(){
  digitalWrite(PIN_POWER_5V, HIGH); // On active l'alimentation du RPI
  uint32_t timeBefore = millis();
  while(bitRead(state, FLAG_RPI_POWER) == 0 && millis() - timeBefore < TIMEOUT_WAKEUP){}; // On attends que le RPI s'initialise 
}

/**
 * \brief Désactive l'alimentation du Raspberry Pi.
 */
void powerDownRPI(){
   delay(TIMEOUT_SHUTDOWN);
  digitalWrite(PIN_POWER_5V, LOW);
}

/**
 * \brief Initialisation du microcontrolleur à son premier démarrage (lors de l'installation de la station).
 */
void setup()
{
  // SigFox INIT
  SigFox.begin();
  delay(200);
  SigFox.debug();
  SigFox.end();
  delay(200);

  // RTC INIT
  rtc.begin(false); // Démarre l'horloge RTC

  // PIN INIT
  pinMode(PIN_POWER_5V, OUTPUT);
  pinMode(PIN_POWER_ATTINY, OUTPUT);
  pinMode(PIN_BATTERY, INPUT);

  // REGISTER INIT
  state = 0; 
  error_code = 0;
  battery = 0;
  sleep_hour = DEFAULT_SLEEP_HOUR;
  wakeup_hour = DEFAULT_WAKEUP_HOUR;
  battery_threshold = DEFAULT_BATTERY_THRESHOLD;
  request_sigfox_time = false;
  request_sigfox_data = false;
 if(SHUTDOWN_RPI){
    bitSet(state, FLAG_SHUTDOWN_RPI);
  }
  else{
    bitClear(state, FLAG_SHUTDOWN_RPI);
  }

  // Lors du 1er démarrage on règle l'heure dans la plage de fonctionnement, on alimente l'ATTINY et on active le flag du premier cycle
  rtc.setHours(12); 
  digitalWrite(PIN_POWER_ATTINY, HIGH);
  bitSet(state, FLAG_FIRST_CYCLE);
}

/**
 * \brief Boucle exécutant un cycle à chaque réveil.
 */
void loop()
{
    rtc.disableAlarm(); // On désactive l'alarme
    initI2C(); // On initialise l'I2C
    if(rtc.getHours() < sleep_hour && rtc.getHours() > wakeup_hour){  // Si on est dans la plage horaire de fonctionnement 
      digitalWrite(PIN_POWER_ATTINY, HIGH); // On alimente l'ATTINY
      battery = analogRead(PIN_BATTERY)*BATTERY_CONSTANT; // On mesure la tension de la batterie pour vérifier l'état de charge
      if(battery > battery_threshold){ // Si la tension est suffisament grande (batterie suffisament chargée)
        powerUpRPI(); // On allume le RPI et on attend son message nous indiquant qu'il est prêt à fonctionner
        uint32_t timeBefore = millis();
        while(bitRead(state, FLAG_RPI_POWER) == 1 && millis() - timeBefore < TIMEOUT_CYCLE){ // Tant que le RPI n'a pas terminé, on continue à répondre aux commandes I2C
            if(request_sigfox_time){ // Si le RPI nous demande d'obtenir l'heure par le module SigFox
              setRTCTime(getTimeFromSigfox()); // On récupere l'heure par SigFox et met à jour le registre et le module RTC
              request_sigfox_time = false; // On clear la demande une fois qu'elle a été satisfaite
            }
            if(request_sigfox_data){ // Si le RPI nous demande d'envoyer les données par SigFox
              sendDataToSigfox(); // On envoie les données par SigFox
              request_sigfox_data = false; // On clear la demande une fois qu'elle a été satisfaite
            }
        };
        if(SHUTDOWN_RPI){ // Si il faut éteindre le RPI
          powerDownRPI(); // On éteints le RPI
        }
      }
      bitClear(state, FLAG_FIRST_CYCLE);
      setAlarmForNextCycle(); // On prépare le prochain révéil pour le prochain cycle
    }else{  // On prépare le prochain révéil pour le jour suivant
      digitalWrite(PIN_POWER_ATTINY, LOW); // On éteint l'ATTINY
      bitSet(state, FLAG_FIRST_CYCLE);  // On active le flag du premier cycle
      setAlarmForNextCycle();
    }
}