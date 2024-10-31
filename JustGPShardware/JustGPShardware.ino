#include <TinyGPS++.h>  // Include TinyGPS++ library

TinyGPSPlus gps;  // Create a GPS object

void setup() {
  Serial.begin(9600);  // Start the hardware serial for GPS communication
  
  // Optional: small delay to allow the GPS module to initialize
  delay(1000);
  Serial.println("GPS Module Initializing...");
}

void loop() {
  // Continuously read data from the GPS module
  while (Serial.available() > 0) {
    gps.encode(Serial.read());  // Feed the GPS data into the TinyGPS++ library
    
    // If valid GPS data is received, print latitude and longitude
    if (gps.location.isUpdated()) {
      Serial.print("Latitude: ");
      Serial.println(gps.location.lat(), 6);  // Print Latitude with 6 decimal places
      
      Serial.print("Longitude: ");
      Serial.println(gps.location.lng(), 6);  // Print Longitude with 6 decimal places
    }
  }
}
