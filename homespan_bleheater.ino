#include "HomeSpan.h"
#include "BLEHeater.h"
#include "HeaterService.h"
#include "BLEHeaterCommands.h"

BLEHeater heater("48:84:0E:1B:23:0E");

const unsigned long rebootInterval = 6UL * 60UL * 60UL * 1000UL;  // 6 hours

void setup() {
    Serial.begin(115200);
    homeSpan.begin(Category::Heaters, "Diesel Heater");
    
    heater.begin();
    heater.connect();

    new SpanAccessory();
        new Service::AccessoryInformation();
            new Characteristic::Name("Diesel Heater");
            new Characteristic::Manufacturer("VEVOR");
            new Characteristic::Identify();
        new HeaterService(heater);
}

void loop() {
  if (millis() >= rebootInterval) ESP.restart();
  homeSpan.poll();
}