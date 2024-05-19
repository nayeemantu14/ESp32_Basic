#include <ESP32Servo.h>

#define SERVOPIN 12
Servo myservo;

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
}

void valveOff()
{
    myservo.writeMicroseconds(900);
}