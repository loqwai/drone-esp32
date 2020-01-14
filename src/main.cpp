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
vector<BLEAdvertisedDevice> devices;
uint8_t mac[6];
char message[256];
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
  void onResult(BLEAdvertisedDevice device)
  {
    devices.push_back(device);    
  }
};

void setup()
{
  esp_efuse_mac_get_default(mac);
  Serial.begin(9600);
  Serial.println("Scanning...");

  BLEDevice::init("Fake-Drone");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(10);
  pBLEScan->setWindow(9); // less or equal setInterval value
}

void findBleDevices()
{
  // put your main code here, to run repeatedly:
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  Serial.println(foundDevices.getCount());
  pBLEScan->clearResults(); // delete results fromBLEScan buffer to release memory
}

void sendMessage() {
  Serial.println(message);
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

    snprintf(message, sizeof(message), "%s, rssi: %d, micros: %d, id: %#6x", device.toString().c_str(), device.getRSSI(), micros(), mac);
    sendMessage();
  }
  
}