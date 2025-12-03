# MycilaDS18

[![Latest Release](https://img.shields.io/github/release/mathieucarbou/MycilaDS18.svg)](https://GitHub.com/mathieucarbou/MycilaDS18/releases/)
[![PlatformIO Registry](https://badges.registry.platformio.org/packages/mathieucarbou/library/MycilaDS18.svg)](https://registry.platformio.org/libraries/mathieucarbou/MycilaDS18)

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Contributor Covenant](https://img.shields.io/badge/Contributor%20Covenant-2.1-4baaaa.svg)](code_of_conduct.md)

[![Build](https://github.com/mathieucarbou/MycilaDS18/actions/workflows/ci.yml/badge.svg)](https://github.com/mathieucarbou/MycilaDS18/actions/workflows/ci.yml)
[![GitHub latest commit](https://badgen.net/github/last-commit/mathieucarbou/MycilaDS18)](https://GitHub.com/mathieucarbou/MycilaDS18/commit/)
[![Gitpod Ready-to-Code](https://img.shields.io/badge/Gitpod-Ready--to--Code-blue?logo=gitpod)](https://gitpod.io/#https://github.com/mathieucarbou/MycilaDS18)

ESP32 / Arduino Library for Dallas / Maxim DS18 sensors using RMT peripheral

## Features

- ⚡ Based on ESP32 RMT peripheral implementation from [https://github.com/junkfix/esp32-ds18b20](https://github.com/junkfix/esp32-ds18b20)
- ✅ Compatible with Arduino 2 and 3
- 🌡️ Supports multiple DS18 sensor models:
  - DS18S20
  - DS1822
  - DS18B20
  - DS1825
  - DS28EA00
- 🚀 Non-blocking temperature readings
- 🔔 Callback support with change detection
- ⏱️ Value expiration support
- 🔗 Multiple sensors on same bus support
- 📊 Optional JSON output support (with ArduinoJson)
- 🛡️ `std::optional` for safe temperature value handling

## Installation

### PlatformIO

```ini
lib_deps = mathieucarbou/MycilaDS18
```

### Arduino IDE

Install from the Arduino Library Manager or download from [GitHub releases](https://github.com/mathieucarbou/MycilaDS18/releases).

## API Overview

### Initialization

```c++
// Auto-search for first device on pin
void begin(const int8_t pin, uint8_t maxSearchCount = 10);

// Use specific device address on pin
void begin(const int8_t pin, uint64_t address);

// Use existing OneWire32 instance with specific address
void begin(OneWire32* oneWire, uint64_t address);

// Stop sensor
void end();
```

### Reading Temperature

```c++
// Non-blocking read - returns true if new reading is available
bool read();

// Get temperature as optional<float> (returns nullopt if invalid/expired)
std::optional<float> getTemperature() const;

// Check if sensor is enabled and configured
bool isEnabled() const;

// Check if last reading is valid (not expired)
bool isValid() const;

// Check if last reading has expired
bool isExpired() const;
```

### Configuration

```c++
// Set expiration delay in seconds (0 = no expiration)
void setExpirationDelay(uint32_t seconds);
uint32_t getExpirationDelay() const;

// Register callback for temperature changes
// "changed" parameter indicates if temperature changed by > 0.3°C
void listen(DS18ChangeCallback callback);
```

### Information

```c++
// Get sensor information
gpio_num_t getPin() const;
uint64_t getAddress() const;
const char* getModel() const;

// Get timing information
uint32_t getLastTime() const;      // Last read time in millis
uint32_t getElapsedTime() const;   // Time since last reading

// Get OneWire instance
OneWire32* getOneWire() const;

// Export to JSON (requires MYCILA_JSON_SUPPORT)
void toJson(const JsonObject& root) const;
```

## Usage Examples

### Basic Usage

```c++
#include <MycilaDS18.h>

Mycila::DS18 temp;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    continue;

  // Auto-detect first sensor on GPIO 18
  temp.begin(18);

  // Listen to temperature changes in a callback
  // "changed" is true if temperature changed by > 0.3°C
  temp.listen([](float temperature, bool changed) {
    if (changed) {
      Serial.printf("Temperature changed: %.2f°C\n", temperature);
    }
  });
}

void loop() {
  // Non-blocking read
  temp.read();
  delay(2000);
}
```

### Using Specific Sensor Address

```c++
#include <MycilaDS18.h>

Mycila::DS18 temp;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    continue;

  // Connect to specific sensor by address
  temp.begin(18, 0x983cee0457ea9f28ULL);

  temp.listen([](float temperature, bool changed) {
    Serial.printf("Temperature: %.2f°C\n", temperature);
  });
}

void loop() {
  temp.read();
  delay(2000);
}
```

### Multiple Sensors on Same Bus

```c++
#include <MycilaDS18.h>

OneWire32 oneWire(18);
Mycila::DS18 temp1;
Mycila::DS18 temp2;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    continue;

  // Search for devices
  uint64_t addresses[2] = {0};
  size_t found = 0;

  Serial.println("Searching for DS18 sensors...");
  for (int i = 0; i < 10 && found < 2; i++) {
    found = oneWire.search(addresses, 2);
    vTaskDelay(portTICK_PERIOD_MS);
  }

  for (size_t i = 0; i < found; i++) {
    Serial.printf("Found device %u: %016llx\n", i + 1, addresses[i]);
  }

  if (found > 0) {
    temp1.begin(&oneWire, addresses[0]);
    temp1.listen([](float temperature, bool changed) {
      Serial.printf("Temperature 1: %.2f°C\n", temperature);
    });
  }

  if (found > 1) {
    temp2.begin(&oneWire, addresses[1]);
    temp2.listen([](float temperature, bool changed) {
      Serial.printf("Temperature 2: %.2f°C\n", temperature);
    });
  }
}

void loop() {
  temp1.read();
  temp2.read();
  delay(2000);
}
```

### JSON Output

```c++
#include <ArduinoJson.h>
#include <MycilaDS18.h>

Mycila::DS18 temp;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    continue;

  temp.begin(18);

  temp.listen([](float temperature, bool changed) {
    JsonDocument doc;
    temp.toJson(doc.to<JsonObject>());
    serializeJson(doc, Serial);
    Serial.println();
  });
}

void loop() {
  temp.read();
  delay(2000);
}
```

### Value Expiration

```c++
#include <MycilaDS18.h>

Mycila::DS18 temp;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    continue;

  temp.begin(18);

  // Temperature readings expire after 30 seconds
  temp.setExpirationDelay(30);
}

void loop() {
  temp.read();

  if (temp.isValid()) {
    float t = temp.getTemperature().value_or(0);
    Serial.printf("Valid temperature: %.2f°C\n", t);
  } else if (temp.isExpired()) {
    Serial.println("Temperature reading expired!");
  }

  delay(2000);
}
```

## Advanced Usage

### Safe Temperature Access with std::optional

```c++
std::optional<float> tempValue = temp.getTemperature();
if (tempValue.has_value()) {
  Serial.printf("Temperature: %.2f°C\n", tempValue.value());
} else {
  Serial.println("No valid temperature available");
}
```

### Change Detection Threshold

The library considers a temperature change significant if it differs by more than 0.3°C from the previous reading. This threshold can be customized by defining `MYCILA_DS18_RELEVANT_TEMPERATURE_CHANGE` before including the library:

```c++
#define MYCILA_DS18_RELEVANT_TEMPERATURE_CHANGE 0.5f
#include <MycilaDS18.h>
```

## Examples

The library includes several examples in the `examples/` folder:

- **Search**: Basic usage with auto-detection
- **SetAddress**: Using a specific sensor address
- **MultipleDS18**: Multiple sensors on the same bus
- **Json**: JSON output support

## License

MIT License - see [LICENSE](LICENSE) file for details
