#define TINY_GSM_MODEM_SIM800

#include <TinyGsmClient.h> //https://github.com/vshymanskyy/TinyGSM
#include <ArduinoHttpClient.h> //https://github.com/arduino-libraries/ArduinoHttpClient
#include <TinyGPSPlus.h> // GPS library
#include <EEPROM.h>
#include <SoftwareSerial.h>

// Firebase settings
const char FIREBASE_HOST[]  = "siml-19451-default-rtdb.firebaseio.com";
const String FIREBASE_AUTH  = "G3ehgtFyaYSN3LJGpcGIe5OrWxLM6mJ3lKEGe3Ir";
const String FIREBASE_PATH  = "/loc";  // Update this path to where you want to post the data
const int SSL_PORT          = 443;

// GSM settings
char apn[]  = "smartlte";  // type your APN here
char user[] = "";
char pass[] = "";

// GSM and GPS pins
SoftwareSerial sim800(4, 2);  // RX, TX for SIM800
SoftwareSerial gpsSerial(8, 6);  // RX, TX for GPS

TinyGsm modem(sim800);
TinyGPSPlus gps;  // GPS object
TinyGsmClientSecure gsm_client_secure_modem(modem, 0);
HttpClient http_client = HttpClient(gsm_client_secure_modem, FIREBASE_HOST, SSL_PORT);

void setup()
{
  Serial.begin(9600);
  Serial.println("Arduino serial initialized");

  sim800.begin(9600);
  Serial.println("SIM800L serial initialized");

  gpsSerial.begin(9600);  // Initialize GPS
  Serial.println("NeoGPS serial initialized");

  Serial.println("GPS Logger Started");

  delay(3000);

  // Restart the modem 
  while (!modem.restart()) {
    Serial.println("Failed to restart modem, trying again in 10 seconds...");
    delay(10000);
  }
  Serial.println("Modem successfully restarted");

  // Set up GPRS
  while (!modem.gprsConnect(apn, user, pass)) {
    Serial.println("GPRS connection failed, retrying in 10 seconds...");
    delay(10000);  // Wait before retrying
  }
  Serial.println("GPRS connected");

  while (!http_client.connect(FIREBASE_HOST, SSL_PORT)) {
    Serial.println("Connection to Firebase failed, retrying in 10 seconds...");
    delay(10000);  // Wait before retrying
  }
  Serial.println("Connected to Firebase");

  String modemInfo = modem.getModemInfo();
  Serial.print("Modem: ");
  Serial.println(modemInfo);
  http_client.setHttpResponseTimeout(10 * 1000); // 10 secs timeout
  
}

void loop()
{
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

      // Post to Firebase
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

  String url;
  if (path[0] != '/')
  {
    url = "/";
  }
  url += path + ".json";
  url += "?auth=" + FIREBASE_AUTH;

  http->patch(url, "application/json", data);  // Use PATCH request for updating

  statusCode = http->responseStatusCode();  // Get status code for verification
  Serial.print("Status code: ");
  Serial.println(statusCode);

  response = http->responseBody();
  Serial.print("Response: ");
  Serial.println(response);

  if (!http->connected()) {
    Serial.println();
    http->stop();  // Shutdown
    Serial.println("HTTP POST disconnected");
  }
}