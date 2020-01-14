#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLEAdvertising.h>
#include <string.h>
using namespace std;
int scanTime = 5; //In seconds
bool connected = false;
BLEScan *pBLEScan;
BLEClient* client;
const string dronePrefix = "Swing_";
vector<BLEAdvertisedDevice> devices;
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
  void onResult(BLEAdvertisedDevice device)
  {
    devices.push_back(device);    
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

void findBleDevices()
{
  // put your main code here, to run repeatedly:
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  Serial.println(foundDevices.getCount());
  pBLEScan->clearResults(); // delete results fromBLEScan buffer to release memory
}

void loop()
{  
  Serial.println("Scanning for devices...");
  findBleDevices();     
  Serial.printf("%s %d %s",
                "Found ",
                devices.size(),
                "devices.");
  for(auto device : devices) {
    Serial.printf("%s, rssi: %d, micros: %d\n", device.toString().c_str(), device.getRSSI(), micros());
  }
  
}