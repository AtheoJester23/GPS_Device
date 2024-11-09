#define TINY_GSM_MODEM_SIM800

#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>
#include <ArduinoHttpClient.h>
#include <TinyGsmClient.h>

#define DEVICE_ID "12345678902"
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
const String UPDATE_PATH = "/ArduinoDeviceId123/12345678932";

HttpClient https(client, server, port);

String fireData="";

TinyGPSPlus gps;

int redPin = 12;  
int greenPin = 11; 
int bluePin = 10;  

unsigned long lastBalanceCheck = 0;
String operatorName;

bool balanceChecked = false;

String balanceData = "";  

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

  SerialAT.println(F("AT"));

  Serial.println(balanceData);

  Serial.println(F("Initializing modem..."));
  Led(bluePin, 5);

  while (!modem.restart()) {
    Serial.println(F("Failed to restart modem, trying again in 10 seconds..."));
    delay(10000);
  }
  Serial.println(F("Modem successfully restarted"));

  delay(1000);
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

    // Print the operator name
    Serial.print(F("Operator Name: "));
    Serial.println(operatorName);



    if(batteryLevel <= 15){
      digitalWrite(greenPin, 0);
      digitalWrite(A2, LOW);
      digitalWrite(A4, HIGH);
    }else{
      digitalWrite(redPin, 0);
      digitalWrite(A4, LOW);
      digitalWrite(A2, HIGH);
    }
    delay(500);

    Serial.println(F("Umabot na dito..."));

    fireData="";

    fireData += "{";
    fireData += " \"longitude\" : " + String(gps.location.lng(), 6) + ",";
    fireData += " \"status\" : \"Online\",";
    fireData += "\"data_balance\" : \"" + balanceData + "\",";
    fireData += " \"battery\" : " + String(batteryLevel);
    fireData += "}";
}

void sendData(const String & path , const String & data, HttpClient* http) {  
  if (!modem.isGprsConnected()) {
    Serial.println(F("GPRS not connected, reconnecting..."));
    connectInternet();
  }


  http->connectionKeepAlive(); 
  String url;
  if (path[0] != '/') {
    url = "/";
  }
  url += path + ".json";

  http->get(url);

  int statusCode = http->responseStatusCode();
  Serial.print(F("Status code (GET): "));
  Serial.println(statusCode);

  if (statusCode > 0) {
    String response = http->responseBody();

    if (response == "null" || response.length() == 0) {
      Serial.println(F("firebase(response is null)..."));
      http->stop();
      return false;
    }
    Serial.println(F("The path does exist"));

    http->patch(url, "application/json", data);
  }
  
  if (!http->connected()) {
    http->stop();
    Serial.println(F("HTTP POST disconnected"));
    fireData="";
  }

  http->stop();
}

void updateData(){
  sendData(UPDATE_PATH, fireData, &https);
}

void displayInfo(){
  if(gps.location.isValid()){
    data();
    updateData();
    Led(greenPin, 5);
  }else{
    Serial.println(F("Wait di pa valid yung GPS data..."));
  }
  fireData="";
  balanceChecked = false;
}

void Led(int pin, int num) {
  for (int i = 0; i < num; i++) {
    digitalWrite(pin, HIGH);  
    delay(500);               
    digitalWrite(pin, LOW);   
    delay(500);               
  }
}

String filterPrintable(String data) {
  String cleanedData = "";
  for (int i = 0; i < data.length(); i++) {
    if (isPrintable(data[i])) {
      cleanedData += data[i];  // Append only printable characters
    }
  }
  return cleanedData;
}

String captureData() {
  unsigned long startTime = millis();
  int cusdCount = 0;  // Counter to track occurrences of "+CUSD"
  String extractedData = ""; // Temporary storage for extracted data

  while (millis() - startTime < 30000) {  // Adjust timeout as needed
    if (SerialAT.available()) {
      String response = SerialAT.readString();

      // Check for "+CUSD" response
      if (response.indexOf("+CUSD") != -1) {
        cusdCount++;  // Increment counter on finding "+CUSD"

        if (cusdCount == 2) {  // Process the second "+CUSD" only
          int backIndex = response.indexOf("0) Back");  // Locate "0) Back"
          if (backIndex != -1) {
            // Find the newline just before "0) Back"
            int startIndex = response.lastIndexOf('\n', backIndex - 1);
            if (startIndex != -1) {
              // Find the previous newline, indicating the start of the target line
              int endIndex = startIndex;
              startIndex = response.lastIndexOf('\n', startIndex - 1);
              if (startIndex != -1) {
                // Extract the line above "0) Back"
                extractedData = response.substring(startIndex + 1, endIndex);
                extractedData.trim();  // Remove any leading/trailing whitespace
                
                // Filter non-printable characters
                extractedData = filterPrintable(extractedData);
                
                Serial.print("Data: ");
                Serial.println(extractedData);
                return extractedData;  // Return the cleaned data
              }
            }
          }
        }
      }
    }
  }
  Serial.println("No valid data line found before '0) Back'.");
}

void getDataBal() {
  // Enable USSD mode
  SerialAT.println(F("AT+CUSD=1"));
  delay(1000);
  
  // Check balance
  SerialAT.println(F("AT+CUSD=1,\"*123#\""));
  delay(7000);
  
  // Navigate through USSD options if needed
  SerialAT.println(F("AT+CUSD=1,\"8\""));
  unsigned long startTime = millis();
  bool step2Confirmed = false;

  // Wait for expected response for Step 2
  while (millis() - startTime < 20000) {  // 10 seconds timeout
    if (SerialAT.available()) {
      String response = SerialAT.readString();
      if (response.indexOf("+CUSD") != -1) {  // Check for option "8" response
        Serial.println("Option '8' response received.");
        step2Confirmed = true;
        break;
      }
    }
  }
  
  if (!step2Confirmed) {
    Serial.println("Failed to receive option '8' response.");
    return;  // Exit if step 2 fails
  }
  delay(2000);  // Additional delay for processing

  SerialAT.println(F("AT+CUSD=1,\"1\""));

  // Capture balance and store in global variable
  balanceData = captureData();
}

