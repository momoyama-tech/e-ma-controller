const int dataLength = 28;    // タッチセンサとLEDの合計
const int touchPins[] = {T4, T5, T6, T7, T8, T9};  // 子機タッチピン
const int ledPins[] = {5, 18, 19, 21, 22, 23};    // LEDピン
const int numLeds = sizeof(ledPins) / sizeof(ledPins[0]);

int num = 3;

byte touchData[dataLength] = {0};  // タッチデータ（12の要素）

const int threshold[] = {11, 10, 11, 11, 11, 11};// タッチしきい値

byte sendData[dataLength] = {0};    // 送信データ
byte receivedData[dataLength] = {0};  // 受信データ
long intensity[dataLength] = {0};      // 各LEDの明るさ

HardwareSerial mySerial(1);
const int Rx1Pin = 25;
const int Tx1Pin = 26;

#define TXD2 17 // GPIO17 (TX) 
#define RXD2 16 // GPIO16 (RX)

void setup() {
  Serial.begin(115200);  // PCとのシリアル通信
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2); // Serial2を初期化
  mySerial.begin(115200, SERIAL_8N1, Rx1Pin, Tx1Pin);

  // LEDピンの初期化
  for (int i = 0; i < numLeds; i++) {
    ledcAttach(ledPins[i], 5000, 8);
  }
}

void updateTouchState(int index, int touchedValue) {
  sendData[index + dataLength - num * 6] = (touchedValue < threshold[index]) ? 1 : 0; // 後ろ6つに子機のタッチ情報を格納
}

void receiveFromChild() {
  static byte buffer[dataLength];
  static int index = 0;

  while (Serial2.available()) {
    char incomingByte = Serial2.read();

    if (incomingByte == '\n') {  // 改行で受信完了
      int copyLength = (index < dataLength) ? index : dataLength;
      for (int i = 0; i < copyLength; i++) {
        sendData[i] = buffer[i];
      }
      index = 0;  // バッファリセット
      break;
    } else {
      if (index < dataLength) {
        buffer[index++] = (byte)incomingByte;
      } else {
        // バッファオーバーフロー対策（無視する）
      }
    }
  }
}

// シリアルデータを親機に送信する関数
void sendToParent() {
  for (int i = 0; i < dataLength; i++) {
    Serial2.write(sendData[i]);
  }
  Serial2.write('\n');  // 終端マーカー
}

// シリアルデータを受信する関数
void receiveFromParent() {
  static byte buffer[dataLength];
  static int index = 0;

  while (mySerial.available()) {
    char incomingByte = mySerial.read();

    if (incomingByte == '\n') {  // 改行で受信完了
      int copyLength = (index < dataLength) ? index : dataLength;
      for (int i = 0; i < copyLength; i++) {
        receivedData[i] = buffer[i];
      }
      index = 0;  // バッファリセット
      break;
    } else {
      if (index < dataLength) {
        buffer[index++] = (byte)incomingByte;
      } else {
        // バッファオーバーフロー対策（無視する）
      }
    }
  }
  for (int i = 0; i < dataLength; i++) intensity[i] = (long)receivedData[i];
}


void updateLedBrightness() {
  for (int i = 0; i < numLeds; i++) {
    int touchedValue = touchRead(touchPins[i]);
    updateTouchState(i, touchedValue);
    ledcWrite(ledPins[i], intensity[i + dataLength - num * 6] + sendData[i + dataLength - num * 6]*5  );  // LED強度を設定 
  }
}

void loop() {
  receiveFromChild(); // 子機から受信
  updateLedBrightness(); // LEDの明るさ更新
  sendToParent();    // 親機に送信
  receiveFromParent(); // 親機から受信
}

