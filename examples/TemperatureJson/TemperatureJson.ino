#include <Arduino.h>
#include <MycilaDS18.h>
#include <ArduinoJson.h>

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
  }
  delay(500);
}
