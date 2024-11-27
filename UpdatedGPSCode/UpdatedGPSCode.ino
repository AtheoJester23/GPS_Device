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

const char apn[]  = "smartlte"; 
const char user[] = "";
const char pass[] = "";

const char server[] = "gps-pet-tracker-v1-default-rtdb.firebaseio.com";
const int  port = 443;
const String UPDATE_PATH = "/ArduinoDeviceId/12345678901";

HttpClient https(client, server, port);

String fireData="";

TinyGPSPlus gps;
 
int redPin = 11; 
int bluePin = 10;  
const int redPin2 = 7;
const int greenPin2 = 6;

float lat = 1;
float lng = 1;

float prevLat = 0;
float prevLng = 0;

int batteryLevel = 0;
int notUnder = 0;

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
  if(Serial.available()) {
    while(fireData.equals("")){
      scan();
    }
  }
}

void initializeModem(){
  analogWrite(greenPin2, 4);

  SerialAT.begin(4800);
  
  delay(3000);
  Serial.println(F("Initializing modem..."));
  Led(bluePin, 3);
  while (!modem.restart()) {
    Serial.println(F("Failed to restart modem, trying again in 10 seconds..."));
    delay(5000);
  }
  Serial.println(F("Modem successfully restarted"));

  batteryLevel = modem.getBattPercent();

  if(batteryLevel <= 15){
    analogWrite(greenPin2, 0);
    analogWrite(redPin2, 128);
  }else{
    analogWrite(redPin2, 0);
    analogWrite(greenPin2, 4);
  }
}

void connectInternet(){
  SerialAT.println("AT+CNTP=1");
  delay(5000);
  Led(bluePin, 3);
  while (!modem.gprsConnect(apn, user, pass)) {
    Led(bluePin, 5);
    Serial.println(F("GPRS connection failed, retrying in 10 seconds..."));
    delay(5000);
  }
  Serial.println(F("GPRS connected"));
}

void scan(){
  while (Serial.available() > 0){
    if (gps.encode(Serial.read())){
      displayInfo();
    }
  }
}

void data(){
    batteryLevel = modem.getBattPercent();

    String date = "";        
    String hourMinute = "";

    if (modem.testAT()) {      
      modem.sendAT(GF("+CCLK?"));

      modem.stream.readStringUntil('\n');
      String timeResponse = modem.stream.readStringUntil('\n');
    
      if (timeResponse.indexOf("+CCLK:") != -1) {
        int start = timeResponse.indexOf("\"") + 1;
        int end = timeResponse.indexOf("\"", start);
        String dateTime = timeResponse.substring(start, end);
        
        String date = dateTime.substring(0, dateTime.indexOf(","));
        String time = dateTime.substring(dateTime.indexOf(",") + 1);

        String hourMinute = time.substring(0, 5);

        if(batteryLevel <= 15){
          analogWrite(greenPin2, 0);
          analogWrite(redPin2, 128);
        }else{
          analogWrite(redPin2, 0);
          analogWrite(greenPin2, 4);
        }
        delay(500);

        fireData="";

        int adjustedHour = gps.time.hour() + 8;
        if (adjustedHour >= 24) {
            adjustedHour -= 24; 
        }

        fireData += "{";
        if(batteryLevel != 0 || batteryLevel >= notUnder){
          fireData += " \"battery\" : " + String(batteryLevel) + ",";
        }
        fireData += " \"latitude\" : " + String(lat, 6) + ",";
        fireData += " \"longitude\" : " + String(lng, 6) + ",";
        fireData += " \"status\" : \"Online\",";
        fireData += " \"arduinoIdd\" : \"12345678901\",";
        fireData += " \"date\" : \"" + date + "\",";
        fireData += " \"time\" : \"" + hourMinute  + "\"";
        fireData += "}";
      }
    } else {
      Serial.println(F("Failed to communicate with SIM800L"));
    }
}

void sendData(const String & path , const String & data, HttpClient* http) {  
  if (!modem.isGprsConnected()) {
    analogWrite(bluePin, 0);
    analogWrite(redPin, 0);
    delay(500);
    Serial.println(F("GPRS not connected, reconnecting..."));
    http->stop();
    connectInternet();
  }


  http->connectionKeepAlive(); 
  String url;
  if (path[0] != '/') {
    url = "/";
  }
  url += path + ".json";

  String url2;
  url2 += path + "/arduinoId.json";

  http->get(url2);

  int statusCode = http->responseStatusCode();

  if (statusCode > 0) {
    String response = http->responseBody();

    if (response == "null") {
      Serial.println(F("firebase(response is null)..."));
      http->stop();
      return false;
    }
    Serial.println(F("Uploading..."));

    http->patch(url, "application/json", data);

    prevLat = lat;
    prevLng = lng;
  }
  
  if (!http->connected()) {
    http->stop();
    Serial.println(F("HTTP POST disconnected"));
  }

}

void updateData(){
  sendData(UPDATE_PATH, fireData, &https);
}

void displayInfo(){
  if(gps.location.isValid()){
    analogWrite(bluePin, 0);
    analogWrite(redPin, 0);
    delay(500); 

    lng = gps.location.lng();
    lat = gps.location.lat();

    if(lat != prevLat && lng != prevLng){
      data();
      updateData();
    }
  }else{
    Serial.println(F("Wait di pa valid yung GPS data..."));
  }
  analogWrite(bluePin, 12);
  analogWrite(redPin, 7);
  delay(500);
  int prevBatt = modem.getBattPercent();
  notUnder = prevBatt - 2;

  fireData="";
}

void Led(int pin, int num) {
  for (int i = 0; i < num; i++) {
    analogWrite(pin, 10);  
    delay(500);
    analogWrite(pin, 0);               
    delay(500);               
  }
}