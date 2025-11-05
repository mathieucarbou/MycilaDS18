#include <Arduino.h>
#include <MycilaDS18.h>

Mycila::DS18 temp;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    continue;

  // set your address here
  temp.begin(18, 0x983cee0457ea9f28ULL);
  // temp.begin(18, 10969904494789959464ULL);

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
