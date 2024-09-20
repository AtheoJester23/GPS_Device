// RGB LED Pins
const int bluePin = 10;
const int greenPin = 11;
const int redPin = 12;

bool isConnected = true; // Boolean to track connection status
unsigned long previousMillis = 0; // Time tracking for LED blink
const long interval = 50; // Interval for blinking the LED

void setup() {
  Serial.begin(9600);

  // Set up RGB LED pins as output
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
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
    analogWrite(bluePin, 255); // Turn on the blue LED
    delay(500);                // Wait 500 ms
    analogWrite(bluePin, 0);   // Turn off the blue LED
    delay(500);                // Wait 500 ms
  }
  analogWrite(bluePin, 0);     // Make sure the LED is off if it ever stops (though it won't)
}