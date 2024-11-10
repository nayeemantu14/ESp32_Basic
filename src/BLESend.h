#include <Arduino.h>
#include "debug.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

BLEServer *pServer = NULL;
BLECharacteristic *pPressureCharacteristic = NULL;
BLECharacteristic *pFlowCharacteristic = NULL;


bool deviceConnected = false;
bool oldDeviceConnected = false;

uint16_t bPressure = 0;
float bflowrate = 0.0;

#define SERVICE_UUID "5fd4d1bd-3973-450c-a204-3a06da56e6d1"
#define PRESSURE_CHARACTERISTIC_UUID "8b33eeb9-20ba-4dfc-8ab7-224980eac6df"
#define FLOW_CHARACTERISTIC_UUID "10947cfd-ac10-4ff8-8efc-b2bdca0bb3e9"


// Setup callbacks onConnect and onDisconnect
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    BLEDevice::startAdvertising();
    debugln("Device Connected");
  }
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    debugln("Device Disconnected");
  }
};

void init_Ble() 
{
    BLEDevice::init("SmartTee");
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());


    BLEService *pService = pServer->createService(SERVICE_UUID);

    pPressureCharacteristic = pService->createCharacteristic(
                              PRESSURE_CHARACTERISTIC_UUID,
                              BLECharacteristic::PROPERTY_READ   |
                              BLECharacteristic::PROPERTY_NOTIFY |
                              BLECharacteristic::PROPERTY_INDICATE
                            );

  // Create a BLE Characteristic for Flowrate
    pFlowCharacteristic = pService->createCharacteristic(
                              FLOW_CHARACTERISTIC_UUID,
                              BLECharacteristic::PROPERTY_READ   |
                              BLECharacteristic::PROPERTY_NOTIFY |
                              BLECharacteristic::PROPERTY_INDICATE
                            );
    //pPressureCharacteristic->addDescriptor(new BLE2902());
    //pFlowCharacteristic->addDescriptor(new BLE2902());

    pService->start();

    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(false);
    pAdvertising->setMinPreferred(0x0);
    BLEDevice::startAdvertising();
    debugln("Waiting to Connect");
}

void handleBLEdata(uint16_t bPressure, float bflowrate )
{
    if (deviceConnected)
    {
        pPressureCharacteristic->setValue(bPressure);
        pPressureCharacteristic->notify();

        pFlowCharacteristic->setValue(bflowrate);
        pFlowCharacteristic->notify();
    }

    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the Bluetooth stack time to reset
        pServer->startAdvertising(); // restart advertising
        Serial.println("Started advertising");
        oldDeviceConnected = deviceConnected;
    }
    // Connecting
    if (deviceConnected && !oldDeviceConnected) {
        oldDeviceConnected = deviceConnected;
    }

}