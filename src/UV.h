#ifndef UV_H
#define UV_H

#include <Arduino.h>

#define UVPIN 7

void UV_init();
uint16_t readUV();


void UV_init()
{
    pinMode(UVPIN, INPUT);
}

uint16_t readUV()
{
    uint16_t analogUV = analogRead(UVPIN);
    return analogUV;
}

#endif