#pragma once
#include <BLEDevice.h>
#include <BLEHeaterCommands.h>

class BLEHeater {
public:
    BLEHeater(const char *macAddress);
    void begin();                   // initialize BLE
    bool connect();                  // connect to heater
    bool isConnected();              // check connection
    void poll();  

    // getters
    bool getPower();
    float getTemperature();

    // setters
    void setPower(bool on);

    std::function<void(bool power, float temp)> onStateChanged;

private:
    const char* mac;
    BLEClient *client = nullptr;
    BLERemoteCharacteristic *writeChar = nullptr;
    bool heaterConnected = false;

    bool powerState = false;
    float temperature = 0.0;

    void sendCommand(const Command &cmd);
    uint8_t calcChecksum(const uint8_t *data, size_t len);
    static void notifyCallback(BLERemoteCharacteristic* pChar, uint8_t* pData, size_t length, bool isNotify);
    void handleNotification(uint8_t* data, size_t length);
};