#define TINY_GSM_MODEM_SIM800

#include <TinyGsmClient.h>
#include <SoftwareSerial.h>

// Define RX and TX pins for SIM800L
SoftwareSerial sim800l(4, 2); // SIM800L TX to pin 4, RX to pin 2

// Set your forwarding phone number
const char forwardNumber[] = "214"; // Replace with your number

TinyGsm modem(sim800l);

void setup() {
  // Begin serial communication
  Serial.begin(4800);
  sim800l.begin(4800);

  // Restart the modem and connect to the network
  modem.restart();

  Serial.println("Initializing modem...");
  if (!modem.waitForNetwork()) {
    Serial.println("Network connection failed");
    while (true);
  }

  // Set SMS mode to text
  modem.sendAT("+CMGF=1"); 
  delay(1000);

  // Enable the SIM800L to display new SMS messages immediately
  modem.sendAT("+CNMI=1,2,0,0,0");
  delay(1000);

  Serial.println("Modem is ready. Waiting for incoming SMS...");
}

void loop() {
  // Check if there's incoming data from the modem
  if (sim800l.available()) {
    String smsMessage = readFullSMS();

    if (smsMessage.length() > 0) {
      Serial.print("Received SMS: ");
      Serial.println(smsMessage);
      
      // Forward the SMS
      forwardSMS(forwardNumber, smsMessage);
    }
  }
}

// Function to read full SMS, handles longer messages
String readFullSMS() {
  String smsContent = "";
  unsigned long timeout = millis() + 5000;  // 5 seconds timeout to receive the full message
  
  // Wait for and read all data available from the SIM800L
  while (millis() < timeout) {
    while (sim800l.available()) {
      smsContent += (char)sim800l.read();
    }
    delay(100);  // Allow buffer to fill with more data
  }

  // Process the SMS if it contains the SMS text marker "+CMT:"
  if (smsContent.indexOf("+CMT:") != -1) {
    int smsStart = smsContent.indexOf("\r\n", smsContent.indexOf("+CMT:")) + 2;
    int smsEnd = smsContent.indexOf("\r\n", smsStart);
    
    // Check if the message is long and requires more chunks
    while (smsEnd == -1) {
      smsEnd = smsContent.indexOf("\r\n", smsStart);
      delay(100); // wait for more data to arrive
    }

    return smsContent.substring(smsStart, smsEnd);
  }

  return "";
}

void forwardSMS(const char* number, const String& message) {
  Serial.print("Forwarding SMS to: ");
  Serial.println(number);
  
  if (modem.sendSMS(number, message)) {
    Serial.println("SMS forwarded successfully!");

    
  } else {
    Serial.println("SMS forward failed.");
  }
}
