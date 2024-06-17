/*
https://github.com/junkfix/esp32-ds18b20
*/

#pragma once

#include <esp_idf_version.h>
#if ESP_IDF_VERSION_MAJOR >= 5

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/rmt_tx.h"
#include "driver/rmt_rx.h"

IRAM_ATTR bool owrxdone(rmt_channel_handle_t ch, const rmt_rx_done_event_data_t *edata, void *udata);

class OneWire32 {
	private:
		gpio_num_t owpin;
		rmt_channel_handle_t owtx;
		rmt_channel_handle_t owrx;
		rmt_encoder_handle_t owcenc;
		rmt_encoder_handle_t owbenc;
		rmt_symbol_word_t *owbuf;
		QueueHandle_t owqueue;
		uint8_t drv = 0;
	public:
		OneWire32(uint8_t pin);
		~OneWire32();
		bool reset();
		void request();
		uint8_t getTemp(uint64_t &addr, float &temp);
		uint8_t search(uint64_t *addresses, uint8_t total);
		bool read(uint8_t &data, uint8_t len = 8);
		bool write(const uint8_t data, uint8_t len = 8);
};

#endif // ESP_IDF_VERSION_MAJOR >= 5
