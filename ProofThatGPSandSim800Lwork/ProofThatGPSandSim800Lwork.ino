#define TINY_GSM_MODEM_SIM800

//For GPS:
#include <TinyGPSPlus.h>
#include <EEPROM.h>
#include <AltSoftSerial.h>  // AltSoftSerial for GPS

//For SIM800L:
#include <TinyGsmClient.h>
#include <ArduinoHttpClient.h> // For HTTP requests
#include <SoftwareSerial.h>

// Firebase settings
const char FIREBASE_HOST[] = "siml-19451-default-rtdb.firebaseio.com";
const String FIREBASE_AUTH = "G3ehgtFyaYSN3LJGpcGIe5OrWxLM6mJ3lKEGe3Ir";
const String FIREBASE_PATH = "/loc";  // Firebase location path
const int SSL_PORT = 443;

// GSM settings
char apn[] = "smartlte";  // APN for your GSM
char user[] = "";
char pass[] = "";

// GSM and GPS pins
SoftwareSerial sim800(4, 2); // RX, TX for SIM800
AltSoftSerial gpsSerial;     // AltSoftSerial for GPS (Uses Pin 8 RX and Pin 9 TX by default)

// Initialize GSM and GPS
TinyGsm modem(sim800);
TinyGPSPlus gps;  // GPS object

// Initialize secure GSM client and HTTP client for Firebase
TinyGsmClientSecure gsm_client_secure_modem(modem, 0);
HttpClient http_client = HttpClient(gsm_client_secure_modem, FIREBASE_HOST, SSL_PORT);

void setup() {
  Serial.begin(9600);
  sim800.begin(9600);   // Initialize SIM800L
  gpsSerial.begin(9600); // Initialize AltSoftSerial for GPS
  delay(3000);

  // Initialize the modem
  Serial.println("Initializing modem...");
  while (!modem.restart()) {
    Serial.println("Failed to restart modem, trying again in 10 seconds...");
    delay(10000);
  }
  Serial.println("Modem successfully restarted");

  // Set up GPRS
  modem.gprsConnect(apn, user, pass);

  Serial.println("GPRS connected");

  // Attempt to connect to Firebase host over SSL
  while (!http_client.connect(FIREBASE_HOST, SSL_PORT)) {
    Serial.println("Connection to Firebase failed, retrying in 10 seconds...");
    delay(10000);  // Wait before retrying
  }
  Serial.println("Connected to firebase");

  // Get modem info
  String modemInfo = modem.getModemInfo();
  Serial.print("Modem: ");
  Serial.println(modemInfo);

  http_client.setHttpResponseTimeout(10 * 1000); // 10 seconds timeout for HTTP requests

  // GPS Logger Started
  Serial.println("GPS Logger Started");

  // Read last saved GPS location from EEPROM
  float lastLatitude = readFloatFromEEPROM(0);
  float lastLongitude = readFloatFromEEPROM(4);

  // Print last saved location
  Serial.print("Last saved Latitude: "); Serial.println(lastLatitude, 6);
  Serial.print("Last saved Longitude: "); Serial.println(lastLongitude, 6);
}

void loop() {

  // Check for available GPS data
  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());

    // If GPS location is updated, send data to Firebase
    if (gps.location.isUpdated()) {
      float latitude = gps.location.lat();
      float longitude = gps.location.lng();

      // Print current GPS data
      Serial.print("Latitude= "); Serial.print(latitude, 6);
      Serial.print(" Longitude= "); Serial.println(longitude, 6);

      // Save current GPS data to EEPROM
      saveFloatToEEPROM(0, latitude);
      saveFloatToEEPROM(4, longitude);

      // Prepare JSON data for Firebase
      String jsonData = "{\"lat\":\"" + String(latitude, 6) + "\",\"lng\":\"" + String(longitude, 6) + "\"}";
      PostFirebase("/loc", jsonData, &http_client);  // Send the updated location to Firebase

      delay(1000);  // Delay for demonstration
    }
  }
}

void PostFirebase(const String &path, const String &data, HttpClient* http) {
  String response;
  int statusCode = 0;
  http->connectionKeepAlive(); // Currently, this is needed for HTTPS

  String url = path + ".json";
  url += "?auth=" + FIREBASE_AUTH;

  // Send PATCH request to Firebase
  http->patch(url, "application/json", data);

  // Get status code for verification
  statusCode = http->responseStatusCode();
  Serial.print("Status code: ");
  Serial.println(statusCode);

  // Get response
  response = http->responseBody();
  Serial.print("Response: ");
  Serial.println(response);

  // If HTTP disconnected, stop
  if (!http->connected()) {
    Serial.println();
    http->stop();  // Shutdown
    Serial.println("HTTP POST disconnected");
  }
}

// Function to save float value to EEPROM
void saveFloatToEEPROM(int address, float value) {
  byte* p = (byte*)(void*)&value;
  for (int i = 0; i < sizeof(value); i++) {
    EEPROM.write(address + i, *(p + i));
  }
}

// Function to read float value from EEPROM
float readFloatFromEEPROM(int address) {
  float value;
  byte* p = (byte*)(void*)&value;
  for (int i = 0; i < sizeof(value); i++) {
    *(p + i) = EEPROM.read(address + i);
  }
  return value;
}
