#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEAdvertisedDevice.h>
#include <BLEAdvertising.h>
#include <string.h>
#include <WifiEspNowBroadcast.h>
using namespace std;

//wifi
#define GROUP_NAME "DroneDimension"
#define MAX_PEERS 20
int oldPeerCount = -1;

//ble
#define SCAN_TIME 1
#define MESSAGE_SIZE WIFIESPNOW_MAXMSGLEN
BLEScan *pBLEScan;
vector<BLEAdvertisedDevice> devices;
int oldBleCount = -1;

//mqtt
char message[MESSAGE_SIZE];
String id = WiFi.macAddress();
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice device)
  {
    devices.push_back(device);    
  }
};

void printReceivedMessage(const uint8_t mac[6], const uint8_t* buf, size_t count, void* cbarg) {
  Serial.printf("Message from %02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  for (int i = 0; i < count; ++i) {
    Serial.print(static_cast<char>(buf[i]));
  }
  Serial.println();
}


void wifiSetup() {
  WiFi.persistent(false);
  bool ok = WifiEspNowBroadcast.begin(GROUP_NAME);
  if (!ok) {
    Serial.println("WifiEspNowBroadcast.begin() failed");
    ESP.restart();
  }
  WifiEspNowBroadcast.onReceive(printReceivedMessage, nullptr);

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
  pBLEScan->clearResults(); // delete results fromBLEScan buffer to release memory
}

int printPeers() {
  WifiEspNowPeerInfo peers[MAX_PEERS];
  int nPeers = std::min(WifiEspNow.listPeers(peers, MAX_PEERS), MAX_PEERS);
  if(nPeers == 0) return 0; 
  for (int i = 0; i < nPeers; ++i) {
    // Heltec.display->drawStringMaxWidth(0,25*i, 25, peers[i]);
    Serial.printf(" %02X:%02X:%02X:%02X:%02X:%02X\n", peers[i].mac[0], peers[i].mac[1], peers[i].mac[2], peers[i].mac[3], peers[i].mac[4], peers[i].mac[5]);
    Serial.println();
  }
  return nPeers;
}
void sendMessage() {
  // WifiEspNowBroadcast.send(reinterpret_cast<const uint8_t*>(message), MESSAGE_SIZE);
}

void loop()
{  
  findBleDevices();
  if(oldBleCount != devices.size()){     
    Serial.printf("%d ble devices\n", devices.size());
    oldBleCount = devices.size();
  }
  for(auto device : devices) {

    snprintf(message, sizeof(message), "%s, rssi: %d, micros: %d, id: %s", device.toString().c_str(), device.getRSSI(), micros(), id.c_str());
    sendMessage();
  }
  devices.clear();
  printPeers();
  WifiEspNowBroadcast.loop();
  delay(10);
}