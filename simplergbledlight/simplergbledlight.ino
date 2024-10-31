// Pin assignments for the RGB LED
int redPin = 12;    // Red LED pin
int greenPin = 11; // Green LED pin
int bluePin = 10;  // Blue LED pin

void setup() {
  // Set the RGB pins as output
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  pinMode(A2, OUTPUT);
  pinMode(A4, OUTPUT);
}

void loop() {
  // Red color
  setColor(255, 0, 0); // Red
  delay(500);         // 1 second delay

  // Turn off all
  setColor(0, 0, 0);  // Off
  delay(500);

  // Green color
  setColor(0, 255, 0); // Green
  delay(500);

  // Turn off all
  setColor(0, 0, 0);  // Off
  delay(500);

  // Blue color
  setColor(0, 0, 255); // Blue
  delay(500);

  // Turn off all
  setColor(0, 0, 0);  // Off
  delay(500);

  digitalWrite(A2, HIGH);
  delay(500);

  digitalWrite(A2, LOW);
  delay(500);

  digitalWrite(A4, HIGH);
  delay(500);

  digitalWrite(A4, LOW);
  delay(500);
}

void setColor(int red, int green, int blue) {
  analogWrite(redPin, red);    // For common cathode, reverse the value
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);
}
