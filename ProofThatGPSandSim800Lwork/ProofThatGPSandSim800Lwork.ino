#define TINY_GSM_MODEM_SIM800

#include <TinyGsmClient.h>
#include <TinyGPSPlus.h>
#include <EEPROM.h>
#include <AltSoftSerial.h>

#include <SoftwareSerial.h>
SoftwareSerial sim800(4, 2);

#define TINY_GSM_SERIAL sim800
TinyGsm modem(sim800);



TinyGPSPlus gps;
AltSoftSerial gpsSerial;  // RX is pin 8, TX is pin 9

void setup() {
  Serial.begin(9600);
  delay(1000);

  sim800.begin(9600);
  delay(3000);

  gpsSerial.begin(9600);
  delay(1000);

  // Initialize the modem
  Serial.println("Initializing modem...");

  // Restart the modem 
  while (!modem.restart()) {
    Serial.println("Failed to restart modem, trying again in 10 seconds...");
    delay(10000);
  }
  Serial.println("Modem successfully restarted");


  Serial.println("GPS Logger Started");

  // Read last saved GPS location from EEPROM
  float lastLatitude = readFloatFromEEPROM(0);
  float lastLongitude = readFloatFromEEPROM(4);

  // Print last saved location
  Serial.print("Last saved Latitude: "); Serial.println(lastLatitude, 6);
  Serial.print("Last saved Longitude: "); Serial.println(lastLongitude, 6);
}

void loop() {
  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());

    if (gps.location.isUpdated()) {
      float latitude = gps.location.lat();
      float longitude = gps.location.lng();
      
      // Print current GPS data
      Serial.print("Latitude= "); Serial.print(latitude, 6);
      Serial.print(" Longitude= "); Serial.println(longitude, 6);

      // Save current GPS data to EEPROM
      saveFloatToEEPROM(0, latitude);
      saveFloatToEEPROM(4, longitude);

      delay(1000);  // Delay for demonstration
    }
  }
}

void saveFloatToEEPROM(int address, float value) {
  byte* p = (byte*)(void*)&value;
  for (int i = 0; i < sizeof(value); i++) {
    EEPROM.write(address + i, *(p + i));
  }
}

float readFloatFromEEPROM(int address) {
  float value;
  byte* p = (byte*)(void*)&value;
  for (int i = 0; i < sizeof(value); i++) {
    *(p + i) = EEPROM.read(address + i);
  }
  return value;
}
