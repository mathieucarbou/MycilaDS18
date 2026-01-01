// SPDX-License-Identifier: MIT
/*
 * Copyright (C) Mathieu Carbou
 */
#include <Arduino.h>
#include <MycilaDS18.h>

Mycila::DS18 temp;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    continue;

  temp.begin(18);
  temp.listen([](float temperature, bool changed) {
    Serial.printf("Temperature: %.2f\n", temperature);
  });
}

void loop() {
  if (!temp.read()) {
    Serial.println("Not ready yet");
  }
  delay(2000);
}
