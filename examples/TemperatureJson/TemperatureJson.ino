#include <Arduino.h>
#include <ArduinoJson.h>
#include <MycilaDS18.h>

#include <string>

Mycila::DS18 temp;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    continue;

  temp.begin(18);
  temp.listen([](float temperature, bool changed) {
    JsonDocument doc;
    temp.toJson(doc.to<JsonObject>());
    serializeJson(doc, Serial);
    Serial.println();
  });
}

void loop() {
  if (!temp.read()) {
    Serial.println("Not ready yet");
  } else {
    Serial.printf("Temperature: %s\n", std::to_string(temp.getTemperature().value_or(0)).c_str());
  }
  delay(2000);
}
