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
int count=0;
String fireData="";

#ifdef DUMP_AT_COMMANDS
  #include <StreamDebugger.h>
  StreamDebugger debugger(SerialAT, SerialMon);
  TinyGsm modem(debugger);
#else
  TinyGsm modem(SerialAT);
#endif

TinyGsmClientSecure client(modem);
HttpClient https(client, server, port);

TinyGPSPlus gps;

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
    scan();}
  }
  else{
    //Serial.println("gps not listening");
    fireData="";
    ss.begin(gpsBAUD); 
  
  } 
}

//send data to firebase
void sendData(const char* method, const String & path , const String & data, HttpClient* http) {

  http->connectionKeepAlive(); // Currently, this is needed for HTTPS
  String url;
  if (path[0] != '/') {
    url = "/";
  }
  url += path + ".json";
  url += "?print=silent";
  url += "&x-http-method-override=";
  url += String(method);
  //Serial.print("POST:");
  //Serial.println(url);
  String contentType = "application/json";
  http->post(url, contentType, data);
  // read the status code and body of the response
  int statusCode = http->responseStatusCode();
  http->responseBody();
  Serial.print("Status code: ");
  Serial.println(statusCode);
  
  //Serial.print("Response: ");
  //Serial.println(response);
  
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
  //Serial.print(F("Realtime Loc: ")); 
  if (gps.location.isValid())
  {
    //Serial.print(gps.location.lat(), 6);
    //Serial.print(F(","));
    //Serial.print(gps.location.lng(), 6);
    data();
  }
  else
  {
   // Serial.print(F("INVALID"));
    data();
  }
  //Serial.println();
  SerialAT.begin(modemBAUD);
  updateData();   
}

// connect to internet 
void tesKoneksi(){
  SerialMon.print(F("Connecting to apn: "));
  SerialMon.print(apn);
  while (!modem.gprsConnect(apn, user, pass)) {
    Serial.println("GPRS connection failed, retrying in 10 seconds...");
    delay(10000);  // Wait before retrying
  }
  Serial.println("GPRS connected");
}

//updata data firebase
void updateData(){
  sendData("PUT", UPDATE_PATH, fireData, &https);
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
    fireData="";
    fireData += "{";
    fireData += " \"lat\" : \"\\\"" + String(gps.location.lat(),6)+"\\\"\",";
    fireData += " \"lng\" : \"\\\"" + String(gps.location.lng(),6)+"\\\"\"";
    fireData += "}";
}

//parsing gsm location coordinates and gsm time
String gsmData(String data,int pos)
{
  char separator=',';
  int index=pos;
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}
