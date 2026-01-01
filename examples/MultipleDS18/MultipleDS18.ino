// SPDX-License-Identifier: MIT
/*
 * Copyright (C) Mathieu Carbou
 */
#include <Arduino.h>
#include <MycilaDS18.h>

OneWire32 oneWire(18);
Mycila::DS18 temp1;
Mycila::DS18 temp2;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    continue;

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

  if (found == 0) {
    Serial.println("No devices found!");
    return;
  }

  temp1.begin(&oneWire, addresses[0]);
  temp1.listen([](float temperature, bool changed) {
    Serial.printf("Temperature 1: %.2f\n", temperature);
  });

  if (found < 2) {
    Serial.println("Only one device found!");
    return;
  }

  temp2.begin(&oneWire, addresses[1]);
  temp2.listen([](float temperature, bool changed) {
    Serial.printf("Temperature 2: %.2f\n", temperature);
  });
}

void loop() {
  if (!temp1.read()) {
    Serial.println("Not ready yet");
  }
  if (!temp2.read()) {
    Serial.println("Not ready yet");
  }
  delay(2000);
}
