#define TINY_GSM_MODEM_SIM800

#include <TinyGsmClient.h> //https://github.com/vshymanskyy/TinyGSM
#include <ArduinoHttpClient.h> //https://github.com/arduino-libraries/ArduinoHttpClient

#include <SoftwareSerial.h>
SoftwareSerial sim800(4, 2);
 
// RGB LED Pins
const int redPin = 10;
const int greenPin = 11;
const int bluePin = 12;
 
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
 
bool isConnected = false; // Boolean to track connection status
unsigned long previousMillis = 0; // Time tracking for LED blink
const long interval = 500; // Interval for blinking the LED

void setup()
{
  Serial.begin(9600);
  sim800.begin(4800);

  // Set up RGB LED pins as output
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  setRGBColor(0, 0, 255); // Turn off the LED initially

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
    if(isConnected == false){
      blinkRedLED();
    }else{
      setRGBColor(0, 255, 0);
    }

    if (!http_client.connected())
    {
      Serial.println();
      http_client.stop();// Shutdown
      Serial.println("HTTP  not connect");
      blinkRedLED();
      break;
    }
    else
      isConnected=true;
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

void blinkRedLED() {
  unsigned long currentMillis = millis();

  // If enough time has passed, toggle the LED
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Alternate between red on and off
    static bool ledState = false;
    if (ledState) {
      setRGBColor(255, 0, 0); // Red
    } else {
      setRGBColor(0, 0, 0); // Off
    }
    ledState = !ledState;
  }
}

void setRGBColor(int red, int green, int blue) {
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);
}