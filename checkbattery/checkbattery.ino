#define TINY_GSM_MODEM_SIM800

#include <TinyGsmClient.h>
#include <SoftwareSerial.h>
#include <AltSoftSerial.h>

// Configure TinyGSM for your setup
SoftwareSerial sim800(4, 2); // RX = 2, TX = 4 for SIM800L
TinyGsm modem(sim800);

void setup() {
  Serial.begin(4800);
  delay(10);
  
  // Start communication with SIM800L
  sim800.begin(4800);
  // modem.restart();

  // Get battery percentage
  int batteryLevel = modem.getBattPercent();
  Serial.print("Battery Level: ");
  Serial.print(batteryLevel);
  Serial.println("%");
  delay(1000);

  Serial.print("Free memory: ");
  Serial.print(availableMemory());
  Serial.println(" bytes");
}

void loop() {
  // Do other stuff
}

// Function to check available free memory
int availableMemory() {
  int size = 2048;  // Maximum SRAM size for ATmega328p is 2048 bytes
  byte *buf;
  while ((buf = (byte *) malloc(--size)) == NULL);  // Decrease size until malloc is successful
  free(buf);  // Free allocated memory
  return size;
}