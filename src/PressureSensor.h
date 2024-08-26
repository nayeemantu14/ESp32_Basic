#include<Arduino.h>

#define PRESSUREPIN 3

void init_pressure()
{
    pinMode(PRESSUREPIN, INPUT);
}

uint16_t readPressure()
{
    uint16_t analogPressure = analogRead(PRESSUREPIN);
    return analogPressure;
}

