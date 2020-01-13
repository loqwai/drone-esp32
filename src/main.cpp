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
vector<BLEAdvertisedDevice> drones;

void cloneService(BLEServer *server, BLERemoteService *remote){
  BLEService *service = server->createService(remote->getUUID()); 
  BLEAdvertising *ad = BLEDevice::getAdvertising();
  ad->addServiceUUID(remote->getUUID());
  ad->setScanResponse(true);
  ad->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  ad->setMinPreferred(0x12); 
  service->start();
}

void cloneDrone(BLEClient &client)
{    
  Serial.println("cloneDrone");
  BLEServer *server = BLEDevice::createServer();
  auto services = client.getServices();
  Serial.println("read services");
  Serial.printf("Num services: %d",services->size());

  for (auto entry = services->begin(); entry != services->end(); entry++)
  {
    auto service = entry->second;    
    Serial.printf("Found service: %s", service->getUUID().toString().c_str());
    cloneService(server, service);
  }
  BLEDevice::startAdvertising();
}
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
  void onResult(BLEAdvertisedDevice device)
  {
    string deviceName = device.getName();
    if (deviceName.find(dronePrefix) == -1)
      return;

    drones.push_back(device);
    Serial.printf("Advertised Device: %s \n", device.toString().c_str());
  }
};

class MyClientCallback : public BLEClientCallbacks
{
  void onConnect(BLEClient *c)
  {    
    Serial.println("onConnect");
    connected = true;
    client = c;
  }

  void onDisconnect(BLEClient *pclient)
  {    
    Serial.println("onDisconnect");
    connected = false;
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
void connectToDrone(BLEAdvertisedDevice &drone)
{
  BLEClient *pClient = BLEDevice::createClient();
  Serial.println(" - Created client");

  // Connect to the BLE Server.
  pClient->setClientCallbacks(new MyClientCallback());  
  pClient->connect(&drone);
  

  // Obtain a reference to the service we are after in the remote BLE server.
  // BLERemoteService *pRemoteService = pClient->getService(serviceUUID);
  // if (pRemoteService == nullptr)
  // {
  //   Serial.print("Failed to find our service UUID: ");
  //   Serial.println(serviceUUID.toString().c_str());
  //   pClient->disconnect();
  //   return;
  // }
}
 
void startClone(BLEAdvertisedDevice &drone)
{
  Serial.println("cloneDrone()");
  BLEAdvertising *fakeAd = BLEDevice::getAdvertising();
  fakeAd->addServiceUUID(drone.getServiceUUID());
  fakeAd->setScanResponse(true);
  fakeAd->setMinPreferred(0x06); // functions that help with iPhone connections issue
  fakeAd->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  
}
void loop()
{
  if (drones.size() == 0)
  {
    Serial.println("Scanning for devices...");
    return findBleDevices();    
  }
  Serial.printf("%s %d %s",
                "Found ",
                drones.size(),
                " Swing devices.");
                
  auto drone = drones[0];
  if(!connected) {
    return connectToDrone(drone);
  }
  cloneDrone(*client);  
  delay(20000);
}