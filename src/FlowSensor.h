#ifndef FLOWSENSOR_H
#define FLOWSENSOR_H

#include <Arduino.h>

// Function Prototypes
float bytesToFloat(byte b1, byte b2, byte b3, byte b4);
bool readFlowSensorData(byte* command, size_t commandSize, float& flowrate, byte* data, size_t dataSize = 8);

float bytesToFloat(byte b1, byte b2, byte b3, byte b4) {
    union {
        float f;
        byte b[4];
    } converter;
    converter.b[0] = b4;
    converter.b[1] = b3;
    converter.b[2] = b2;
    converter.b[3] = b1;
    return converter.f;
}

bool readFlowSensorData(byte* command, size_t commandSize, float& flowrate, byte* data, size_t dataSize) {
    Serial1.write(command, commandSize);

    if (Serial1.available() >= dataSize) {
        Serial1.readBytes(data, dataSize); 
        if (data[0] == 0x01) {
            // Extract data and convert to float
            flowrate = bytesToFloat(data[3], data[4], data[5], data[6]);
            return true;
        } else {
            Serial.println("Error: Sensor data not valid");
            return false;
        }
    } else {
        return false;
    }
}

#endif // FLOWSENSOR_H
