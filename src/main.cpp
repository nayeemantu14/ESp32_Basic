#include <Arduino.h>
#include <ArduinoJson.h>
#include "debug.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <esp_sleep.h>
#include "wifimqtt.h"



//#define DHTPIN 21
//#define DHTTYPE DHT11
#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 20       /* Time ESP32 will go to sleep (in seconds) */
// Objects
//DHT dht(DHTPIN, DHTTYPE);

// Global Variables
uint32_t tempTime = millis();
uint32_t flowTime = 0;
byte data[9];
float temp;
float humidity;
static float flowrate;
bool isFlowAvailable = false;
extern WiFiManager wm;

byte readtempCommand[] = {0x01, 0x03, 0x00, 0x08, 0x00, 0x02, 0x45, 0xC9};
byte readflowCommand[] = {0x01, 0x03, 0x00, 0x06, 0x00, 0x02, 0x24, 0x0A};
byte readcumflowCommand[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x02, 0xC4, 0x0B};

// Function Prototypes

void displayTemperature();
void displayHumidity();
void displayFlow();
void sendESPdata();
float bytesToFloat(byte b1, byte b2, byte b3, byte b4);

void setup()
{
  Serial.begin(115200);
  Serial1.begin(9600, SERIAL_8E1);
  adc_attenuation_t::ADC_11db;
  connectAP();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  //dht.begin();
  // Configure wake up source
  //esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
}

void loop()
{
  uint32_t now = millis();
  if(now - flowTime > 5000)
  {
    flowTime = millis();
    Serial1.write(readflowCommand, sizeof(readflowCommand));

    if(Serial1.available()>=8)
    {
      Serial1.readBytes(data, sizeof(data)); 
      if (data[0] == 0x01 ) 
      {
        // Extract data and convert to float
        float value = bytesToFloat(data[3], data[4], data[5], data[6]);
        isFlowAvailable = true;
        flowrate = value;
      } 
      else 
      {
        debugln("Error: Sensor data not valid");
        isFlowAvailable = false;
      }
    }
    else
    {
      isFlowAvailable = false;
    }
  }

  if (!client.connected())
  {
    reconnect();
  }
  if (!client.loop())
  {
    client.connect("ESP32-");
  }
  // Enter deep sleep mode
   if(now - tempTime > 30000)
  {
    displayFlow();
    sendESPdata();
    client.loop();
    tempTime = now;
    //client.disconnect();
    //delay(500);
    //esp_deep_sleep_start();
  }
}

// Functions
void displayTemperature()
{
  char *msg = new char[255];
  sprintf(msg, "Temperature: %.2f", temp);
  debugln(msg);
  free(msg);
}

void displayHumidity()
{
  char *msg = new char[255];
  sprintf(msg, "Humidity: %.2f", humidity);
  debugln(msg);
  free(msg);
}
void displayFlow()
{
  char *msg = new char[255];
  sprintf(msg, "Flowrate: %.3f", flowrate);
  debugln(msg);
  free(msg);
}
void sendESPdata()
{
  JsonDocument doc;

  doc["device"] = WiFi.macAddress();
  if(isFlowAvailable)
  {
    doc["flowrate"] = flowrate;
  }
  else
  {
    doc["flowrate"] = "no data";
  }
  doc.shrinkToFit(); // optional
  char buff[256];
  serializeJson(doc, buff);
  debugln(buff);
  client.publish("SmartTee", buff);
}
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