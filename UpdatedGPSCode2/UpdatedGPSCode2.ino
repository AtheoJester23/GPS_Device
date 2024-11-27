#define TINY_GSM_MODEM_SIM800

#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>
#include <ArduinoHttpClient.h>
#include <TinyGsmClient.h>

#define modemBAUD 4800
#define gpsBAUD 9600

SoftwareSerial SerialAT(4, 2);
TinyGsm modem(SerialAT);
TinyGsmClientSecure client(modem);

const char apn[] = "smartlte";
const char user[] = "";
const char pass[] = "";

const char server[] = "gps-pet-tracker-v1-default-rtdb.firebaseio.com";
const int port = 443;
const String UPDATE_PATH = "/ArduinoDeviceId/12345678902.json";

HttpClient https(client, server, port);

String fireData = "";

TinyGPSPlus gps;

int redPin = 11;
int bluePin = 10;
const int redPin2 = 7;
const int greenPin2 = 6;

float lat = 4;
float lng = 4;

float prevLat = 0;
float prevLng = 0;

int batteryLevel = 0;
int notUnder = 0;

bool accessed = false;

unsigned long lastBatteryUpdate = 0;
const unsigned long batteryUpdateInterval = 900000;  // 15 minutes in milliseconds

void setup() {
  pinMode(greenPin2, OUTPUT);
  pinMode(redPin2, OUTPUT);

  pinMode(redPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  analogWrite(redPin, 0);
  analogWrite(bluePin, 0);

  Serial.begin(9600);
  delay(10);

  initializeModem();

  connectInternet();
}

void loop() {
  while (Serial.available() > 0) {
    if (gps.encode(Serial.read())) {
      if (gps.location.isValid()) {
        lng = gps.location.lng();
        lat = gps.location.lat();

        if (lat != prevLat && lng != prevLng) {
          sendData(UPDATE_PATH, fireData, &https);
        } else {
          return;
        }
      }
      return;
    }
  }
}

void initializeModem() {
  analogWrite(greenPin2, 5);

  SerialAT.begin(4800);
  
  Serial.println(F("Initializing modem..."));
  Led(bluePin, 3);
  while (!modem.restart()) {
    Serial.println(F("Failed to restart modem, trying again in 10 seconds..."));
    delay(5000);
  }
  Serial.println(F("Modem successfully restarted"));

  batteryLevel = modem.getBattPercent();

  if (batteryLevel <= 15) {
    analogWrite(greenPin2, 0);
    analogWrite(redPin2, 128);
  } else {
    analogWrite(redPin2, 0);
    analogWrite(greenPin2, 5);
  }
}

void connectInternet() {
  Led(bluePin, 3);
  while (!modem.gprsConnect(apn, user, pass)) {
    Led(bluePin, 5);
    Serial.println(F("GPRS connection failed, retrying in 10 seconds..."));
    delay(5000);
  }
  Serial.println(F("GPRS connected"));

  return;
}

void prepData() {
  if (millis() - lastBatteryUpdate >= batteryUpdateInterval) {
    batteryLevel = modem.getBattPercent();
    lastBatteryUpdate = millis();  // Reset the timer
  }

  String date = "";
  String hourMinute = "";

  if (batteryLevel <= 15) {
    analogWrite(greenPin2, 0);
    analogWrite(redPin2, 128);
  } else {
    analogWrite(redPin2, 0);
    analogWrite(greenPin2, 5);
  }
  delay(200);

  fireData = "";

  int adjustedHour = gps.time.hour() + 8;
  if (adjustedHour >= 24) {
    adjustedHour -= 24;
  }

  int fullYear = gps.date.year();  // e.g., 2024
  int month = gps.date.month();
  int day = gps.date.day();

  int shortYear = fullYear % 100;  // This will give you 24

  String formattedMinutes = gps.time.minute() < 10 ? "0" + String(gps.time.minute()) : String(gps.time.minute());

  fireData += "{";
  if (batteryLevel != 0 && batteryLevel >= notUnder) {
    fireData += " \"battery\" : " + String(batteryLevel) + ",";
  }
  if (lat > 3 && lng > 3) {
    fireData += " \"latitude\" : " + String(lat, 6) + ",";
    fireData += " \"longitude\" : " + String(lng, 6) + ",";
  }
  if(shortYear >= 24 && month != 0 && day !=0){
    fireData += " \"date\" : \"" + String(shortYear) + "/" + String(month) + "/" + String(day) + "\",";
    fireData += " \"time\" : \"" + String(adjustedHour) + ":" + String(formattedMinutes) + "\",";
  }
  fireData += " \"status\" : \"Online\",";
  fireData += " \"arduinoIdd\" : \"12345678902\"";
  fireData += "}";

  return;
}

void sendData(const String& path, const String& data, HttpClient* http) {
  prepData();

  if (!modem.isGprsConnected()) {
    analogWrite(bluePin, 0);
    analogWrite(redPin, 0);
    Serial.println(F("GPRS not connected, reconnecting..."));
    http->stop();
    connectInternet();
  }

  http->connectionKeepAlive();

  if(accessed == false){
    http->get(path);

    String response = http->responseBody();

    if (response == "null") {
      Serial.println(F("firebase(response is null)..."));
      http->stop();

      analogWrite(bluePin, 0);
      analogWrite(redPin, 0);
      delay(200);

      return false;
    }else{
      accessed = true;
      return;
    }
  }

  Serial.println(F("Uploading..."));

  http->beginRequest();

  http->patch(path, "application/json", data);

  http->httpResponseTimeout();

  // http->stop();

  http->endRequest();

  http->flush();

  http->stop();

  prevLat = lat;
  prevLng = lng;

  analogWrite(bluePin, 7);
  analogWrite(redPin, 20);
  delay(200);
  analogWrite(bluePin, 0);
  analogWrite(redPin, 0);
  int prevBatt = batteryLevel;
  notUnder = prevBatt - 1;

  // if (!http->connected()) {
  //   http->endRequest();

  //   http->flush();

  //   Serial.println(F("HTTP POST disconnected"));
  //   return;
  // }

  return;
}

void Led(int pin, int num) {
  for (int i = 0; i < num; i++) {
    analogWrite(pin, 10);
    delay(500);
    analogWrite(pin, 0);
    delay(500);
  }
}