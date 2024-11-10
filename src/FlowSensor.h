#ifndef FLOWSENSOR_H
#define FLOWSENSOR_H

#include <Arduino.h>

// Function Prototypes
float flowToFloat(byte b1, byte b2, byte b3, byte b4);
bool readFlowSensorData(byte* command, size_t commandSize, float& flowrate, byte* data, size_t dataSize);

float flowToFloat(byte b1, byte b2, byte b3, byte b4) {
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
        if (data[0] != 0x42 && data[1] !=0x4d) {
            Serial.println(data[0], HEX);
            Serial.println(data[1], HEX);
            // Extract data and convert to float
            Serial.println("Error: Sensor data not valid");
            //ESP.restart();
            return false;
        } else {
            flowrate = flowToFloat(data[16], data[17], data[18], data[19]);
            Serial.println(data[0], HEX);
            Serial.println(data[1], HEX);
            return true;
        }
    } else {
        return false;
    }
}

#endif // FLOWSENSOR_H
