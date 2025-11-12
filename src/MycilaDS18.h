// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2025 Mathieu Carbou
 */
#pragma once

#include <esp_idf_version.h>
#include "./esp32-ds18b20/OneWireESP32.h"

#ifdef MYCILA_JSON_SUPPORT
  #include <ArduinoJson.h>
#endif

#include <esp32-hal.h>

#include <mutex>
#include <optional>
#include <utility>

#define MYCILA_DS18_VERSION          "5.1.1"
#define MYCILA_DS18_VERSION_MAJOR    5
#define MYCILA_DS18_VERSION_MINOR    1
#define MYCILA_DS18_VERSION_REVISION 1

// If the temperature is changing from less than 0.3 degrees, we consider it has not changed, to avoid too many updates
// Example:
// [105300][D][MycilaDS18.cpp:75] read(): [DS18] 0x6ba9645509646128 on pin 18: Read success:: 25.19
// [105826][D][MycilaDS18.cpp:75] read(): [DS18] 0x6ba9645509646128 on pin 18: Read success:: 25.12
// [106352][D][MycilaDS18.cpp:75] read(): [DS18] 0x6ba9645509646128 on pin 18: Read success:: 25.19
// [106878][D][MycilaDS18.cpp:75] read(): [DS18] 0x6ba9645509646128 on pin 18: Read success:: 25.12
#ifndef MYCILA_DS18_RELEVANT_TEMPERATURE_CHANGE
  #define MYCILA_DS18_RELEVANT_TEMPERATURE_CHANGE 0.3f
#endif

#define MYCILA_DS18_DS18S20  0x10
#define MYCILA_DS18_DS1822   0x22
#define MYCILA_DS18_DS18B20  0x28
#define MYCILA_DS18_DS1825   0x3B
#define MYCILA_DS18_DS28EA00 0x42

namespace Mycila {
  // callback signature for temperature reads.
  // "changed" will be true if the temperature has changed by more than "MYCILA_DS18_RELEVANT_TEMPERATURE_CHANGE" degrees
  typedef std::function<void(float temperature, bool changed)> DS18ChangeCallback;
  class DS18 {
    public:
      ~DS18() { end(); }

      void setExpirationDelay(uint32_t seconds) { _expirationDelay = seconds; }
      uint32_t getExpirationDelay() const { return _expirationDelay; }

      void listen(DS18ChangeCallback callback) { _callback = std::move(callback); }

      void begin(const int8_t pin, uint8_t maxSearchCount = 10);
      void begin(const int8_t pin, uint64_t address);
      void begin(OneWire32* oneWire, uint64_t address);
      void end();

      const char* getModel() const {
        switch (_deviceAddress & 0xFF) {
          case MYCILA_DS18_DS18S20:
            return "DS18S20";
          case MYCILA_DS18_DS1822:
            return "DS1822";
          case MYCILA_DS18_DS18B20:
            return "DS18B20";
          case MYCILA_DS18_DS1825:
            return "DS1825";
          case MYCILA_DS18_DS28EA00:
            return "DS28EA00";
          default:
            return "Unknown";
        }
      }

      // Read the temperature from the sensor (async and non-blocking)
      // Check if a reading is available and if so, process it and request another reading
      // If no reading available or reading is invalid, returns false.
      // Otherwise returns true, which means that a new reading was available,
      // but it does not mean that the temperature has changed: it can still be the same
      // This method can be called in the loop
      bool read();

      gpio_num_t getPin() const { return _pin; };
      uint64_t getAddress() const { return _deviceAddress; };
      bool isEnabled() const { return _enabled; }

      // Get the last time when the temperature was read
      uint32_t getLastTime() const { return _lastTime; }

      // Get the elapsed time since the last reading
      uint32_t getElapsedTime() const { return _enabled ? millis() - _lastTime : 0; }

      // Check if the last reading has expired
      bool isExpired() const { return _expirationDelay > 0 && (getElapsedTime() >= _expirationDelay * 1000); }

      // Check if the last reading is valid and present
      bool isValid() const { return _enabled && _lastTime > 0 && !isExpired(); }

      // Get the temperature in Celsius
      std::optional<float> getTemperature() const {
        if (isValid()) {
          return _temperature;
        }
        return std::nullopt;
      }

      OneWire32* getOneWire() const { return _oneWire; }

#ifdef MYCILA_JSON_SUPPORT
      void toJson(const JsonObject& root) const;
#endif

    private:
      OneWire32* _oneWire = nullptr;
      bool _ownOneWire = true;
      uint64_t _deviceAddress = 0;
      gpio_num_t _pin = GPIO_NUM_NC;
      bool _enabled = false;
      const char* _name = "Unknown";
      float _temperature = 0;
      uint32_t _lastTime = 0;
      uint32_t _expirationDelay = 0;
      DS18ChangeCallback _callback = nullptr;
      std::mutex _mutex;
  };
} // namespace Mycila
