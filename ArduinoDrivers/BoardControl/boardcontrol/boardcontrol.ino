// Pin definitions
const int latchPin = 22;      // Pin to trigger the latch
const int clockEnablePin = 23; // Clock enable pin
const int dataIn = 25;        // Serial data input pin
const int clockPin = 24;      // Shift clock pin

// Number of shift registers in the chain
const int numRegisters = 8; // 8 shift registers
const int numBits = numRegisters * 8; // Total bits to read

void setup() {
  Serial.begin(9600);

  pinMode(latchPin, OUTPUT);
  pinMode(clockEnablePin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataIn, INPUT);
  
  digitalWrite(clockPin, LOW); // Ensure clock starts low

  digitalWrite(clockEnablePin, LOW); // Ensure clock is enabled
  digitalWrite(latchPin, HIGH);
}

void testShiftIn() {
  digitalWrite(latchPin, LOW);
  delayMicroseconds(5); // Small delay for stability
  digitalWrite(latchPin, HIGH);
  delayMicroseconds(5); // Small delay for stability

  digitalWrite(clockPin, HIGH);
  digitalWrite(clockEnablePin, LOW);
  byte incoming = shiftIn(dataIn, clockPin, MSBFIRST);
  digitalWrite(clockEnablePin, HIGH);

  Serial.print("PIN STATES:\r\n");
  Serial.println(incoming, BIN);
  delay(1000); // Wait 1 second before reading again
}

void loop() {
  // Trigger the latch to store the current state of the inputs
  digitalWrite(latchPin, LOW);
  delayMicroseconds(5); // Small delay for stability
  digitalWrite(latchPin, HIGH);
  delayMicroseconds(5);

  // Read the data from the shift registers
  uint64_t boardState = readShiftRegisters();
  // Print the board state in binary
  Serial.print("Board state: ");
  for (int i = numBits - 1; i >= 0; i--) {
    Serial.print((int)(boardState >> i) & 1); // Print each bit
    if (i % 8 == 0) Serial.print(" ");  // Add space after every 8 bits
  }
  Serial.println();

  delay(1000); // Wait 1 second before reading again
//testShiftIn();

}

uint64_t readShiftRegisters() {
  uint64_t result = 0; // Initialize result
  for (int i = 0; i < numBits; i++) {
    digitalWrite(clockPin, LOW);  // Start with clock LOW
    delayMicroseconds(5);         // Small delay for stability
    result <<= 1;                 // Shift result to the left
    result |= digitalRead(dataIn); // Read data bit
    digitalWrite(clockPin, HIGH); // Pulse clock HIGH
    delayMicroseconds(5);         // Small delay
  }
  digitalWrite(clockPin, LOW);    // Ensure clock ends LOW
  return result;
}
