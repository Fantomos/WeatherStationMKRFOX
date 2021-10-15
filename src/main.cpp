#include <main.h>

uint32_t state_rpi = 0x00;
uint32_t state_pic = 0x00;
uint32_t batterie = 0;
uint32_t sleep_hour = 2;
uint32_t wakeup_hour = 4;

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

    case REG_EVEIL:
      Wire.write(wakeup_hour);
      break;

    default:
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

      case REG_EVEIL:
        wakeup_hour = val;
        break;

      default:
        break;
    }
  }
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);
  while (!Serial){};

  Wire.begin(MKRFOX_ADDR);
  Wire.onReceive(receiveI2C); // register event

  
  state_rpi = 0; // 0-Eteint 1-Démarré 2-Données compilées 3-Terminé
  state_pic = 0;


  if (!SigFox.begin())
  {
    Serial.println("Shield error or not present!");
    return;
  }
}

void loop()
{


}