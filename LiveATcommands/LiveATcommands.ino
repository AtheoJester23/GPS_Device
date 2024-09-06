#include <SoftwareSerial.h>

String Arsp, Grsp;
SoftwareSerial gsm(4, 2); // RX, TX

void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);
  Serial.println("Live AT commands:");
  gsm.begin(9600);

}

void loop() {
  // put your main code here, to run repeatedly:

  if(gsm.available())
  {
    Grsp = gsm.readString();
    Serial.println(Grsp);
  }

  if(Serial.available())
  {
    Arsp = Serial.readString();
    delay(2000);
    gsm.println(Arsp);
  }

}