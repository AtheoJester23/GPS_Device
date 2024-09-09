void setup() {
  Serial.begin(9600);
  delay(3000);  // Give time for serial to initialize
  
  Serial.print("Free memory: ");
  Serial.print(availableMemory());
  Serial.println(" bytes");
}

void loop() {
  // Your main code goes here
}

// Function to check available free memory
int availableMemory() {
  int size = 2048;  // Maximum SRAM size for ATmega328p is 2048 bytes
  byte *buf;
  while ((buf = (byte *) malloc(--size)) == NULL);  // Decrease size until malloc is successful
  free(buf);  // Free allocated memory
  return size;
}
