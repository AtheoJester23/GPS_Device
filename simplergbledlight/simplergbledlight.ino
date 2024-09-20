// Pin assignments for the RGB LED
int redPin = 12;    // Red LED pin
int greenPin = 11; // Green LED pin
int bluePin = 10;  // Blue LED pin

void setup() {
  // Set the RGB pins as output
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
}

void loop() {
  // Red color
  setColor(255, 0, 0); // Red
  delay(1000);         // 1 second delay

  // Green color
  setColor(0, 255, 0); // Green
  delay(1000);

  // Blue color
  setColor(0, 0, 255); // Blue
  delay(1000);

  // Turn off all
  setColor(0, 0, 0);  // Off
  delay(1000);
}

void setColor(int red, int green, int blue) {
  analogWrite(redPin, red);    // For common cathode, reverse the value
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);
}
