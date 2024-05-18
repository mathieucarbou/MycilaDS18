// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#pragma once

#include "./DallasTemperature.h"
#include "./OneWire.h"

#ifdef MYCILA_JSON_SUPPORT
#include <ArduinoJson.h>
#endif

#define MYCILA_DS18_VERSION "1.0.0"
#define MYCILA_DS18_VERSION_MAJOR 1
#define MYCILA_DS18_VERSION_MINOR 0
#define MYCILA_DS18_VERSION_REVISION 0

// If the temperature is changing from less than 0.3 degrees, we consider it has not changed
#ifndef MYCILA_DS18_RELEVANT_TEMPERATURE_CHANGE
#define MYCILA_DS18_RELEVANT_TEMPERATURE_CHANGE 0.3
#endif

namespace Mycila {
  typedef std::function<void(float temperature)> DS18ChangeCallback;
  class DS18 {
    public:
      ~DS18() { end(); }

      void setExpirationDelay(uint32_t seconds) { _expirationDelay = seconds; }
      uint32_t getExpirationDelay() const { return _expirationDelay; }

      void listen(DS18ChangeCallback callback) { _callback = callback; }

      void begin(const int8_t pin);
      void end();

      // Read the temperature from the sensor (async and non-blocking)
      // Check if a reading is available and if so, process it and request another reading
      // If no reading available or reading is invalid, returns false.
      // Otherwise returns true, which means that a new reading was available,
      // but it does not mean that the temperature has changed: it can still be the same
      // This method can be called in the loop
      bool read();

      gpio_num_t getPin() const { return _pin; };
      bool isEnabled() const { return _enabled; }
      float getLastTemperature() const { return _temperature; }
      uint32_t getLastTime() const { return _lastTime; }
      uint32_t getElapsedTime() const { return millis() - _lastTime; }
      bool isExpired() const { return _expirationDelay > 0 && (getElapsedTime() >= _expirationDelay * 1000); }

#ifdef MYCILA_JSON_SUPPORT
      void toJson(const JsonObject& root) const;
#endif

    private:
      OneWire _oneWire;
      DallasTemperature _dallas;
      DeviceAddress _deviceAddress;
      gpio_num_t _pin = GPIO_NUM_NC;
      bool _enabled = false;
      float _temperature = 0;
      uint32_t _lastTime = 0;
      uint32_t _expirationDelay = 0;
      DS18ChangeCallback _callback = nullptr;
  };
} // namespace Mycila
