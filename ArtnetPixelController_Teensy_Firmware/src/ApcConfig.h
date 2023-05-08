#ifndef APC_CONFIG_H
#define APC_CONFIG_H

#include <ArduinoJson.h>
#include <StreamUtils.h>

#include "ApcVariables.h"

namespace apc {

class Config {
    private:
    int address;
    size_t size;
   public:
    StaticJsonDocument<1024> json;
    Config(){};
    void init(int address, size_t size) { 
        this->address = address;
        this->size = size;
        read(address, size); 
    }
    void read(int address, size_t size) {
        EepromStream eepromStream(address, size);
        DeserializationError error = deserializeJson(json, eepromStream);
        // TODO: 書き込み後の初回起動にeepromを無視してデフォルトにできる機能
        if (error) {
            Serial.print(F("deserializeJson() failed !!!: "));
            Serial.println(error.f_str());
        } else {
            Serial.println(F("deserializeJson() success !!!"));
        }
        using namespace variables::defaultSettings;
        
        JsonObject networkObj;
        initCategory("network", networkObj);
        initValue(networkObj, "ip", network::ip);
        initValue(networkObj, "gateway", network::gateway);
        initValue(networkObj, "subnet", network::subnet);
        initArray(networkObj, "mac", network::mac, 6);
        initValue(networkObj, "ipAddId", network::ipAddId);
        initValue(networkObj, "macAddId", network::macAddId);

        JsonObject artnetObj;
        initCategory("artnet", artnetObj);
        initValue(artnetObj, "port", artnet::port);
        initValue(artnetObj, "net", artnet::net);
        initValue(artnetObj, "subNet", artnet::subNet);
        initValue(artnetObj, "universe", artnet::universe);

        JsonObject oscObj;
        initCategory("osc", oscObj);
        initValue(oscObj, "dstPort", osc::dstPort);
        initValue(oscObj, "dstIp", osc::dstIp);
        initValue(oscObj, "srcPort", osc::srcPort);
        initValue(oscObj, "dstPortAddId", osc::dstPortAddId);

        JsonObject ledObj;
        initCategory("led", ledObj);
        initValue(ledObj, "numPixels", led::numPixels);
        initValue(ledObj, "protocol", led::protocol);
        initValue(ledObj, "colorOrder", led::colorOrder);
        initValue(ledObj, "initTest", led::initTest);

        serializeJson(json, eepromStream);

        serializeJsonPretty(json, Serial);
        Serial.println();
    }
    void write(int address = 0) {
        EepromStream eepromStream(address, size);
        serializeJson(json, eepromStream);
    }

    void initCategory(const char* name, JsonObject& obj) {
        if (json.containsKey(name)) {
            obj = json[name].as<JsonObject>();
        } else {
            obj = json.createNestedObject(name);
        }
    }
    void initValue(JsonObject& obj, const char* name, bool value) {
        if (!obj.containsKey(name)) {
            obj[name] = value;
        }
    }
    void initValue(JsonObject& obj, const char* name, int value) {
        if (!obj.containsKey(name)) {
            obj[name] = value;
        }
    }
    void initValue(JsonObject& obj, const char* name, IPAddress value) {
        if (!obj.containsKey(name)) {
            obj[name] = value;
        }
    }
    void initArray(JsonObject& obj, const char* name, const uint8_t* array, size_t length) {
        JsonArray arr;
        if (!obj.containsKey(name)) {
            arr = obj.createNestedArray(name);
        } else {
            arr = obj[name].as<JsonArray>();
        }
        for (size_t i = 0; i < length; i++) {
            if (arr.size() <= i) {
                arr.add(array[i]);
            }
        }
    }
    bool setValue(const char* category, const char* name, int value) {
        bool result = false;
        if(json.containsKey(category)) {
            JsonObject obj = json[category].as<JsonObject>();
            Serial.printf("json contains key: %s\n", category);
            if (obj.containsKey(name)) {
                Serial.printf("obj contains key: %s\n", name);
                if (obj[name].is<int>()) {
                    obj[name] = value;
                    result = true;
                    serializeJsonPretty(json, Serial);
                    Serial.println();
                }
            }
        }
        return result;
    }
    bool setValue(const char* category, const char* name, int* value, size_t size) {
        bool result = false;
        if(json.containsKey(category)) {
            JsonObject obj = json[category].as<JsonObject>();
            Serial.printf("json contains key: %s\n", category);
            if (obj.containsKey(name)) {
                JsonArray arr = obj[name].as<JsonArray>();
                if (arr.size() == size) {
                    for (uint8_t i = 0; i < size; i ++) {
                        Serial.printf("obj contains key: %s[%d]=%d\n", name, i, value[i]);
                        arr[i] = value[i];
                    }
                    result = true;
                    serializeJsonPretty(json, Serial);
                    Serial.println();
                }
            }
        }
        return result;
    }
    bool setValue(const char* category, const char* name, IPAddress value) {
        bool result = false;
        if(json.containsKey(category)) {
            JsonObject obj = json[category].as<JsonObject>();
            Serial.printf("json contains key: %s\n", category);
            if (obj.containsKey(name)) {
                Serial.printf("obj contains key: %s\n", name);
                if (obj[name].is<IPAddress>()) {
                    obj[name] = value;
                    result = true;
                    serializeJsonPretty(json, Serial);
                    Serial.println();
                }
            }
        }
        return result;
    }
    void print(bool bPretty = false) {
        if (bPretty) {
            serializeJsonPretty(json, Serial);
        } else {
            serializeJson(json, Serial);
        }
        Serial.println();
    }
};
}  // namespace apc
#endif  // APC_CONFIG_H