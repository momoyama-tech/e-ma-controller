//templeate for child
const int dataLength = 28;    // Length of data to send/receive = number of flowers
const int touchPins[] = {T4, T5, T6, T7, T8, T9};  // Touch sensor pins for the child unit
const int ledPins[] = {5, 18, 19, 21, 22, 23};     // LED pins
const int numLeds = sizeof(ledPins) / sizeof(ledPins[0]); // Number of LEDs

int num = ;  // put child unit's index (e.g., second unit)

byte touchData[dataLength] = {0};  // Touch state data (not used in this sketch)

const int threshold[] = {10, 10, 10, 10, 10, 10}; // Threshold values for touch detection

byte sendData[dataLength] = {0};       // Data to be sent to the parent
byte receivedData[dataLength] = {0};   // Data received from the parent
long intensity[dataLength] = {0};      // Brightness level for each LED

HardwareSerial mySerial(1);            // Hardware serial port for communication with parent
const int Rx1Pin = 25;                 // RX pin for parent communication
const int Tx1Pin = 26;                 // TX pin for parent communication

#define TXD2 17 // GPIO17 used as TX for communication with another child (optional)
#define RXD2 16 // GPIO16 used as RX for communication with another child (optional)

void setup() {
  Serial.begin(115200);  // Serial monitor communication
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2); // Initialize Serial2 (child-child communication)
  mySerial.begin(115200, SERIAL_8N1, Rx1Pin, Tx1Pin); // Initialize Serial1 (child-parent communication)

  // Initialize LED pins with PWM control
  for (int i = 0; i < numLeds; i++) {
    ledcAttach(ledPins[i], 5000, 8); // PWM at 5kHz, 8-bit resolution
  }
}

// Update touch state and store it in the corresponding position of sendData
void updateTouchState(int index, int touchedValue) {
  sendData[index + dataLength - num * 6] = (touchedValue < threshold[index]) ? 1 : 0; // Store this unit's touch state in the last 6 slots
}

// Receive data from another child unit via Serial2 (optional)
void receiveFromChild() {
  static byte buffer[dataLength];
  static int index = 0;

  while (Serial2.available()) {
    char incomingByte = Serial2.read();

    if (incomingByte == '\n') {  // End of message detected
      int copyLength = (index < dataLength) ? index : dataLength;
      for (int i = 0; i < copyLength; i++) {
        sendData[i] = buffer[i]; // Copy received data into sendData
      }
      index = 0;  // Reset buffer
      break;
    } else {
      if (index < dataLength) {
        buffer[index++] = (byte)incomingByte;
      } else {
        // Ignore overflow
      }
    }
  }
}

// Send this unit's data to the parent unit via mySerial
void sendToParent() {
  for (int i = 0; i < dataLength; i++) {
    Serial2.write(sendData[i]);
  }
  Serial2.write('\n');  // End-of-line marker
}

// Receive LED brightness data from the parent unit
void receiveFromParent() {
  static byte buffer[dataLength];
  static int index = 0;

  while (mySerial.available()) {
    char incomingByte = mySerial.read();

    if (incomingByte == '\n') {  // End of message detected
      int copyLength = (index < dataLength) ? index : dataLength;
      for (int i = 0; i < copyLength; i++) {
        receivedData[i] = buffer[i]; // Copy received data into receivedData
      }
      index = 0;  // Reset buffer
      break;
    } else {
      if (index < dataLength) {
        buffer[index++] = (byte)incomingByte;
      } else {
        // Ignore overflow
      }
    }
  }

  // Convert received data to brightness values
  for (int i = 0; i < dataLength; i++) intensity[i] = (long)receivedData[i];
}

// Update each LED's brightness based on received intensity and touch state
void updateLedBrightness() {
  for (int i = 0; i < numLeds; i++) {
    int touchedValue = touchRead(touchPins[i]); // Read touch sensor value
    updateTouchState(i, touchedValue); // Update touch state
    // Set LED brightness: base intensity + small boost if touched
    ledcWrite(ledPins[i], intensity[i + dataLength - num * 6] + sendData[i + dataLength - num * 6]*5);
  }
}

// Main loop
void loop() {
  receiveFromChild();     // Receive data from other child unit (if used)
  updateLedBrightness();  // Update LED brightness
  sendToParent();         // Send current touch state to the parent
  receiveFromParent();    // Receive updated LED intensities from the parent
}
