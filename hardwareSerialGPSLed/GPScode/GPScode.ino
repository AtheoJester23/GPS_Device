#define TINY_GSM_MODEM_SIM800

#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>
#include <ArduinoHttpClient.h>
#include <TinyGsmClient.h>

#define DEVICE_ID "12345678901"
#define modemBAUD 4800
#define gpsBAUD 9600

SoftwareSerial SerialAT(4, 2);
TinyGsm modem(SerialAT);
TinyGsmClientSecure client(modem);

const char apn[]  = "smartlte";
const char user[] = "";
const char pass[] = "";

const char server[] = "testing-26d04-default-rtdb.firebaseio.com";
const int  port = 443;
const String UPDATE_PATH = "ArduinoDeviceId/"+ String(DEVICE_ID);

HttpClient https(client, server, port);

String fireData="";

TinyGPSPlus gps;

int redPin = 12;  
int greenPin = 11; 
int bluePin = 10;  

void setup() {
  pinMode(A2, OUTPUT);
  pinMode(A4, OUTPUT);

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  
  digitalWrite(redPin, 0);
  digitalWrite(greenPin, 0);
  digitalWrite(bluePin, 0);  

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
  digitalWrite(A2, HIGH);
  SerialAT.begin(4800);
  delay(3000);
  Serial.println(F("Initializing modem..."));
  Led(bluePin, 5);
  while (!modem.restart()) {
    Serial.println(F("Failed to restart modem, trying again in 10 seconds..."));
    delay(10000);
  }
  Serial.println("Modem successfully restarted");
  
  String modemInfo = modem.getModemInfo();
  Serial.print(F("Modem: "));
  Serial.println(modemInfo);
  Serial.println(F("wait 10 seconds"));
  delay(10000);
}

void connectInternet(){
  Serial.print(F("Connecting to apn: "));
  Serial.println(apn);
  Led(bluePin, 5);
  while (!modem.gprsConnect(apn, user, pass)) {
    Led(bluePin, 5);
    Serial.println(F("GPRS connection failed, retrying in 10 seconds..."));
    delay(10000);
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
    int batteryLevel = modem.getBattPercent();
    Serial.print(F("Battery Level: "));
    Serial.print(batteryLevel);
    Serial.println(F("%"));

    String date = "";        
    String hourMinute = "";

    if (modem.testAT()) {
      Serial.println("SIM800L connected successfully!");
      
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

        Serial.print(F("Date: "));
        Serial.println(date); 

        Serial.print(F("Time: "));
        Serial.println(hourMinute);

        if(batteryLevel <= 54){
          digitalWrite(greenPin, 0);
          digitalWrite(A2, LOW);
          digitalWrite(A4, HIGH);
          Led(redPin, 5);
        }else{
          digitalWrite(redPin, 0);
          digitalWrite(A4, LOW);
          digitalWrite(A2, HIGH);
        }
        delay(1000);

        Serial.println(F("Umabot na dito..."));

        fireData="";

        int adjustedHour = gps.time.hour() + 8;
        if (adjustedHour >= 24) {
            adjustedHour -= 24; 
        }

        fireData += "{";
        fireData += " \"latitude\" : " + String(gps.location.lat(), 6) + ","; 
        fireData += " \"longitude\" : " + String(gps.location.lng(), 6) + ",";
        fireData += " \"date\" : \"" + date + "\",";
        fireData += " \"time\" : \"" + hourMinute + "\",";
        fireData += " \"battery\" : " + String(batteryLevel);
        fireData += "}";
      }
    } else {
      Serial.println(F("Failed to communicate with SIM800L"));
    }
}

void sendData(const String & path , const String & data, HttpClient* http) {
  Led(greenPin, 5);

  http->connectionKeepAlive();
  String url;
  if (path[0] != '/') {
    url = "/";
  }
  url += path + ".json";
  url += "?print=silent";
  

  String contentType = "application/json";

  http->patch(url, contentType, data);

  int statusCode = http->responseStatusCode();
  http->responseBody();
  Serial.print(F("Status code: "));
  Serial.println(statusCode);
  
  if (!http->connected()) {
    Serial.println();
    http->stop();
    Serial.println(F("HTTP POST disconnected"));
    SerialAT.begin(modemBAUD);
    connectInternet();
    fireData="";
    Serial.begin(9600);
  }
}

void updateData(){
  Led(greenPin, 5);
  sendData(UPDATE_PATH, fireData, &https);
}

void displayInfo(){
  if(gps.location.isValid()){
    data();
  }else{
    data();
  }

  SerialAT.begin(modemBAUD);
  updateData();
  delay(9000);
  fireData="";
}

void Led(int pin, int num) {
  for (int i = 0; i < num; i++) {
    digitalWrite(pin, HIGH);  
    delay(500);               
    digitalWrite(pin, LOW);   
    delay(500);               
  }
}