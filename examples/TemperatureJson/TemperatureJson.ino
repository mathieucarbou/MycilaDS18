#include <Arduino.h>
#include <ArduinoJson.h>
#include <MycilaDS18.h>

Mycila::DS18 temp;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    continue;

  temp.begin(18);
  temp.listen([](float temperature) {
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
    Serial.println("Temperature: " + String(temp.getTemperature().value_or(0)));
  }
  delay(500);
}
