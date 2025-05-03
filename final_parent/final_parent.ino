const int dataLength = 28;    // Total number of data bytes to send/receive
const int touchPins[] = {T4, T5, T6, T7, T8, T9};  // Touch sensor pins for the main (parent) device
const int ledPins[] = {5, 18, 19, 21, 22, 23};     // LED output pins
const int numLeds = sizeof(ledPins) / sizeof(ledPins[0]);  // Number of LEDs

int num = 1;  // Device ID or index (used for data offset calculations)

byte touchData[dataLength] = {0};  // Buffer for touch sensor data

const int threshold[] = {8, 10, 10, 9, 10, 9};  // Touch sensitivity thresholds for each sensor

byte sendData[dataLength] = {0};       // Data buffer to send to Unity or other device
byte receivedData[dataLength] = {0};   // Data buffer received from Unity
long intensity[dataLength] = {0};      // Brightness values for each LED

#define TXD2 17 // TX pin for Serial2 (to child device)
#define RXD2 16 // RX pin for Serial2 (from child device)

void setup() {
  Serial.begin(115200);  // Initialize Serial communication with PC (e.g., Unity)
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2); // Initialize Serial2 communication (for child device)

  // Initialize LED PWM channels
  for (int i = 0; i < numLeds; i++) {
    ledcAttach(ledPins[i], 5000, 8);  // Attach PWM to LED pin with 5kHz frequency and 8-bit resolution
  }
}

void updateTouchState(int index, int touchedValue) {
  // Update touch state for main device; 1 if touched (below threshold), 0 otherwise
  sendData[index + dataLength - num * 6] = (touchedValue < threshold[index]) ? 1 : 0;
}

void receiveFromChild() {
  static byte buffer[dataLength];  // Temporary buffer for incoming bytes
  static int index = 0;

  // Read incoming bytes from child device
  while (Serial2.available()) {
    char incomingByte = Serial2.read();

    if (incomingByte == '\n') {  // End of transmission (newline character)
      int copyLength = (index < dataLength) ? index : dataLength;
      for (int i = 0; i < copyLength; i++) {
        sendData[i] = buffer[i];  // Copy received data into sendData
      }
      index = 0;  // Reset buffer index
      break;
    } else {
      if (index < dataLength) {
        buffer[index++] = (byte)incomingByte;  // Store byte in buffer
      } else {
        // Ignore overflow bytes
      }
    }
  }
}

void sendToUnity() {
  // Send data to Unity via Serial (PC connection)
  for (int i = 0; i < dataLength; i++) {
    Serial.write(sendData[i]);
  }
  Serial.write('\n');  // End of transmission marker
}

void receiveFromUnity() {
  static byte buffer[dataLength];  // Temporary buffer for incoming bytes
  static int index = 0;

  // Read incoming bytes from Unity (PC)
  while (Serial.available()) {
    char incomingByte = Serial.read();

    if (incomingByte == '\n') {  // End of transmission
      int copyLength = (index < dataLength) ? index : dataLength;
      for (int i = 0; i < copyLength; i++) {
        receivedData[i] = buffer[i];  // Store received data
      }
      index = 0;  // Reset buffer index
      break;
    } else {
      if (index < dataLength) {
        buffer[index++] = (byte)incomingByte;
      } else {
        // Ignore overflow bytes
      }
    }
  }

  // Convert received data to brightness values
  for (int i = 0; i < dataLength; i++) intensity[i] = (long)receivedData[i];
}

void updateLedBrightness() {
  for (int i = 0; i < numLeds; i++) {
    int touchedValue = touchRead(touchPins[i]);  // Read current touch value
    updateTouchState(i, touchedValue);  // Update touch status
    // Set LED brightness: base intensity + additional if touched
    ledcWrite(ledPins[i], intensity[i + dataLength - num * 6] + sendData[i + dataLength - num * 6]*5);
  }
}

void sendToChild() {
  // Send LED intensity values to child device
  for (int i = 0; i < dataLength; i++) {
    Serial2.write(intensity[i]);
  }
  Serial2.write('\n');  // End of transmission marker
}

void loop() {
  receiveFromChild();     // Read data from child device
  updateLedBrightness();  // Update LED brightness based on touch and received data
  sendToUnity();          // Send data to Unity (PC)
  receiveFromUnity();     // Receive data from Unity (PC)
  sendToChild();          // Send updated LED intensity to child device
}
