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
SoftwareSerial ss(8, 9); // RX,TX serial GPS

// Internet setting
const char apn[]  = "smartlte"; // GSM apn 
const char user[] = "";
const char pass[] = "";

//Firebase setting 
const char server[] = "siml-19451-default-rtdb.firebaseio.com";
const int  port = 443;
const String UPDATE_PATH = "gps_devices/"+ String(DEVICE_ID); // firebase root table

//global variables
String fireData="";

TinyGsm modem(SerialAT);

TinyGsmClientSecure client(modem);
HttpClient https(client, server, port);

TinyGPSPlus gps;

unsigned long lastUpdateTime = 0;  // Store the last update time for 15-minute updates
const unsigned long updateInterval = 900000; // 15 minutes in milliseconds (15 * 60 * 1000)

void setup() {
  // Set console baud rate
  SerialMon.begin(115200);
  delay(10);
  initializeModem(); // check modem communication
  tesKoneksi(); // test internet connection
  ss.begin(gpsBAUD); // open gps serial

}

void loop() {

  if(ss.isListening()){
    //Serial.println("gps listening");
    while(fireData.equals("")){
      scan();
    }
  }
  else{
    //Serial.println("gps not listening");
    fireData="";
    ss.begin(gpsBAUD); 
  } 
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
    Serial.println("HTTP POST disconnected");
    SerialAT.begin(modemBAUD);
    tesKoneksi();
    fireData="";
    ss.begin(gpsBAUD);
  }
}

//scanning gps location
void scan(){
  while (ss.available() > 0)
    if (gps.encode(ss.read()))
      displayInfo(); 
}

void displayInfo()
{
  if (gps.location.isValid())
  {
    data();
  }
  else
  {
    data();
  }
  //Serial.println();
  SerialAT.begin(modemBAUD);
  updateData();   
}

// connect to internet 
void tesKoneksi(){
  SerialMon.print(F("Connecting to apn: "));
  SerialMon.println(apn);
  while (!modem.gprsConnect(apn, user, pass)) {
    Serial.println("GPRS connection failed, retrying in 10 seconds...");
    delay(10000);  // Wait before retrying
  }
  Serial.println("GPRS connected");
}

//updata data firebase
void updateData(){
  sendData(UPDATE_PATH, fireData, &https);
  //kirimData();
}

//initialize modem sim800
void initializeModem(){
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

//get gps data
void data(){
    // to delay send
    // SerialMon.println("Wait for 30 sec...");
    // delay(10000);

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
                                  String(gps.time.second()) + "\"";
    // // Check if 15 minutes have passed
    // if (millis() - lastUpdateTime > updateInterval) {
    //     // Add last latitude and last longitude
    //     fireData += ", \"lastLatitude\" : " + String(gps.location.lat(), 6) + ",";
    //     fireData += " \"lastLongitude\" : " + String(gps.location.lng(), 6);
    //     lastUpdateTime = millis();  // Reset the last update time
    // }
    fireData += "}";
}