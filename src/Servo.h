#include <ESP32Servo.h>
#include <EEPROM.h>

#define SERVOPIN 12
#define EEPROM_SIZE 1

Servo myservo;

uint8_t addr = 0;
uint8_t isValveOn;


void servoInit()
{
    myservo.attach(SERVOPIN, 900, 2100);
}

void servoDeInit()
{
    myservo.detach();
}

void valveOn()
{
    myservo.writeMicroseconds(1900);
    delay(500);
    isValveOn = 1;
    EEPROM.writeBool(addr, isValveOn);
    EEPROM.commit();
}

void valveOff()
{
    myservo.writeMicroseconds(900);
    delay(500);
    isValveOn = 0;
    EEPROM.writeBool(addr, isValveOn);
    EEPROM.commit();
}