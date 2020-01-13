#include <Arduino.h>
/*
   Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleScan.cpp
   Ported to Arduino ESP32 by Evandro Copercini
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLEAdvertising.h>
#include <string.h>
using namespace std;
int scanTime = 5; //In seconds
BLEScan *pBLEScan;
const string dronePrefix  = "Swing_";
vector<BLEAdvertisedDevice> swingDevices;

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
  void onResult(BLEAdvertisedDevice device)
  {
    string deviceName = device.getName();
    if(deviceName.find(dronePrefix) == -1)
    {
      swingDevices.push_back(device);
      return;
    }    
    Serial.printf("Advertised Device: %s \n", device.toString().c_str());
  }

};

void setup()
{
  Serial.begin(9600);
  Serial.println("Scanning...");

  BLEDevice::init("Fake-Drone");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99); // less or equal setInterval value
}

void findBleDevices() {
  // put your main code here, to run repeatedly:
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  Serial.println(foundDevices.getCount());
  pBLEScan->clearResults(); // delete results fromBLEScan buffer to release memory
}
void cloneDrone() {
  Serial.println("cloneDrone()");
  auto drone = swingDevices[0];
  
  BLEServer *pServer = BLEDevice::createServer();
  auto pAdvertising = pServer->getAdvertising();
  auto fakeDrone = new BLEAdvertisementData();
  fakeDrone->setManufacturerData(drone.getManufacturerData());
  fakeDrone->setServiceData(drone.getServiceDataUUID(), drone.getServiceData());
  fakeDrone->setName("Fake Drone");  
  pAdvertising->setAdvertisementData(*fakeDrone);
  Serial.println("About to advertise...");
  pAdvertising->start();
}
void loop()
{
  if(swingDevices.size() == 0) {
    Serial.println("Scanning for devices...");
    return findBleDevices();
  }
  
  Serial.printf("%s %d %s", 
                "Found ",
                swingDevices.size(),
                " Swing devices.");
  cloneDrone();
  delay(2000);
}