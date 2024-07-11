#include <string.h>
#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Servo.h>


String clientID = "ESP32- ";
const char *mqtt_server = "35.189.51.29";
const char *mqtt_user = "nayeem";
const char *mqtt_password = "Moon-2008";
WiFiClient espClient;
PubSubClient client(espClient);


void reconnect()
{
    while(!client.connected())
    {
        debugln("Attempting MQTT Connection...");
        clientID = String(WiFi.macAddress());
        debugln(clientID);
        if(client.connect(clientID.c_str(), mqtt_user, mqtt_password))
        {
            debugln("Connected to MQTT");
            client.subscribe("motor");
        }
        else
        {
            debugln("failed, rc=");
            debugln(client.state());
            debugln("try again in 5S");
            delay(5000);
        }
    }
}

void callback(char *topic, byte *message, unsigned int length)
{
    String messageTemp;

    for(int i =0; i<length; i++)
    {
        messageTemp += (char)message[i];
    }
    if(String(topic) == "motor")
    {
        debugln("Motor topic received");
        EEPROM.begin(EEPROM_SIZE);
        uint8_t valveState = EEPROM.readBool(addr);
       if(messageTemp == "on" && !valveState)
       {
            valveOn();
       }
       else if(messageTemp == "off" && valveState)
       {
            valveOff();
       }
       EEPROM.end();
    }
}

void connectAP()
{
    debugln("Connecting to WiFi");
    WiFiManager wm;
    bool res;
    neopixelWrite(RGB_BUILTIN,2,0,0);
    res = wm.autoConnect("SMTAP", "EnwareIOT1");
    byte cnt = 0;
    if(!res)
    {
        while(WiFi.status() != WL_CONNECTED)
        
        {
            delay(1000);
            debug(".");
            cnt++;
            if(cnt>30)
            {
                ESP.restart();
            }
        }
    }
    debugln("Connected to WiFi");
    neopixelWrite(RGB_BUILTIN,0,2,0);
}
