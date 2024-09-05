#define TINY_GSM_MODEM_SIM800

#include <TinyGsmClient.h> //https://github.com/vshymanskyy/TinyGSM
#include <ArduinoHttpClient.h> //https://github.com/arduino-libraries/ArduinoHttpClient

#include <SoftwareSerial.h>
SoftwareSerial sim800(4, 2);

const char FIREBASE_HOST[]  = "siml-19451-default-rtdb.firebaseio.com";
const String FIREBASE_AUTH  = "G3ehgtFyaYSN3LJGpcGIe5OrWxLM6mJ3lKEGe3Ir";
const String FIREBASE_PATH  = "/b";  // Path for object "b"
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

  while (!http_client.connect(FIREBASE_HOST, SSL_PORT)) {
    Serial.println("Connection to Firebase failed, retrying in 10 seconds...");
    delay(10000);  // Wait before retrying
  }
  Serial.println("Connected to firebase");

  // Check if "b" exists, then post if it doesn't
  checkAndPostIfBDoesNotExist();
}

void loop()
{
  
}

void checkAndPostIfBDoesNotExist()
{
  String response = GetFirebase("/b", &http_client);

  // If "b" does not exist (response is empty or null)
  if (response == "null") {
    Serial.println("\"b\" does not exist, posting new data...");

    // Data for "b"
    String jsonData = "{\"name\":\"bilog\",\"age\":\"4months old\",\"email\":\"none\"}";
    
    // Post the object "b"
    PostFirebase("/b", jsonData, &http_client);
  } else {
    Serial.println("\"b\" already exists, no need to post.");
  }

  http_client.stop(); // Shutdown the connection
}

String GetFirebase(const String & path , HttpClient* http)
{
  String response;
  http->connectionKeepAlive();

  String url;
  
  if (path[0] != '/')
  {
    url = "/";
  }
  url += path + ".json";
  url += "?auth=" + FIREBASE_AUTH;

  // Perform GET request to check if "b" exists
  http->get(url);

  int statusCode = http->responseStatusCode();
  Serial.print("GET status code: ");
  Serial.println(statusCode);

  response = http->responseBody();
  Serial.print("GET response: ");
  Serial.println(response);

  return response;
}

void PostFirebase(const String & path, const String & data, HttpClient* http)
{
  String url;
  
  if (path[0] != '/')
  {
    url = "/";
  }
  url += path + ".json";
  url += "?auth=" + FIREBASE_AUTH;

  // Use POST request to create "b"
  http->post(url, "application/json", data);

  int statusCode = http->responseStatusCode();
  Serial.print("POST status code: ");
  Serial.println(statusCode);

  String response = http->responseBody();
  Serial.print("POST response: ");
  Serial.println(response);
}
