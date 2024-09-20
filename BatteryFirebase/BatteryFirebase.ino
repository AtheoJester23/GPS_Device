#include <AltSoftSerial.h>

// Select your modem:
#define TINY_GSM_MODEM_SIM800
#include <TinyGPS++.h>
#include <TinyGsmClient.h>
#include <ArduinoHttpClient.h>
#include <SoftwareSerial.h>

#define SerialMon Serial
#define DEVICE_ID "sim800gps00001"
#define modemBAUD 4800
#define gpsBAUD 9600
SoftwareSerial SerialAT(4, 2); // RX, TX serial modem

// Internet setting
const char apn[]  = "smartlte"; // GSM apn 
const char user[] = "";
const char pass[] = "";

// Firebase setting 
const char server[] = "siml-19451-default-rtdb.firebaseio.com";
const int  port = 443;
const String UPDATE_PATH = "gps_devices/" + String(DEVICE_ID); // firebase root table

// global variables
int count = 0;
String fireData = "";

// LED pins
int redPin = 12;
int greenPin = 11;

TinyGsm modem(SerialAT);
TinyGsmClientSecure client(modem);
HttpClient https(client, server, port);

TinyGPSPlus gps;

unsigned long lastUpdateTime = 0;  // Store the last update time for 15-minute updates
const unsigned long updateInterval = 900000; // 15 minutes in milliseconds (15 * 60 * 1000)

void setup() {
  // Set console baud rate
  SerialMon.begin(9600);
  delay(10);

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  analogWrite(redPin, 255);
  initializeModem(); // check modem communication
  tesKoneksi(); // test internet connection

  int batteryLevel = modem.getBattPercent();
  SerialMon.println(batteryLevel);
  
  Serial.begin(gpsBAUD); // Begin hardware serial for GPS
}

void loop() {

  if (Serial.available() > 0) {  // Check if GPS data is available on hardware serial
    while (fireData.equals("")) {
      scan();
    }
    analogWrite(greenPin, 255);
  }
  else {
    fireData = "";
    Serial.begin(gpsBAUD); // Ensure GPS hardware serial is active
    analogWrite(greenPin, 0);
  } 
}

// Send data to Firebase
void sendData(const String & path , const String & data, HttpClient* http) {

  http->connectionKeepAlive(); // Currently, this is needed for HTTPS
  String url;
  if (path[0] != '/') {
    url = "/";
  }
  url += path + ".json";
  url += "?print=silent";
  
  String contentType = "application/json";

  http->patch(url, contentType, data);

  // read the status code and body of the response
  int statusCode = http->responseStatusCode();
  http->responseBody();
  Serial.print("Status code: ");
  Serial.println(statusCode);
  
  if (!http->connected()) {
    Serial.println();
    http->stop(); // Shutdown
    SerialAT.begin(modemBAUD);
    tesKoneksi();
    fireData = "";
    Serial.begin(gpsBAUD); // Restart GPS serial if disconnected
  }
}

// Scanning GPS location
void scan() {
  while (Serial.available() > 0){  // Reading from hardware serial

  // Read bytes from the GPS module
  gps.encode(Serial.read());

  SerialMon.println("The Code Got Here...");
  delay(5000);
    if (gps.location.isUpdated()){
      displayInfo(); 
    }
  }
}

void displayInfo() {
  if (gps.location.isValid()) {
    data();
  }
  else {
    data();
  }
  SerialAT.begin(modemBAUD);
  updateData();   
}

// Connect to internet 
void tesKoneksi() {
  SerialMon.print(F("Connecting to apn: "));
  SerialMon.println(apn);
  
  while (!modem.gprsConnect(apn, user, pass)) {
    SerialMon.println("GPRS connection failed, retrying in 10 seconds...");
    delay(10000);  // Wait before retrying
  }
  SerialMon.println("GPRS connected");
}

// Update data to Firebase
void updateData() {
  sendData(UPDATE_PATH, fireData, &https);
}

// Initialize modem SIM800
void initializeModem() {
  SerialAT.begin(modemBAUD);
  delay(3000);
  SerialMon.println(F("Initializing modem..."));

  while (!modem.restart()) {
    Serial.println("Failed to restart modem, trying again in 10 seconds...");
    delay(10000);
  }
  Serial.println("Modem successfully restarted");

  String modemInfo = modem.getModemInfo();
  SerialMon.print(F("Modem: "));
  SerialMon.println(modemInfo);
  Serial.println("wait 10 seconds");
  delay(10000);
}

// Get GPS data
void data() {
    fireData = "";

    // Adjust the time by adding 4 hours (or adjust to your local offset)
    int adjustedHour = gps.time.hour() + 8;
    if (adjustedHour >= 24) {
        adjustedHour -= 24;  // Wrap around if the hour exceeds 23
    }

    fireData += "{";
    fireData += " \"latitude\" : " + String(gps.location.lat(), 6) + ","; 
    fireData += " \"longitude\" : " + String(gps.location.lng(), 6) + ",";
    fireData += " \"date\" : \"" + String(gps.date.day()) + "/" +
                              String(gps.date.month()) + "/" +
                              String(gps.date.year()) + "\",";
    fireData += " \"time\" : \"" + String(adjustedHour) + ":" +
                                  String(gps.time.minute()) + ":" +
                                  String(gps.time.second()) + "\"";
    fireData += "}";
}
