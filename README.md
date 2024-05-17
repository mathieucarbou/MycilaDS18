# MycilaDS18

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Continuous Integration](https://github.com/mathieucarbou/MycilaDS18/actions/workflows/ci.yml/badge.svg)](https://github.com/mathieucarbou/MycilaDS18/actions/workflows/ci.yml)
[![PlatformIO Registry](https://badges.registry.platformio.org/packages/mathieucarbou/library/MycilaDS18.svg)](https://registry.platformio.org/libraries/mathieucarbou/MycilaDS18)

ESP32 / Arduino Library for Dallas / Maxim Temperature Integrated Circuits

## Usage

```c++
#include <Arduino.h>
#include <MycilaDS18.h>

Mycila::DS18 temp;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    continue;

  temp.begin(18);

  temp.listen([](float temperature) {
    Serial.printf("Temperature: %.2f\n", temperature);
  });
}

void loop() {
  temp.read();
  delay(500);
}
```
