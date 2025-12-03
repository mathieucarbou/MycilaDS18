// SPDX-License-Identifier: MIT
/*
 * Copyright (C) 2023-2024 Mathieu Carbou
 */
#include <MycilaDS18.h>

#define TAG "DS18"

#ifndef GPIO_IS_VALID_OUTPUT_GPIO
  #define GPIO_IS_VALID_OUTPUT_GPIO(gpio_num) ((gpio_num >= 0) && \
                                               (((1ULL << (gpio_num)) & SOC_GPIO_VALID_OUTPUT_GPIO_MASK) != 0))
#endif

void Mycila::DS18::begin(const int8_t pin, uint8_t maxSearchCount) {
  if (_enabled)
    return;

  if (GPIO_IS_VALID_OUTPUT_GPIO(pin)) {
    _pin = (gpio_num_t)pin;
  } else {
    ESP_LOGE(TAG, "Disable DS18 Sensor: Invalid pin: %" PRId8, pin);
    _pin = GPIO_NUM_NC;
    return;
  }

  _oneWire = new OneWire32(_pin);

  ESP_LOGI(TAG, "Searching for DS18 sensor on pin: %" PRId8 "...", pin);
  while (maxSearchCount-- > 0 && !_oneWire->search(&_deviceAddress, 1)) {
    vTaskDelay(portTICK_PERIOD_MS);
  }

  if (!_deviceAddress) {
    ESP_LOGE(TAG, "No DS18 sensor found on pin: %" PRId8, pin);
    return;
  }

  _name = getModel();

  ESP_LOGI(TAG, "Found %s sensor at address 0x%llx on pin: %" PRId8 " (remaining search count: %d)", _name, _deviceAddress, _pin, maxSearchCount);

  _oneWire->request(_deviceAddress);

  ESP_LOGI(TAG, "%s 0x%llx @ pin %d enabled!", _name, _deviceAddress, _pin);
  _enabled = true;
}

void Mycila::DS18::begin(const int8_t pin, uint64_t address) {
  if (_enabled)
    return;

  if (GPIO_IS_VALID_OUTPUT_GPIO(pin)) {
    _pin = (gpio_num_t)pin;
  } else {
    ESP_LOGE(TAG, "Disable DS18 Sensor: Invalid pin: %" PRId8, pin);
    _pin = GPIO_NUM_NC;
    return;
  }

  _deviceAddress = address;

  if (!_deviceAddress) {
    ESP_LOGE(TAG, "Invalid DS18 sensor address: 0x%llx", address);
    return;
  }

  _oneWire = new OneWire32(_pin);
  _name = getModel();
  _oneWire->request(_deviceAddress);

  ESP_LOGI(TAG, "%s 0x%llx @ pin %d enabled!", _name, _deviceAddress, _pin);
  _enabled = true;
}

void Mycila::DS18::begin(OneWire32* oneWire, uint64_t address) {
  _deviceAddress = address;

  if (!_deviceAddress) {
    ESP_LOGE(TAG, "Invalid DS18 sensor address: 0x%llx", address);
    return;
  }

  _oneWire = oneWire;
  _pin = oneWire->pin();
  _ownOneWire = false;
  _name = getModel();
  _oneWire->request(_deviceAddress);

  ESP_LOGI(TAG, "%s 0x%llx @ pin %d enabled!", _name, _deviceAddress, _pin);
  _enabled = true;
}

void Mycila::DS18::end() {
  if (_enabled) {
    std::lock_guard<std::mutex> lock(_mutex);
    _enabled = false;
    if (_ownOneWire && _oneWire) {
      delete _oneWire;
      _oneWire = nullptr;
      // Give some time for RMT channels to be properly released
      vTaskDelay(pdMS_TO_TICKS(10));
    }
    _temperature = 0;
    _lastTime = 0;
    _pin = GPIO_NUM_NC;
    _deviceAddress = 0;
    ESP_LOGI(TAG, "%s 0x%llx @ pin %d disabled!", _name, _deviceAddress, _pin);
  }
}

bool Mycila::DS18::read() {
  std::lock_guard<std::mutex> lock(_mutex);

  if (!_enabled)
    return false;

  float read;
  OneWire32::Result result = _oneWire->getTemp(_deviceAddress, read);

  // request new reading
  _oneWire->request(_deviceAddress);

  // process data when no error
  if (result != OneWire32::Result::OK) {
    switch (result) {
      case OneWire32::Result::OK:
        break;
      case OneWire32::Result::CRC:
        ESP_LOGW(TAG, "%s 0x%llx @ pin %d: CRC error", _name, _deviceAddress, _pin);
        return false;
      case OneWire32::Result::BAD_DATA:
        ESP_LOGW(TAG, "%s 0x%llx @ pin %d: Bad data", _name, _deviceAddress, _pin);
        return false;
      case OneWire32::Result::TIMEOUT:
        ESP_LOGW(TAG, "%s 0x%llx @ pin %d: Timeout", _name, _deviceAddress, _pin);
        return false;
      case OneWire32::Result::DRIVER:
        ESP_LOGW(TAG, "%s 0x%llx @ pin %d: Driver not initialized", _name, _deviceAddress, _pin);
        return false;
    }
    return false;
  }

  // discard any invalid read
  if (std::isnan(read))
    return false;

  // read is valid, record the time
  _lastTime = millis();

  const bool changed = std::abs(read - _temperature) > _threshold || !isValid();

  if (changed) {
    _temperature = read;
    ESP_LOGD(TAG, "%s 0x%llx @ pin %d: %f °C", _name, _deviceAddress, _pin, read);
  }

  if (_callback)
    _callback(_temperature, changed);

  return true;
}

#ifdef MYCILA_JSON_SUPPORT
void Mycila::DS18::toJson(const JsonObject& root) const {
  root["enabled"] = _enabled;
  root["model"] = getModel();
  root["address"] = _deviceAddress;
  root["elapsed"] = getElapsedTime();
  root["expired"] = isExpired();
  root["temp"] = getTemperature().value_or(0);
  root["time"] = _lastTime;
  root["valid"] = isValid();
}
#endif
