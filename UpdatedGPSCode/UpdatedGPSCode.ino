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
const String UPDATE_PATH = "/ArduinoDeviceId/12345678902";

HttpClient https(client, server, port);

String fireData="";

TinyGPSPlus gps;

int redPin = 12;  
int greenPin = 11; 
int bluePin = 10;  

String operatorName;

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
  Led(bluePin, 3);
  while (!modem.restart()) {
    Serial.println(F("Failed to restart modem, trying again in 10 seconds..."));
    delay(10000);
  }
  Serial.println(F("Modem successfully restarted"));

  SerialAT.println("AT+CNTP=1");
  delay(1000);

}

void connectInternet(){
  SerialAT.println("AT+CNTP=1");
  delay(5000);
  Led(bluePin, 3);
  while (!modem.gprsConnect(apn, user, pass)) {
    Led(bluePin, 5);
    Serial.println(F("GPRS connection failed, retrying in 10 seconds..."));
    delay(10000);
  }
  Serial.println(F("GPRS connected"));

  sendATCommand("AT+COPS?");
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
          digitalWrite(greenPin, 0);
          digitalWrite(A2, LOW);
          digitalWrite(A4, HIGH);
        }else{
          digitalWrite(redPin, 0);
          digitalWrite(A4, LOW);
          digitalWrite(A2, HIGH);
        }
        delay(1000);

        fireData="";

        int adjustedHour = gps.time.hour() + 8;
        if (adjustedHour >= 24) {
            adjustedHour -= 24; 
        }

        fireData += "{";
        fireData += " \"latitude\" : " + String(gps.location.lat(), 6) + ","; 
        fireData += " \"longitude\" : " + String(gps.location.lng(), 6) + ",";
        fireData += " \"status\" : \"Online\",";
        fireData += " \"date\" : \"" + date + "\",";
        fireData += " \"time\" : \"" + hourMinute + "\",";
        fireData += " \"sim\" : \"" + operatorName + "\",";
        fireData += " \"battery\" : " + String(batteryLevel);
        fireData += "}";
      }
    } else {
      Serial.println(F("Failed to communicate with SIM800L"));
    }
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

  if (statusCode > 0) {
    String response = http->responseBody();

    if (response == "null") {
      Serial.println(F("firebase(response is null)..."));
      http->stop();
      return false;
    }
    Serial.println(F("Uploading..."));

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
  Led(greenPin, 3);
}

void displayInfo(){
  if(gps.location.isValid()){
    data();
    updateData();
  }else{
    Serial.println(F("Wait di pa valid yung GPS data..."));
  }
  delay(5000);
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

// Function to send AT command and print the response
void sendATCommand(const char* command) {
  Serial.println("Sending command: " + String(command));
  
  // Send the AT command
  SerialAT.println(command);
  delay(500); // Short delay to give the modem time to start responding
  
  String response;
  unsigned long startTime = millis(); // Track the start time for timeout
  unsigned long timeout = 2000; // 2 seconds timeout for response reading

  while (millis() - startTime < timeout) { 
    while (SerialAT.available()) { 
      char c = SerialAT.read(); 
      response += c;

      // Check if the response ends with "OK\r\n"
      if (response.endsWith("OK\r\n")) {
        // End of response detected, exit the loop
        break;
      }
    }

    // If the response is complete, break the outer timeout loop too
    if (response.endsWith("OK\r\n")) {
      break;
    }
  }

  // Print the complete response
  Serial.print(F("Response: "));
  Serial.println(response);

  // Extract the operator name from the response
  operatorName = extractOperatorName(response);
}

// Function to extract the operator name from the response
String extractOperatorName(const String& response) {
    String detectedOperator = "";

    // Check if the response contains "+COPS: "
    int startIndex = response.indexOf("+COPS: ");
    if (startIndex != -1) {
        int startQuoteIndex = response.indexOf("\"", startIndex); // Find the starting quote of the operator name
        int endQuoteIndex = response.indexOf("\"", startQuoteIndex + 1); // Find the ending quote of the operator name
        if (startQuoteIndex != -1 && endQuoteIndex != -1) {
            // Extract the full operator name
            detectedOperator = response.substring(startQuoteIndex + 1, endQuoteIndex); // Extract operator name only
            
            // Trim any whitespace (though it's unlikely)
            detectedOperator.trim(); 

            // Assign the short name based on the operator name
            if (detectedOperator == "Globe Telecom") {
                return "Globe";
            } else if (detectedOperator == "SMART Gold") {
                return "Smart";
            } else {
                return "Unknown"; // Fallback for other operators
            }
        }
    }
    return detectedOperator;
}