#define TINY_GSM_MODEM_SIM800

#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>
#include <ArduinoHttpClient.h>
#include <TinyGsmClient.h>

#define DEVICE_ID "sim800gps00001"
#define modemBAUD 4800
#define gpsBAUD 9600

// Configure TinyGSM for your setup
SoftwareSerial SerialAT(4, 2); // RX = 2, TX = 4 for sim800L
TinyGsm modem(SerialAT);
TinyGsmClientSecure client(modem);

// Internet setting
const char apn[]  = "smartlte"; // GSM apn 
const char user[] = "";
const char pass[] = "";

//Firebase setting 
const char server[] = "siml-19451-default-rtdb.firebaseio.com";
const int  port = 443;
const String UPDATE_PATH = "gps_devices/"+ String(DEVICE_ID); // firebase root table

HttpClient https(client, server, port);

//global variables
String fireData="";

// Create a TinyGPS object
TinyGPSPlus gps;

int redPin = 12;    // Red LED pin
int greenPin = 11;  // Green LED pin
int bluePin = 10;   // Blue LED pin (not used)

void setup() {
  // Set the RGB pins as output
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  
  // Turn off all LEDs initially
  digitalWrite(redPin, 0);
  digitalWrite(greenPin, 0);
  digitalWrite(bluePin, 0);

  // Start hardware serial for communication with the GPS module and serial monitor
  Serial.begin(9600); // Serial monitor
  delay(10);

  // Start communication with sim800L
  initializeModem();

  //Connect to internet
  connectInternet();


}

void loop() {
  if(Serial.available()) {
    digitalWrite(greenPin, 255);  // Turn on green LED
    while(fireData.equals("")){
      scan();
    }
  }
}

//initialize modem sim800
void initializeModem(){
  SerialAT.begin(4800);
  delay(3000);
  Serial.println(F("Initializing modem..."));
  digitalWrite(bluePin, 255);  // Turn on green LED
  while (!modem.restart()) {
    Serial.println("Failed to restart modem, trying again in 10 seconds...");
    delay(10000);
  }
  Serial.println("Modem successfully restarted");
  
  String modemInfo = modem.getModemInfo();
  Serial.print(F("Modem: "));
  Serial.println(modemInfo);
  Serial.println("wait 10 seconds");
  delay(10000);
}

// connect to internet 
void connectInternet(){
  Serial.print(F("Connecting to apn: "));
  Serial.println(apn);
  digitalWrite(bluePin, 255);  // Turn on green LED
  while (!modem.gprsConnect(apn, user, pass)) {
    Serial.println("GPRS connection failed, retrying in 10 seconds...");
    delay(10000);  // Wait before retrying
  }
  Serial.println("GPRS connected");
  digitalWrite(bluePin, 0);
  delay(1000);
  digitalWrite(greenPin, 1);
}

// scanning gps location
void scan(){
  while (Serial.available() > 0){
    digitalWrite(greenPin, 255);
    if (gps.encode(Serial.read())){
      displayInfo();
    }
  }
}

//get gps data
void data(){
    digitalWrite(greenPin, 255);

    int batteryLevel = modem.getBattPercent();
    Serial.print("Battery Level: ");
    Serial.print(batteryLevel);
    Serial.println("%");

    if(batteryLevel <= 54){
      digitalWrite(greenPin, 0);
      digitalWrite(redPin, 255);
    }else{
      digitalWrite(redPin, 0);
    }
    delay(1000);

    Serial.println("Umabot na dito...");

    fireData="";

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
                                  String(gps.time.second()) + "\",";
    fireData += " \"battery\" : " + String(batteryLevel);
    fireData += "}";
}

//send data to firebase
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
    http->stop();// Shutdown
    digitalWrite(greenPin, 0);
    digitalWrite(redPin, 0);
    Serial.println("HTTP POST disconnected");
    SerialAT.begin(modemBAUD);
    connectInternet();
    fireData="";
    Serial.begin(9600);
  }
}

void updateData(){
  sendData(UPDATE_PATH, fireData, &https);
}

void displayInfo(){
  if(gps.location.isValid()){
    data();
  }else{
    // data will be sent even if it's not valid
    data();
  }

  SerialAT.begin(modemBAUD);
  updateData();
  delay(9000);
  fireData="";
}