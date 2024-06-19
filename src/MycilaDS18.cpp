// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou and others
 */
#include <MycilaDS18.h>

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

#define TAG "DS18B20"

#ifndef GPIO_IS_VALID_OUTPUT_GPIO
  #define GPIO_IS_VALID_OUTPUT_GPIO(gpio_num) ((gpio_num >= 0) && \
                                               (((1ULL << (gpio_num)) & SOC_GPIO_VALID_OUTPUT_GPIO_MASK) != 0))
#endif

static const char* err_desc[] = {"", "CRC ERROR", "BAD DATA", "TIMEOUT", "DRIVER NOT INITIALIZED"};

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

  _oneWire = new OneWire32(_pin);
  uint32_t start = millis();
  LOGD(TAG, "Searching for DS18B20 sensor on pin: %" PRId8 "...", pin);
  while (!_oneWire->search(&_deviceAddress, 1) && millis() - start < 2000) {
    delay(10);
  }
  if (!_deviceAddress) {
    LOGE(TAG, "No DS18B20 sensor found on pin: %" PRId8, pin);
    return;
  }

  _oneWire->request();

  LOGI(TAG, "0x%llx on pin %d enabled!", _deviceAddress, _pin);
  _enabled = true;
}

void Mycila::DS18::end() {
  if (_enabled) {
    LOGI(TAG, "0x%llx on pin %d disabled!", _deviceAddress, _pin);
    _enabled = false;
    _temperature = MYCILA_DS18_INVALID_TEMPERATURE;
    _lastTime = 0;
    _pin = GPIO_NUM_NC;
    _deviceAddress = 0;
    delete _oneWire;
  }
}

bool Mycila::DS18::read() {
  if (!_enabled)
    return false;

  float read;
  uint8_t err = _oneWire->getTemp(_deviceAddress, read);
  if (err) {
    LOGW(TAG, "0x%llx on pin %d: Read error: %s", _deviceAddress, _pin, err_desc[err]);
  } else {
    LOGD(TAG, "0x%llx on pin %d: Read success: %.2f", _deviceAddress, _pin, read);
  }

  // request new reading
  _oneWire->request();

  // process data when no error
  if (err) {
    return false;
  }

  // discard any invalid read
  if (isnan(read) || read <= 0)
    return false;

  // read is valid, record the time
  _lastTime = millis();

  // make it on 2 decimals
  read = round(read * 100) / 100;

  if (abs(read - _temperature) >= MYCILA_DS18_RELEVANT_TEMPERATURE_CHANGE || !isValid()) {
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
