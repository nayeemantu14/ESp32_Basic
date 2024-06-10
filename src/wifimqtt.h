#include <string.h>
#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h>
#include <Servo.h>


String clientID = "ESP32- ";
const char *mqtt_server = "35.197.176.42";
const char *mqtt_user = "nayeem";
const char *mqtt_password = "Moon-2008";
WiFiClient espClient;
PubSubClient client(espClient);


void reconnect()
{
    while(!client.connected())
    {
        debugln("Attempting MQTT Connection...");
        clientID += String(random(0xffff), HEX);
        if(client.connect(clientID.c_str(), mqtt_user, mqtt_password))
        {
            debugln("Connected to MQTT");
            client.subscribe("fromNodeRED");
            client.subscribe("motor");
            client.subscribe("time");
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
        servoInit();
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
       servoDeInit();
    }

    if(String(topic) == "time")
    {
        // Stream& input;

        JsonDocument doc;

        DeserializationError error = deserializeJson(doc, messageTemp);

        if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        return;
        }

        float temperature = doc["temperature"];
        int humidity = doc["humidity"];

        String output = "Temperature: "+String(temperature)+" Humidity: "+ String(humidity);
        debugln(output);
    }
}

void connectAP()
{
    debugln("Connecting to WiFi");
    WiFiManager wm;
    bool res;
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
    debug("Connected to WiFi");
}