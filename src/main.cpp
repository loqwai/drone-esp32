#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEAdvertisedDevice.h>
#include <BLEAdvertising.h>
#include <string.h>
#include <WiFi.h>
#include <PubSubClient.h>
using namespace std;

// Update these with values suitable for your network.
WiFiClient espClient;

PubSubClient client(espClient);

//ble
#define SCAN_TIME 1
BLEScan *pBLEScan;
vector<BLEAdvertisedDevice> devices;

//wifi
#define ssid "ROBOT-WASTELAND"
#define password "lemonade"
#define mqtt_server "10.0.0.124"

//mqtt
char message[256];
String id = WiFi.macAddress();
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice device)
  {
    devices.push_back(device);    
  }
};

void scanNetworks() {
 
  int numberOfNetworks = WiFi.scanNetworks();
 
  Serial.print("Number of networks found: ");
  Serial.println(numberOfNetworks);
 
  for (int i = 0; i < numberOfNetworks; i++) {
 
    Serial.print("Network name: ");
    Serial.println(WiFi.SSID(i));
 
    Serial.print("Signal strength: ");
    Serial.println(WiFi.RSSI(i));
 
    Serial.print("MAC address: ");
    Serial.println(WiFi.BSSIDstr(i));
 
    Serial.print("Encryption type: ");   
    Serial.println("-----------------------");
 
  }
}

void setup()
{
  Serial.begin(9600);
  Serial.println("Setting up...");
  Serial.println("\tWifi");
  scanNetworks();
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(WiFi.status());
    Serial.print(".");
  }
  Serial.println("\tBLE");

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(10);
  pBLEScan->setWindow(9); // less or equal setInterval value
  
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