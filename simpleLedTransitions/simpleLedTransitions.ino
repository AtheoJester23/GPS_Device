// Pin assignments
const int bluePin = 10;
const int greenPin = 11;
const int redPin = 12;

// Time for each color transition
const int transitionTime = 10; // Delay between color changes (in milliseconds)

void setup() {
  // Set RGB LED pins as outputs
  pinMode(bluePin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(redPin, OUTPUT);
}

void loop() {
  // Red to Green transition
  for (int i = 0; i <= 255; i++) {
    analogWrite(redPin, 255 - i);   // Fade red down
    analogWrite(greenPin, i);       // Fade green up
    delay(transitionTime);
  }

  // Green to Blue transition
  for (int i = 0; i <= 255; i++) {
    analogWrite(greenPin, 255 - i); // Fade green down
    analogWrite(bluePin, i);        // Fade blue up
    delay(transitionTime);
  }

  // Blue to Red transition
  for (int i = 0; i <= 255; i++) {
    analogWrite(bluePin, 255 - i);  // Fade blue down
    analogWrite(redPin, i);         // Fade red up
    delay(transitionTime);
  }
}
