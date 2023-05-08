#include <Arduino.h>

#include <NativeEthernet.h>
#include <NativeEthernetUdp.h>

#include "ApcConfig.h"
#include "ApcId.h"
#include "ApcArtnet.h"
#include "ApcOsc.h"
#include "ApcLed.h"

// #define RESTART_ADDR       0xE000ED0C
// #define READ_RESTART()     (*(volatile uint32_t *)RESTART_ADDR)
// #define WRITE_RESTART(val) ((*(volatile uint32_t *)RESTART_ADDR) = (val))

void convertFromJson(JsonVariantConst src, IPAddress& dst) {
  dst.fromString(src.as<const char*>());
}
bool canConvertFromJson(JsonVariantConst src, IPAddress& dst) {
  return dst.fromString(src.as<const char*>());
}
apc::Config config;
apc::Id id;
apc::Artnet artnet;
apc::Osc osc;
apc::Led led;

void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence,
                uint8_t* data, IPAddress remoteIP);
void onSync(IPAddress remoteIP);

void onReceiveRst();
void onReceiveLedTest();
void onReceivePrint();
void onReceiveConfigInt(const char* category, const char* name, int value);
void onReceiveConfigIntArray(const char* category, const char* name, int* value, size_t size);
void onReceiveConfigIpaddress(const char* category, const char* name, IPAddress value);

void printDeviceSetting();
void printIp(const char* name, IPAddress ip);

uint8_t universePerStrip;
uint16_t numPixels;
bool bSerialOpen = false;
void setup() {
    Serial.begin(115200);
    // while (! Serial); 
    delay(1000);
    Serial.println("program start");

    config.init(0, 1024);

    id.init();
    uint8_t deviceId = id.getId();

    uint8_t mac[6];
    for (auto i = 0; i < 6; i ++) {
        mac[i] = config.json["network"]["mac"][i];
    }
    if (config.json["network"]["macAddId"].as<bool>()) {
        mac[5] += deviceId;
    }
    IPAddress ip = config.json["network"]["ip"];
    if (config.json["network"]["ipAddId"].as<bool>()) {
        ip[3] += deviceId;
    }

    Ethernet.begin(
        mac,
        ip, 
        config.json["network"]["gateway"], 
        config.json["network"]["gateway"], 
        config.json["network"]["subnet"]);
    
    Serial.print("mac: ");
    for (auto i = 0; i < 6; i ++) {
        Serial.print(mac[i]);
        if (i < 5) {
            Serial.print(":");
        } else {
            Serial.println();
        }
    }
    Serial.print("ip: ");
    for (auto i = 0; i < 4; i ++) {
        Serial.print(ip[i]);
        if (i < 3) {
            Serial.print(".");
        } else {
            Serial.println();
        }
    }
    // Ethernet.begin(
    //     config.json["network"]["mac"],
    //     ip, gateway, gateway, subnet
    // );
    // Ethernet.begin(mac, ip, gateway, gateway, subnet);
    // Ethernet.begin(mac, ip);
    universePerStrip = config.json["led"]["numPixels"].as<uint16_t>() / 170.0f;
    
    artnet.init(config.json["artnet"]["port"], 
        config.json["artnet"]["net"], 
        config.json["artnet"]["subNet"], 
        config.json["artnet"]["universe"]);
    
    artnet.setArtDmxCallback(onDmxFrame);
    artnet.setArtSyncCallback(onSync);
    int dstPort = config.json["osc"]["dstPort"];
    if (config.json["osc"]["dstPortAddId"]) {
        dstPort += deviceId;
    }
    osc.init(config.json["osc"]["srcPort"], config.json["osc"]["dstIp"], dstPort);
    osc.setRstCallback(onReceiveRst);
    osc.setPrintCallback(onReceivePrint);
    osc.setLedTestCallback(onReceiveLedTest);
    osc.setConfigCallbackInt(onReceiveConfigInt);
    osc.setConfigCallbackIntArray(onReceiveConfigIntArray);
    osc.setConfigCallbackIpaddress(onReceiveConfigIpaddress);

    led.init(
        (int)config.json["led"]["numPixels"],
        (uint8_t)config.json["led"]["colorOrder"],
        (uint8_t)config.json["led"]["protocol"]);
    
    if (config.json["led"]["initTest"].as<bool>()) {
        led.initTest();
    }
}

void loop() {
    artnet.update();
    osc.update();
    if (Serial) {
        if (!bSerialOpen) {
            bSerialOpen = true;
            printDeviceSetting();
        }
    } else {
        if (bSerialOpen) {
            bSerialOpen = false;
        }
    }
    // put your main code here, to run repeatedly:
}
void onDmxFrame(
    uint16_t universe, 
    uint16_t length, 
    uint8_t sequence,
    uint8_t* data, 
    IPAddress remoteIP) {

    uint8_t stripIndex = universe / universePerStrip;
    uint8_t universeIndex = universe % universePerStrip;

    // Serial.printf("on dmx frame: {uni: %d, len: %d, seq: %d}\n", universe, length, sequence);
    // Serial.print(", strip index: ");
    // Serial.print(stripIndex);
    // Serial.print(", univIndex: ");
    // Serial.print(universeIndex);
    // Serial.println("}");
    
    led.setPixels(stripIndex, universeIndex * 170, data, length);
}
//on dmx frame: { uni: 31, len: 512, strip index: 7, univIndex: 3}
void onSync(IPAddress remoteIP) { 
    // Serial.println("on sync: ");
    led.show();
}

void onReceiveRst() {
    // _restart_Teensyduino_();
    // WRITE_RESTART(0x5FA0004);
    // SCB_AIRCR = 0x05FA0004;
    SCB_AIRCR = 0x05FA0004;
    asm volatile ("dsb");
    // asm volatile ("BKPT 254");
}
void onReceiveLedTest() {
    led.initTest();
}
void onReceivePrint() {
    printDeviceSetting();
}
void onReceiveConfigInt(const char* category, const char* name, int value) {
    bool result = false;
    if (config.setValue(category, name, value)) {
        config.write();
        result = true;
    }
    Serial.printf("cat: %s, name: %s, value: %d, result: %d\n", category, name, value, result);
    osc.sendConfigReply(category, name, result);
}
void onReceiveConfigIntArray(const char* category, const char* name, int* value, size_t size) {
    bool result = false;
    if (config.setValue(category, name, value, size)) {
        config.write();
        result = true;
    }
    // Serial.printf("cat: %s, name: %s, value: %d, result: %d\n", category, name, value, result);
    // for (size_t i; i < size; i ++) {

    // }
    osc.sendConfigReply(category, name, result);
}
void onReceiveConfigIpaddress(const char* category, const char* name, IPAddress value) {
    // Serial.printf("cat: %s, name: %s, value: %d.%d.%d.%d\n", category, name, value[0], value[1], value[2], value[3]);
    bool result = false;
    if (config.setValue(category, name, value)) {
        config.write();
        result = true;
    }
    osc.sendConfigReply(category, name, result);
}

void printDeviceSetting() {
    Serial.print("============================\n");
    Serial.printf("device id: %d\n", id.getId());
    printIp("ip", Ethernet.localIP());
    printIp("subnet", Ethernet.subnetMask());
    printIp("gateway", Ethernet.gatewayIP());
    uint8_t mac[6];
    Ethernet.MACAddress(mac);
    Serial.printf("mac: %d:%d:%d:%d:%d:%d\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    Serial.print("============================\n");
    config.print();
}

void printIp(const char* name, IPAddress ip) {
    Serial.printf("%s: %d.%d.%d.%d\n", name, ip[0], ip[1], ip[2], ip[3]);
}