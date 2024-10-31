#include <SoftwareSerial.h>

// Define RX and TX pins for SIM800L
SoftwareSerial sim800l(4, 2); // SIM800L TX to pin 4, RX to pin 2

void setup() {
  // Begin serial communication
  Serial.begin(9600);
  sim800l.begin(4800);

  // Wait for the SIM800L module to initialize
  delay(1000);
  
  // Set SMS mode to text mode
  sim800l.println("AT+CMGF=1"); 
  delay(1000);

  // Enable the SIM800L to show incoming messages
  sim800l.println("AT+CNMI=1,2,0,0,0");
  delay(1000);

  // Send the "DATA BAL" message to 8080
  sendSMS("8080", "DATA BAL");

  Serial.println("Ready to receive SMS messages...");
}

void loop() {
  // Check if data is available from SIM800L
  if (sim800l.available()) {
    while (sim800l.available()) {
      // Read the SMS data from SIM800L and print it to the Serial Monitor
      Serial.write(sim800l.read());
    }
  }
}

// Function to send an SMS
void sendSMS(const char* phoneNumber, const char* message) {
  // Set the phone number
  sim800l.print("AT+CMGS=\"");
  sim800l.print(phoneNumber);
  sim800l.println("\"");
  delay(1000);

  // Send the message
  sim800l.print(message);
  delay(1000);

  // End the message with Ctrl+Z (ASCII code 26)
  sim800l.write(26); 
  delay(1000);

  Serial.println("Message sent!");
}
