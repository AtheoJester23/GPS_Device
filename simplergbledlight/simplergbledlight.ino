// Define the pins for the RGB LED
const int redPin = 12;
const int greenPin = 10;
const int bluePin = 11;

void setup() {
  // Set the RGB LED pins as outputs
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
}

void loop() {
  // Smooth transition from Red to Green
  for (int i = 0; i <= 128; i++) {
    analogWrite(redPin, 128 - i);
    analogWrite(greenPin, i);
    analogWrite(bluePin, 0);
    delay(10);
  }

  // Smooth transition from Green to Blue
  for (int i = 0; i <= 128; i++) {
    analogWrite(redPin, 0);
    analogWrite(greenPin, 128 - i);
    analogWrite(bluePin, i);
    delay(10);
  }

  // Smooth transition from Blue to Red
  for (int i = 0; i <= 128; i++) {
    analogWrite(redPin, i);
    analogWrite(greenPin, 0);
    analogWrite(bluePin, 128 - i);
    delay(10);
  }

  // Smooth transition to White (all colors on)
  for (int i = 0; i <= 128; i++) {
    analogWrite(redPin, i);
    analogWrite(greenPin, i);
    analogWrite(bluePin, i);
    delay(10);
  }

  // Hold White for 1 second
  delay(1000);

  // Smooth transition from White to Red (reset)
  for (int i = 128; i >= 0; i--) {
    analogWrite(redPin, 128);
    analogWrite(greenPin, i);
    analogWrite(bluePin, i);
    delay(10);
  }
}
