#include "HomeSpan.h"
#include "BLEHeater.h"
#include "HeaterService.h"

BLEHeater heater("48:84:0E:1B:48:B4");

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
    homeSpan.poll();
}