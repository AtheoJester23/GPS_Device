String Arsp, Grsp;

void setup() {
  // Initialize hardware serial for SIM800L
  Serial.begin(4800); // SIM800L baud rate
  Serial.println("Live AT commands:");
}

void loop() {
  // Check if there's data from the SIM800L
  if (Serial.available()) {
    Grsp = Serial.readString();
    Serial.println(Grsp); // Output from SIM800L to Serial Monitor
  }

  // Check if there's input from the Serial Monitor to send to SIM800L
  if (Serial.available()) {
    Arsp = Serial.readString();
    delay(2000); // Adjust delay as needed
    Serial.println(Arsp); // Send the input from Serial Monitor to SIM800L
  }
}
