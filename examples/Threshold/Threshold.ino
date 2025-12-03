#include <Arduino.h>
#include <MycilaDS18.h>

Mycila::DS18 temp;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    continue;

  temp.setThreshold(0.0f); // any temperature change will be reported in callback

  temp.listen([](float temperature, bool changed) {
    if (changed) {
      Serial.printf("Temperature changed: %f°C\n", temperature);
    }
  });

  temp.begin(18);
}

void loop() {
  temp.read();
  delay(1000);
}
