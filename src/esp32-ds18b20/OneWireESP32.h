/*
https://github.com/junkfix/esp32-ds18b20
*/

#pragma once

#include "driver/rmt_rx.h"
#include "driver/rmt_tx.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "sdkconfig.h"

class OneWire32 {
  private:
    gpio_num_t owpin;
    rmt_channel_handle_t owtx;
    rmt_channel_handle_t owrx;
    rmt_encoder_handle_t owcenc;
    rmt_encoder_handle_t owbenc;
    rmt_symbol_word_t* owbuf;
    QueueHandle_t owqueue;
    uint8_t drv = 0;

  public:
    enum Result {
      OK = 0,
      CRC = 1,
      BAD_DATA = 2,
      TIMEOUT = 3,
      DRIVER = 4
    };

    OneWire32(uint8_t pin);
    ~OneWire32();
    gpio_num_t pin() const { return owpin; }
    bool reset();
    void request();
    void request(uint64_t& addr);
    Result getTemp(uint64_t& addr, float& temp);
    uint8_t search(uint64_t* addresses, uint8_t total);
    bool read(uint8_t& data, uint8_t len = 8);
    bool write(const uint8_t data, uint8_t len = 8);
};
