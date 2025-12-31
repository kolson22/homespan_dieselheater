#pragma once
#include "HomeSpan.h"
#include "BLEHeater.h"

struct HeaterService : Service::HeaterCooler {
    SpanCharacteristic *active;
    SpanCharacteristic *currentTemp;
    SpanCharacteristic *currentState;
    SpanCharacteristic *targetState;
    SpanCharacteristic *displayUnits;
    SpanCharacteristic *targetTemp;
    BLEHeater &heater;

    HeaterService(BLEHeater &h) : heater(h) {
        active = new Characteristic::Active(0);
        currentTemp = new Characteristic::CurrentTemperature(0.0);
        currentState = new Characteristic::CurrentHeaterCoolerState(1);   // 1 = Idle
        targetState = new Characteristic::TargetHeaterCoolerState(1);     // 1 = Heat

        displayUnits = new Characteristic::TemperatureDisplayUnits(0);    // 0 = Celsius
        targetTemp = new Characteristic::HeatingThresholdTemperature(22.0); // Target heat temp

        active->setVal(0);
        currentTemp->setVal(0.0);
        currentTemp->setRange(-60, 120, 0.1);
        currentState->setVal(1);
        targetState->setValidValues(2, 0, 1);
        targetState->setVal(1);
        displayUnits->setVal(0);
        

        // BLE → HomeKit: heater sends update or poll detects change
        heater.onStateChanged = [this](bool power, float temp, bool heating) {
            if (active->getVal<bool>() != power)
                active->setVal(power ? 1 : 0);
            if (fabs(currentTemp->getVal<float>() - temp) > 0.1)
                currentTemp->setVal(temp);
            currentState->setVal(heating ? 2 : 1);  // 2=Heating, 1=Idle
        };

        // Start background poll task for heater
        xTaskCreatePinnedToCore(
            pollTask,              // task function
            "HeaterPollTask",      // task name
            4096,                  // stack size
            this,                  // parameter = this service
            1,                     // priority
            nullptr,               // task handle
            1                      // core
        );
    }

    boolean update() override {
        heater.setPower(active->getNewVal<bool>());
        // currentState->setVal(active->getNewVal<bool>() ? 2 : 1);
        return true;
    }

    // Background polling loop
    static void pollTask(void *param) {
        auto *svc = static_cast<HeaterService*>(param);
        for (;;) {
            svc->heater.poll();          // request CMD_STATUS
            vTaskDelay(pdMS_TO_TICKS(5000));  // every 5s
        }
    }
};