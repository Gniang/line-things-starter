#include <M5Stack.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <BLEServer.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// Device Name: Maximum 30 bytes
#define DEVICE_NAME "LINE Things Trial M5Stack"

// User service UUID: Change this to your generated service UUID
#define USER_SERVICE_UUID "9DD76B74-9301-4904-9D22-9096F561F4F8"
// User service characteristics
#define WRITE_CHARACTERISTIC_UUID "E9062E71-9E62-4BC6-B0D3-35CDCD9B027B"
#define NOTIFY_CHARACTERISTIC_UUID "62FBD229-6EDD-4D1A-B554-5C4E1BB29169"

// PSDI Service UUID: Fixed value for Developer Trial
#define PSDI_SERVICE_UUID "E625601E-9E55-4597-A598-76018A0d293D"
#define PSDI_CHARACTERISTIC_UUID "26E2B12B-85F0-4F3F-9FDD-91D114270E6E"

#define TEXT_X (M5.Lcd.width() / 2)
#define TEXT_Y (M5.Lcd.height() / 2 / 2)

// Servo
// called this way, it uses the default address 0x40
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
// you can also call it with a different address you want
//Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x41);
// you can also call it with a different address and I2C interface
//Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40, &Wire);

// Depending on your servo make, the pulse width min and max may vary, you 
// want these to be as small/large as possible without hitting the hard stop
// for max range. You'll have to tweak them as necessary to match the servos you
// have!
#define SERVOMIN  150 // this is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX  600 // this is the 'maximum' pulse length count (out of 4096)
// 600

// our servo # counter
uint8_t servonum = 0;

typedef enum{
  LOCK,
  UNLOCK
} LockState;

// typedef structしたいけど、関数にtypedef struct渡しはできないため真面目にstruct XXXする
// 参考:https://jiwashin.blogspot.com/2016/06/arduino-can-not-use-structure-as-parameter-of-function-in-sketch.html
struct ServoStatus{
  int servoNumber;
  LockState lockState;
};

struct ServoStatus ServoStatus1 = { 0 , LOCK };
struct ServoStatus ServoStatus2 = { 1 , LOCK };
struct ServoStatus ServoStatus3 = { 2 , LOCK };

void sendStatus();
bool IsServoLock(struct ServoStatus *);
bool IsServoUnlock(struct ServoStatus *);
void ServoLock(struct ServoStatus *);
void ServoUnlock(struct ServoStatus *);

BLEServer* thingsServer;
BLESecurity *thingsSecurity;
BLEService* userService;
BLEService* psdiService;
BLECharacteristic* psdiCharacteristic;
BLECharacteristic* writeCharacteristic;
BLECharacteristic* notifyCharacteristic;

long timestamp = 1;
bool deviceConnected = false;
bool oldDeviceConnected = false;
char sendbuffer[100];

class serverCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
   deviceConnected = true;
  };

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};

class writeCallback: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *bleWriteCharacteristic) {
    std::string value = bleWriteCharacteristic->getValue();
//    if ((char)value[0] <= 1) {
//      if ((char)value[0] == 1) {
//        M5.Lcd.fillRect(0, 0, M5.Lcd.width(), M5.Lcd.height() / 2, WHITE);
//        M5.Lcd.setTextColor(BLACK);
//        M5.Lcd.setTextSize(4);
//        M5.Lcd.drawString("LED: ON ", TEXT_X, TEXT_Y);
//      }
//      else {
//        M5.Lcd.fillRect(0, 0, M5.Lcd.width(), M5.Lcd.height() / 2, BLACK);
//        M5.Lcd.setTextColor(WHITE);
//        M5.Lcd.setTextSize(4);
//        M5.Lcd.drawString("LED: OFF", TEXT_X, TEXT_Y);
//      }
//    }
    if(value == "0"){
      
    }
    else if (value == "1") {
      M5.Lcd.fillRect(0, 0, M5.Lcd.width(), M5.Lcd.height() / 2, WHITE);
      M5.Lcd.setTextColor(BLACK);
      M5.Lcd.setTextSize(4);
      M5.Lcd.drawString("USE 1", TEXT_X, TEXT_Y);
      
      if(IsServoLock(&ServoStatus1)){
        ServoUnlock(&ServoStatus1);
      }
    }
    else if(value == "2") {
      M5.Lcd.fillRect(0, 0, M5.Lcd.width(), M5.Lcd.height() / 2, WHITE);
      M5.Lcd.setTextColor(BLACK);
      M5.Lcd.setTextSize(4);
      M5.Lcd.drawString("USE 2", TEXT_X, TEXT_Y);
      
      if(IsServoLock(&ServoStatus2)){
        ServoUnlock(&ServoStatus2);
      }
    }      
    else if(value == "3") {
      M5.Lcd.fillRect(0, 0, M5.Lcd.width(), M5.Lcd.height() / 2, WHITE);
      M5.Lcd.setTextColor(BLACK);
      M5.Lcd.setTextSize(4);
      M5.Lcd.drawString("USE 3", TEXT_X, TEXT_Y);
      
      if(IsServoLock(&ServoStatus3)){
        ServoUnlock(&ServoStatus3);
      }
    }
    sendStatus();
  }
};

// Servo

// you can use this function if you'd like to set the pulse length in seconds
// e.g. setServoPulse(0, 0.001) is a ~1 millisecond pulse width. its not precise!
void setServoPulse(uint8_t n, double pulse) {
  double pulselength;
  
  pulselength = 1000000;   // 1,000,000 us per second
  pulselength /= 60;   // 60 Hz
  Serial.print(pulselength); Serial.println(" us per period"); 
  pulselength /= 4096;  // 12 bits of resolution
  Serial.print(pulselength); Serial.println(" us per bit"); 
  pulse *= 1000000;  // convert to us
  pulse /= pulselength;
  Serial.println(pulse);
  pwm.setPWM(n, 0, pulse);
}

void ServoLock(struct ServoStatus *servoStatus){
    for (uint16_t pulselen = SERVOMIN; pulselen < SERVOMAX; pulselen++) {
      pwm.setPWM(servoStatus->servoNumber, 0, pulselen);
    }
    servoStatus->lockState = LOCK;
}

void ServoUnlock(struct ServoStatus *servoStatus){
    for (uint16_t pulselen = SERVOMAX; pulselen > SERVOMIN; pulselen--) {
      pwm.setPWM(servoStatus->servoNumber, 0, pulselen);
    }
    servoStatus->lockState = UNLOCK;
}

bool IsServoLock(struct ServoStatus *servoStatus){
  if(servoStatus->lockState == LOCK) return true;
  else return false;
}

bool IsServoUnlock(struct ServoStatus *servoStatus){
  if(servoStatus->lockState == UNLOCK) return true;
  else return false;
}


void setup() {
  M5.begin();

  BLEDevice::init("");
  BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT_NO_MITM);

  // Security Settings
  BLESecurity *thingsSecurity = new BLESecurity();
  thingsSecurity->setAuthenticationMode(ESP_LE_AUTH_BOND);
  thingsSecurity->setCapability(ESP_IO_CAP_NONE);
  thingsSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);

  setupServices();
  startAdvertising();

  // M5Stack LCD Setup
  M5.Lcd.setTextDatum(MC_DATUM);
  M5.Lcd.clear(BLACK);
  M5.Lcd.qrcode("https://line.me/R/nv/things/deviceLink", 110, 130, 100, 3);
  M5.Lcd.setTextColor(YELLOW);
  M5.Lcd.setTextSize(2);
  M5.Lcd.drawString("Ready to Connect", TEXT_X, TEXT_Y);
  Serial.println("Ready to Connect");

  // Servo Setup
  Serial.begin(9600);
  Serial.println("8 channel Servo start");
  
  pwm.begin();
  pwm.setPWMFreq(60);  // Analog servos run at ~60 Hz updates

  delay(10);
}

void loop() {
  M5.update();

  // Servo
  if(M5.BtnA.wasPressed()){
    if(IsServoUnlock(&ServoStatus1)){
      ServoLock(&ServoStatus1);
    }
    sendStatus();
  }
  
  if(M5.BtnB.wasPressed()){
    if(IsServoUnlock(&ServoStatus2)){
      ServoLock(&ServoStatus2);
    }
    sendStatus();
  }
  
  if(M5.BtnC.wasPressed()){
    if(IsServoUnlock(&ServoStatus3)){
      ServoLock(&ServoStatus3);
    }
    sendStatus();
  }


  // Disconnection
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // Wait for BLE Stack to be ready
    thingsServer->startAdvertising(); // Restart advertising
    oldDeviceConnected = deviceConnected;
    M5.Lcd.fillRect(0, 0, M5.Lcd.width(), M5.Lcd.height() / 2, BLACK);
    M5.Lcd.setTextColor(YELLOW);
    M5.Lcd.setTextSize(2);
    M5.Lcd.drawString("Ready to Connect", TEXT_X, TEXT_Y);
  }
  // Connection
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
    M5.Lcd.fillRect(0, 0, M5.Lcd.width(), M5.Lcd.height() / 2, BLACK);
    M5.Lcd.setTextColor(GREEN);
    M5.Lcd.setTextSize(2);
    M5.Lcd.drawString("Connected", TEXT_X, TEXT_Y);
  }
}

void sendStatus(){
    String value = "1,"; //1…ID、101…ステータス（１、３個目が使用中）
    value.concat(servoStatus(&ServoStatus1,&ServoStatus2,&ServoStatus3)); 
    value.concat(",");
    value.concat(nextStamp());
    value.toCharArray(sendbuffer, value.length()+1);
    notifyCharacteristic->setValue(sendbuffer);
    notifyCharacteristic->notify();
}

String servoStatus(struct ServoStatus *ServoStatus1,struct ServoStatus *ServoStatus2,struct ServoStatus *ServoStatus3){
  String value = "";
  if(ServoStatus1->lockState == LOCK){
    value.concat(String(0));
  }else{
    value.concat(String(1));
  }

  if(ServoStatus2->lockState == LOCK){
    value.concat(String(0));
  }else{
    value.concat(String(1));
  }

  if(ServoStatus3->lockState == LOCK){
    value.concat(String(0));
  }else{
    value.concat(String(1a));
  }

  return value;
}

String nextStamp(){   
    timestamp++;
    if(timestamp > 9){
      timestamp = 0;
    }
    return String(timestamp); 
}

void setupServices(void) {
  // Create BLE Server
  thingsServer = BLEDevice::createServer();
  thingsServer->setCallbacks(new serverCallbacks());

  // Setup User Service
  userService = thingsServer->createService(USER_SERVICE_UUID);
  // Create Characteristics for User Service
  writeCharacteristic = userService->createCharacteristic(WRITE_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_WRITE);
  writeCharacteristic->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);
  writeCharacteristic->setCallbacks(new writeCallback());

  notifyCharacteristic = userService->createCharacteristic(NOTIFY_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_NOTIFY);
  notifyCharacteristic->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);
  BLE2902* ble9202 = new BLE2902();
  ble9202->setNotifications(true);
  ble9202->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);
  notifyCharacteristic->addDescriptor(ble9202);

  // Setup PSDI Service
  psdiService = thingsServer->createService(PSDI_SERVICE_UUID);
  psdiCharacteristic = psdiService->createCharacteristic(PSDI_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ);
  psdiCharacteristic->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);

  // Set PSDI (Product Specific Device ID) value
  uint64_t macAddress = ESP.getEfuseMac();
  psdiCharacteristic->setValue((uint8_t*) &macAddress, sizeof(macAddress));

  // Start BLE Services
  userService->start();
  psdiService->start();
}

void startAdvertising(void) {
  // Start Advertising
  BLEAdvertisementData scanResponseData = BLEAdvertisementData();
  scanResponseData.setFlags(0x06); // GENERAL_DISC_MODE 0x02 | BR_EDR_NOT_SUPPORTED 0x04
  scanResponseData.setName(DEVICE_NAME);

  thingsServer->getAdvertising()->addServiceUUID(userService->getUUID());
  thingsServer->getAdvertising()->setScanResponseData(scanResponseData);
  thingsServer->getAdvertising()->start();
}
