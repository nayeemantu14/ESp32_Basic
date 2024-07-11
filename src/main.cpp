#include <Arduino.h>
#include <ArduinoJson.h>
#include "debug.h"
#include <esp_sleep.h>
#include "wifimqtt.h"
#include "OTA.h"
#include "FlowSensor.h"

//defines
#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 20       /* Time ESP32 will go to sleep (in seconds) */
#define UPDATE_FREQ 3600000
// Objects

// Global Variables
uint32_t tempTime = millis();
uint32_t flowTime = 0;
uint32_t reportTime = millis(); // Track the last report time

byte data[9];
static float flowrate;
static float lastReportedFlowrate = 0.0; // Track the last reported flow rate
bool isFlowAvailable = false;

extern WiFiManager wm;

byte readtempCommand[] = {0x01, 0x03, 0x00, 0x08, 0x00, 0x02, 0x45, 0xC9};
byte readflowCommand[] = {0x01, 0x03, 0x00, 0x06, 0x00, 0x02, 0x24, 0x0A};
byte readcumflowCommand[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x02, 0xC4, 0x0B};

// Function Prototypes
void displayFlow();
void sendESPdata();
float bytesToFloat(byte b1, byte b2, byte b3, byte b4);

void setup()
{
  Serial.begin(115200);
  Serial1.begin(9600, SERIAL_8E1);
  adc_attenuation_t::ADC_11db;
  connectAP();
  enableOTA();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  // Configure wake up source
  //esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
}

void loop()
{
  server.handleClient();
  uint32_t now = millis();

  if (!client.connected())
  {
    reconnect();
  }
  if (!client.loop())
  {
    client.connect("ESP32-");
  }

  if (now - flowTime > 5000)
  {
    flowTime = millis();
    isFlowAvailable = readFlowSensorData(readflowCommand, sizeof(readflowCommand), flowrate, data, sizeof(data));
  }

  // Report data every hour or when the flow rate changes
  if ((now - reportTime >= UPDATE_FREQ) || (isFlowAvailable && (flowrate != lastReportedFlowrate)))
  {
    reportTime = now;
    lastReportedFlowrate = flowrate;
    displayFlow();
    sendESPdata();
    client.loop();
  }
}

// Functions

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
  if (isFlowAvailable)
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
