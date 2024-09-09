#define TINY_GSM_MODEM_SIM800

#include <TinyGsmClient.h> //https://github.com/vshymanskyy/TinyGSM
#include <ArduinoHttpClient.h> //https://github.com/arduino-libraries/ArduinoHttpClient
#include <TinyGPS++.h>
#include <AltSoftSerial.h>
#include <SoftwareSerial.h>
SoftwareSerial sim800(4, 2);
 

 
const char FIREBASE_HOST[]  = "siml-19451-default-rtdb.firebaseio.com";
const String FIREBASE_AUTH  = "G3ehgtFyaYSN3LJGpcGIe5OrWxLM6mJ3lKEGe3Ir";
const String FIREBASE_PATH  = "/";
const int SSL_PORT          = 443;
 
char apn[]  = "smartlte";  // type your APN here
char user[] = "";
char pass[] = "";
 
 
TinyGsm modem(sim800);
 
TinyGsmClientSecure gsm_client_secure_modem(modem, 0);
HttpClient http_client = HttpClient(gsm_client_secure_modem, FIREBASE_HOST, SSL_PORT);


// TinyGPSPlus gps;
// AltSoftSerial gpsSerial; 
 
void setup()
{
  Serial.begin(9600);
  sim800.begin(9600);

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
  
  String modemInfo = modem.getModemInfo();
  Serial.print("Modem: ");
  Serial.println(modemInfo);
  http_client.setHttpResponseTimeout(10 * 1000); //^0 secs timeout
}
 
void loop()
{ 
  http_client.connect(FIREBASE_HOST, SSL_PORT);
 
  while (true) {
    if (!http_client.connected())
    {
      Serial.println();
      http_client.stop();// Shutdown
      Serial.println("HTTP  not connect");
      break;
    }
    else
      GetFirebase("PATCH", FIREBASE_PATH, &http_client);
     
  }
 
}
 
 
 
void GetFirebase(const char* method, const String & path ,  HttpClient* http)
{
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

  http->get(url);
 
 //statusCode = http->responseStatusCode();
 // Serial.print("Status code: ");
  //Serial.println(statusCode);
  response = http->responseBody();
 
  Serial.print("Response: ");
  Serial.println(response);

  if (!http->connected())
  {
    Serial.println();
    http->stop();// Shutdown
    Serial.println("HTTP POST disconnected");
  }
 
}