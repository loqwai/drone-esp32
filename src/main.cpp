#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEAdvertisedDevice.h>
#include <BLEAdvertising.h>
#include <string.h>
#include <WifiEspNowBroadcast.h>
using namespace std;

//wifi
#define GROUP_NAME "DroneDimension"

//ble
#define SCAN_TIME 1
#define MESSAGE_SIZE WIFIESPNOW_MAXMSGLEN
BLEScan *pBLEScan;
vector<BLEAdvertisedDevice> devices;

//mqtt
char message[MESSAGE_SIZE];
String id = WiFi.macAddress();
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice device)
  {
    devices.push_back(device);    
  }
};

void wifiSetup() {
  WiFi.persistent(false);
  bool ok = WifiEspNowBroadcast.begin(GROUP_NAME);
  if (!ok) {
    Serial.println("WifiEspNowBroadcast.begin() failed");
    ESP.restart();
  }

  // WifiEspNowBroadcast.onReceive(processRx, nullptr);
}
void bleSetup() {
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(10);
  pBLEScan->setWindow(9); // less or equal setInterval value
}

void setup()
{
  Serial.begin(9600);
  Serial.println("Setting up...");
  wifiSetup(); 
  bleSetup();
}

void findBleDevices()
{
  // put your main code here, to run repeatedly:
  BLEScanResults foundDevices = pBLEScan->start(SCAN_TIME, false);
  Serial.println(foundDevices.getCount());
  pBLEScan->clearResults(); // delete results fromBLEScan buffer to release memory
}

void sendMessage() {
  Serial.println(message);
  auto ok = WifiEspNowBroadcast.send(reinterpret_cast<const uint8_t*>(message), MESSAGE_SIZE);
  Serial.println(ok);
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

    snprintf(message, sizeof(message), "%s, rssi: %d, micros: %d, id: %s", device.toString().c_str(), device.getRSSI(), micros(), id.c_str());
    sendMessage();
  }
  devices.clear();
}