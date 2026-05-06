#ifdef min
  #undef min
#endif

#ifdef max
  #undef max
#endif

#include <Wire.h>
#include "BMD31M090.h"
#include "Bitmap.h"
#include <algorithm> // 引入 std::sort

#define OLED_WIDTH    128
#define OLED_HEIGHT   64
#define OLED_ADDRESS  0x3C
#define NUM_SENSORS   3
#define MAX_VALID_MM  4.00f   // 最大有效值阈值
#define D0_PIN        7        // 修改D0引脚定义接收信号
#define MEDIAN_FILTER_SIZE 5   // 中值滤波窗口大小
#define DEBOUNCE_DELAY  50    // D0引脚去抖动延时 (ms)

BMD31M090 oled(OLED_WIDTH, OLED_HEIGHT, &Wire1);

// 传感器配置结构体
struct Sensor {
  int clkPin;
  int daPin;
  const int clkHighThreshold;
  const int daHighThreshold;
  const int daLowThreshold;

  volatile uint32_t data;
  volatile int bitCount;
  volatile bool dataReady;
  bool lastCLKHigh;
  float lastValidValue;

  float medianBuffer[MEDIAN_FILTER_SIZE];
  int medianIndex;
  float filteredValue;
};

Sensor sensors[NUM_SENSORS] = {
  {A0, A1, 400, 450, 10, 0, 0, false, false, 0.0f, {0}, 0, 0.0f},
  {A2, A3, 400, 450, 10, 0, 0, false, false, 0.0f, {0}, 0, 0.0f},
  {A4, A5, 400, 450, 10, 0, 0, false, false, 0.0f, {0}, 0, 0.0f}
};

bool d0PinState = false;
bool lastD0PinState = false;
unsigned long lastDebounceTime = 0;
bool d0PinHighDetected = false;

void setup() {
  Serial.begin(115200);
  pinMode(D0_PIN, INPUT_PULLDOWN);
  oled.begin(OLED_ADDRESS);
  oled.setFont(FontTable_8X16);
  oled.clearDisplay();
  oled.drawString(0, 0, (u8*)"measure:");
  oled.drawString(10, 2, (u8*)"S1:");
  oled.drawString(10, 4, (u8*)"S2:");
  oled.drawString(10, 6, (u8*)"S3:");

  for (auto &sensor : sensors) {
    pinMode(sensor.clkPin, INPUT);
    pinMode(sensor.daPin, INPUT);
    sensor.bitCount = 0;
    sensor.dataReady = false;
    sensor.lastCLKHigh = false;
    sensor.data = 0;
    sensor.lastValidValue = 0.0f;
    sensor.medianIndex = 0;
    for (int i = 0; i < MEDIAN_FILTER_SIZE; ++i) {
      sensor.medianBuffer[i] = 0.0f;
    }
    sensor.filteredValue = 0.0f;
  }
}

void processSensor(Sensor &sensor) {
  bool clkHigh = analogRead(sensor.clkPin) > sensor.clkHighThreshold;

  if (!sensor.lastCLKHigh && clkHigh) {
    int daValue = analogRead(sensor.daPin);
    int bit = (daValue > sensor.daHighThreshold) ? 1 :
              (daValue < sensor.daLowThreshold) ? 0 : -1;

    if (bit != -1) {
      sensor.data = (sensor.data >> 1) | (bit << 23);

      if (++sensor.bitCount >= 24) {
        noInterrupts();
        sensor.dataReady = true;
        sensor.bitCount = 0;
        interrupts();
      }
    }
  }
  sensor.lastCLKHigh = clkHigh;
}

// 中值滤波函数
float medianFilter(Sensor &sensor, float newValue) {
  sensor.medianBuffer[sensor.medianIndex] = newValue;
  sensor.medianIndex = (sensor.medianIndex + 1) % MEDIAN_FILTER_SIZE;
  float tempBuffer[MEDIAN_FILTER_SIZE];
  std::copy(sensor.medianBuffer, sensor.medianBuffer + MEDIAN_FILTER_SIZE, tempBuffer);
  std::sort(tempBuffer, tempBuffer + MEDIAN_FILTER_SIZE);
  return tempBuffer[MEDIAN_FILTER_SIZE / 2];
}

void sendHexData() {
  uint8_t num[9];
  bool allReady = true;

  for (auto &sensor : sensors) {
    if (!sensor.dataReady) {
      allReady = false;
      break;
    }
  }

  if (allReady) {
    num[0] = 0xAA;

    int currentD0State = digitalRead(D0_PIN);
    if (currentD0State != lastD0PinState) {
      lastDebounceTime = millis();
    }
    if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
      if (currentD0State != d0PinState) {
        d0PinState = currentD0State;
        if (d0PinState == HIGH) {
          d0PinHighDetected = true;
        }
      }
    }
    lastD0PinState = currentD0State;

    num[1] = d0PinHighDetected ? 1 : 0;
    num[2] = (sensors[0].data & 0xFFFF) >> 8;
    num[3] = (sensors[0].data & 0xFF);
    num[4] = (sensors[1].data & 0xFFFF) >> 8;
    num[5] = (sensors[1].data & 0xFF);
    num[6] = (sensors[2].data & 0xFFFF) >> 8;
    num[7] = (sensors[2].data & 0xFF);
    num[8] = 0xBB;

    for (int i = 0; i < 9; i++) {
      Serial.write(num[i]);
    }

    for (auto &sensor : sensors) {
      sensor.dataReady = false;
      sensor.data = 0;
    }
    d0PinHighDetected = false;
  }
}

void updateDisplay() {
  static char displayText[NUM_SENSORS][16];
  bool allReady = true;

  for (auto &sensor : sensors) {
    if (!sensor.dataReady) {
      allReady = false;
      break;
    }
  }

  if (allReady) {
    for (int i = 0; i < NUM_SENSORS; ++i) {
      float mm = sensors[i].data / 100.0f;

      if (mm <= MAX_VALID_MM) {
        sensors[i].lastValidValue = mm;
        sensors[i].filteredValue = medianFilter(sensors[i], mm); // 应用中值滤波
        dtostrf(sensors[i].filteredValue, 5, 2, displayText[i]);
      } else {
        dtostrf(sensors[i].lastValidValue, 5, 2, displayText[i]);
      }

      oled.drawString(80, 2 + i * 2, (u8*)" mm   ");
      oled.drawString(40, 2 + i * 2, (u8*)displayText[i]);
    }
    sendHexData();
  }
}

void loop() {
  for (auto &sensor : sensors) {
    processSensor(sensor);
  }
  updateDisplay();
}    