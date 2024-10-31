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
const String UPDATE_PATH = "/ArduinoDeviceId/12345678902";

HttpClient https(client, server, port);

String fireData="";

TinyGPSPlus gps;

int redPin = 12;  
int greenPin = 11; 
int bluePin = 10;  

unsigned long lastBalanceCheck = 0;
String operatorName;

bool balanceChecked = true;

String secondToLastLine = ""; 

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
  Serial.println(F("Modem successfully restarted"));
  
  String modemInfo = modem.getModemInfo();
  Serial.print(F("Modem: "));
  Serial.println(modemInfo);
  Serial.println(F("wait 1 seconds"));
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
    if (millis() - lastBalanceCheck >= 30000 && !balanceChecked) {
        lastBalanceCheck = millis();
        getDataBal();
        SerialAT.println(F("AT+CUSD=2"));
        balanceChecked = true; // Set flag after checking balance
        Serial.print(F("Balance Checked Flag Set: "));
        Serial.println(balanceChecked);
    } 

    int batteryLevel = modem.getBattPercent();
    Serial.print(F("Battery Level: "));
    Serial.print(batteryLevel);
    Serial.println(F("%"));

    String date = "";        
    String hourMinute = "";

    // Print the operator name
    Serial.print(F("Operator Name: "));
    Serial.println(operatorName);



    if (modem.testAT()) {
      Serial.println(F("SIM800L connected successfully!"));
      
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
        delay(500);

        Serial.println(F("Umabot na dito..."));

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
    Led(greenPin, 5);
    updateData();
  }else{
    Serial.println(F("Wait di pa valid yung GPS data..."));
  }
  fireData="";
  balanceChecked = false;  // Reset flag after Firebase update
  Serial.print("Free memory: ");
  Serial.print(availableMemory());
  Serial.println(" bytes");
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

String captureData() {
  String line = "";  // Local variable to hold the balance line
  unsigned long startTime = millis();
  
  while (millis() - startTime < 30000) {  // Adjust timeout as needed
    if (SerialAT.available()) {
      String response = SerialAT.readString();

      // Check for "+CMT" response
      if (response.indexOf("+CMT") != -1) {
        int lastNewlineIndex = response.lastIndexOf('\n');
        if (lastNewlineIndex != -1) {
          int secondToLastNewlineIndex = response.lastIndexOf('\n', lastNewlineIndex - 1);

          if (secondToLastNewlineIndex != -1) {
            line = response.substring(secondToLastNewlineIndex + 1, lastNewlineIndex);
            line.trim();  // Remove any leading or trailing whitespace
            Serial.print("Data Balance: ");
            Serial.println(line);
            return line;  // Return the balance line once found
          }
        }
      }
    }
  }
  Serial.println(F("No valid second-to-last line found in the response."));
}

void getDataBal() {
  // Enable USSD mode
  SerialAT.println(F("AT+CUSD=1"));
  
  // Check balance
  SerialAT.println(F("AT+CUSD=1,\"*123#\""));
  
  // Navigate through USSD options if needed
  delay(5000);  // Wait for response before sending next command
  SerialAT.println(F("AT+CUSD=1,\"8\""));
  
  delay(3000);  // Adjust delay based on response time
  SerialAT.println(F("AT+CUSD=1,\"1\""));

  // Capture balance and store in global variable
  secondToLastLine = captureData();
}

// Function to check available free memory
int availableMemory() {
  int size = 2048;  // Maximum SRAM size for ATmega328p is 2048 bytes
  byte *buf;
  while ((buf = (byte *) malloc(--size)) == NULL);  // Decrease size until malloc is successful
  free(buf);  // Free allocated memory
  return size;
}

