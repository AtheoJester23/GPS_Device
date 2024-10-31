#define TINY_GSM_MODEM_SIM800

#include <TinyGsmClient.h> //https://github.com/vshymanskyy/TinyGSM
#include <ArduinoHttpClient.h> //https://github.com/arduino-libraries/ArduinoHttpClient

#include <SoftwareSerial.h>
SoftwareSerial sim800(4, 2);

const char FIREBASE_HOST[]  = "siml-19451-default-rtdb.firebaseio.com";
const String FIREBASE_PATH  = "/";  // Update this path to where you want to post the data
const int SSL_PORT          = 443;

char apn[]  = "smartlte";  // type your APN here
char user[] = "";
char pass[] = "";

TinyGsm modem(sim800);

TinyGsmClientSecure gsm_client_secure_modem(modem, 0);
HttpClient http_client = HttpClient(gsm_client_secure_modem, FIREBASE_HOST, SSL_PORT);

unsigned long previousMillis = 0;

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
  http_client.setHttpResponseTimeout(10 * 1000); // 10 secs timeout
}

void loop()
{ 
  http_client.connect(FIREBASE_HOST, SSL_PORT);
 
  while (true) {
    if (!http_client.connected())
    {
      Serial.println();
      http_client.stop();// Shutdown
      Serial.println("HTTP not connect");
      break;
    }
    else
    {
      String jsonData = "{\"age\":22,\"email\":\"someone@gmail.com\",\"name\":\"nobody\"}";  // Updated data
      PostFirebase("PATCH", "/Update", jsonData, &http_client);  // PATCH request to /Update
    }
  }
}

void PostFirebase(const char* method, const String & path, const String & data, HttpClient* http)
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

  http->patch(url, "application/json", data);  // Use PATCH request for updating

  statusCode = http->responseStatusCode();  // Get status code for verification
  Serial.print("Status code: ");
  Serial.println(statusCode);

  response = http->responseBody();
  Serial.print("Response: ");
  Serial.println(response);

  if (!http->connected())
  {
    Serial.println();
    http->stop();  // Shutdown
    Serial.println("HTTP POST disconnected");
  }
}