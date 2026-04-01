#include "BLEHeater.h"
#include <Arduino.h>

#define SERVICE_UUID     "0000FFE0-0000-1000-8000-00805F9B34FB"
#define WRITE_CHAR_UUID  "0000FFE1-0000-1000-8000-00805F9B34FB"

static BLEHeater* instancePtr = nullptr;

BLEHeater::BLEHeater(const char *macAddress) {
    instancePtr = this;
    mac = macAddress;
}

void BLEHeater::begin() {
    stateMutex = xSemaphoreCreateMutex();
    BLEDevice::init("");
}

bool BLEHeater::connect() {
    if (heaterConnected && client && client->isConnected()) return true;

    if (client) {
        client->disconnect();
        delete client;
        client = nullptr;
    }

    client = BLEDevice::createClient();
    BLEAddress addr(mac);
    if (!client->connect(addr)) {
        heaterConnected = false;
        return false;
    }

    auto service = client->getService(BLEUUID(SERVICE_UUID));
    if (!service) {
        client->disconnect();
        heaterConnected = false;
        return false;
    }

    writeChar = service->getCharacteristic(BLEUUID(WRITE_CHAR_UUID));
    if (!writeChar) {
        client->disconnect();
        heaterConnected = false;
        return false;
    }

    heaterConnected = true;

    if (writeChar->canNotify()) {
        writeChar->registerForNotify(BLEHeater::notifyCallback);
    }

    return true;
}

bool BLEHeater::isConnected() {
    return heaterConnected && client && client->isConnected();
}

void BLEHeater::poll() {
    if (!isConnected()) {
        Serial.println("heater wasn't connected, trying to connect");
        connect(); // try reconnect
        return;
    }
    // Serial.println("Polled heater for status...");
    sendCommand(CMD_STATUS);
}

bool BLEHeater::getPower() {
    xSemaphoreTake(stateMutex, portMAX_DELAY);
    bool val = powerState;
    xSemaphoreGive(stateMutex);
    return val;
}

float BLEHeater::getTemperature() {
    xSemaphoreTake(stateMutex, portMAX_DELAY);
    float val = temperature;
    xSemaphoreGive(stateMutex);
    return val;
}

void BLEHeater::setPower(bool on) {
    if (!isConnected() || !writeChar) return;
    const Command &cmd = on ? CMD_POWER_ON : CMD_POWER_OFF;
    sendCommand(cmd);
}

uint8_t BLEHeater::calcChecksum(const uint8_t *data, size_t len) {
    uint16_t sum = 0;
    for (size_t i = 2; i < len - 1; i++) {
        sum += data[i];
    }
    return (uint8_t)(sum % 256);
}

void BLEHeater::sendCommand(const Command &cmd) {
    if (!isConnected() || !writeChar) return;
    uint8_t message[8] = {0xAA, 0x55, 0x0C, 0x22, 0x01, 0x00, 0x00, 0x00};
    message[4] = cmd.cmd;
    message[5] = cmd.value;
    message[7] = calcChecksum(message, sizeof(message));
    writeChar->writeValue(message, sizeof(message), true);
}

void BLEHeater::handleNotification(uint8_t* data, size_t length) {
    if (length < 17) return;

    bool newPower = data[3] > 0;
    int16_t rawTemp = (int16_t)((uint16_t)data[16] << 8 | data[15]);
    float newTemp = (rawTemp - 32.0f) * 5.0f / 9.0f;  // raw value is in °F; convert to °C
    bool newHeating = data[5] == 3;

    bool changed = false;
    xSemaphoreTake(stateMutex, portMAX_DELAY);
    if (newPower != powerState || newTemp != temperature || newHeating != heating) {
        powerState = newPower;
        temperature = newTemp;
        heating = newHeating;
        changed = true;
    }
    xSemaphoreGive(stateMutex);

    if (changed && onStateChanged) {
        onStateChanged(newPower, newTemp, newHeating);
    }
}

void BLEHeater::notifyCallback(BLERemoteCharacteristic* pChar, uint8_t* pData, size_t length, bool isNotify) {
    if (instancePtr) {
        instancePtr->handleNotification(pData, length);
    }
}