// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <MycilaDS18.h>

#include <Wire.h>

#ifdef MYCILA_LOGGER_SUPPORT
#include <MycilaLogger.h>
extern Mycila::Logger logger;
#define LOGD(tag, format, ...) logger.debug(tag, format, ##__VA_ARGS__)
#define LOGI(tag, format, ...) logger.info(tag, format, ##__VA_ARGS__)
#define LOGW(tag, format, ...) logger.warn(tag, format, ##__VA_ARGS__)
#define LOGE(tag, format, ...) logger.error(tag, format, ##__VA_ARGS__)
#else
#define LOGD(tag, format, ...) ESP_LOGD(tag, format, ##__VA_ARGS__)
#define LOGI(tag, format, ...) ESP_LOGI(tag, format, ##__VA_ARGS__)
#define LOGW(tag, format, ...) ESP_LOGW(tag, format, ##__VA_ARGS__)
#define LOGE(tag, format, ...) ESP_LOGE(tag, format, ##__VA_ARGS__)
#endif

#define TAG "DS18"

#ifndef GPIO_IS_VALID_OUTPUT_GPIO
#define GPIO_IS_VALID_OUTPUT_GPIO(gpio_num) ((gpio_num >= 0) && \
                                             (((1ULL << (gpio_num)) & SOC_GPIO_VALID_OUTPUT_GPIO_MASK) != 0))
#endif

void Mycila::DS18::begin(const int8_t pin) {
  if (_enabled)
    return;

  if (GPIO_IS_VALID_OUTPUT_GPIO(pin)) {
    _pin = (gpio_num_t)pin;
  } else {
    LOGE(TAG, "Disable Temperature Sensor: Invalid pin: %" PRId8, pin);
    _pin = GPIO_NUM_NC;
    return;
  }

  _oneWire.begin(_pin);

  _dallas.setOneWire(&_oneWire);
  _dallas.setWaitForConversion(false);
  _dallas.begin();

  if (_dallas.getDS18Count() == 0 || !_dallas.getAddress(_deviceAddress, 0)) {
    LOGE(TAG, "No DS18B20 sensor found on pin: %" PRId8, pin);
    return;
  }

  _dallas.requestTemperaturesByAddress(_deviceAddress);
  LOGI(TAG, "Enabled Temperature Sensor on pin: %" PRId8, pin);
  _enabled = true;
}

void Mycila::DS18::end() {
  if (_enabled) {
    _enabled = false;
    _temperature = 0;
    _lastTime = 0;
    _pin = GPIO_NUM_NC;
    LOGI(TAG, "Disabled Temperature Sensor on pin: %" PRId8, _pin);
  }
}

bool Mycila::DS18::read() {
  if (!_enabled)
    return 0;

  float read = _dallas.getTempC(_deviceAddress);

  // invalid read or not ready yet
  if (isnan(read) || read <= 0)
    return false;

  read = round(read * 100) / 100;
  _lastTime = millis();
  _dallas.requestTemperaturesByAddress(_deviceAddress);

  if (abs(read - _temperature) >= MYCILA_DS18_RELEVANT_TEMPERATURE_CHANGE || isExpired()) {
    _temperature = read;
    if (_callback)
      _callback(_temperature);
  }

  return true;
}

#ifdef MYCILA_JSON_SUPPORT
void Mycila::DS18::toJson(const JsonObject& root) const {
  root["elapsed_time"] = getElapsedTime();
  root["enabled"] = _enabled;
  root["expired"] = isExpired();
  root["last_temp"] = _temperature;
  root["last_time"] = _lastTime;
}
#endif
