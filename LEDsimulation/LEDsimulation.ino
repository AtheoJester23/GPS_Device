// RGB LED Pins
const int bluePin = 10;
const int redPin = 11;
const int redPin2 = 7;
const int greenPin = 6;
const int testRedPin8 = 8;
const int testBluePin9 = 9;


bool isConnected = true; // Boolean to track connection status
unsigned long previousMillis = 0; // Time tracking for LED blink
const long interval = 50; // Interval for blinking the LED

void setup() {
  Serial.begin(9600);


  pinMode(redPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  delay(2000);

  blinkLed();
  Serial.println("Imagine sim800L getting ready...");
  delay(7000);
}

void loop() {
}

void blinkLed() {
  // Continuous blinking until power is removed
  while (isConnected) {  // isConnected is always true, so it runs indefinitely
    // analogWrite(bluePin, 7); // Turn on the blue LED
    // analogWrite(redPin, 38); // Turn on the blue LED
    // delay(500);                // Wait 500 ms
    // analogWrite(redPin, 0); // Turn on the blue LED
    // analogWrite(bluePin, 0); // Turn on the blue LED
    // delay(500);                // Wait 500 ms

    // analogWrite(redPin2, 128); // Turn on the blue LED
    // delay(500);                // Wait 500 ms
    // analogWrite(redPin2, 0);
    // delay(500);                // Wait 500 ms

    analogWrite(greenPin, 5); // Turn on the blue LED
    delay(500);                // Wait 500 ms
    // analogWrite(greenPin, 0); // Turn on the blue LED
    // delay(500);                // Wait 500 ms

    // analogWrite(bluePin, 10); // Turn on the blue LED
    // delay(500);
    // analogWrite(bluePin, 0); // Turn on the blue LED
    // delay(500);                // Wait 500 ms

    // // For 2.2k resistor: Lowbat
    // analogWrite(testRedPin8, 180);
    // delay(500);
    // analogWrite(testRedPin8, 0);
    // delay(500);

    // analogWrite(testBluePin9, 250);
    // delay(500);
    // analogWrite(testBluePin9, 0);

    // For 2.7k resistor: Sending
    // delay(500);
    // analogWrite(testRedPin8, 250);
    // analogWrite(testBluePin9, 15);
    // delay(500);
    // analogWrite(testRedPin8, 0);
    // analogWrite(testBluePin9, 0);

    // // For 2.2k resistor: Sending
    // delay(500);
    // analogWrite(testRedPin8, 128);
    // analogWrite(testBluePin9, 30);
    // delay(500);
    // analogWrite(testRedPin8, 0);
    // analogWrite(testBluePin9, 0);


  }
}