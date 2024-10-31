#include <SoftwareSerial.h>

SoftwareSerial SerialAT(4, 2);  // RX, TX

void setup() {
  Serial.begin(4800);
  SerialAT.begin(4800);  // Initialize SIM800L communication

  // Send the AT command to get battery status
  SerialAT.println("AT+CBC");
  delay(1000);  // Wait for SIM800L to process and respond

  String response = "";

  // Read response line by line
  while (SerialAT.available()) {
    String line = SerialAT.readStringUntil('\n');  // Read each line until newline
    line.trim();                                   // Remove any extra whitespace, carriage return, or newline

    if (line.startsWith("+CBC:")) {
      // Extract battery percentage
      int commaIndex1 = line.indexOf(',');                   // Find first comma
      int commaIndex2 = line.indexOf(',', commaIndex1 + 1);  // Find second comma

      // Get the value between the first and second comma (the battery percentage)
      String batteryPercentage = line.substring(commaIndex1 + 1, commaIndex2);

      Serial.println("Battery Percentage: " + batteryPercentage);  // Print extracted value
    }
  }
}

void loop() {
  // Your main code
}
