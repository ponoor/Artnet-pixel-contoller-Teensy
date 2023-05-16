#ifndef APC_OSC_H
#define APC_OSC_H
#include <Arduino.h>
#include <LiteOSCParser.h>
#include <NativeEthernetUdp.h>

namespace apc {
class Osc {
   private:
    EthernetUDP Udp;
    qindesign::osc::LiteOSCParser osc;
    uint8_t oscBuffer[1024];

    IPAddress dstIp;
    int dstPort;

    void (*rstCallback)();
    void (*ledTestCallback)();
    void (*printCallback)();
    void (*clearConfigCallback)();
    void (*configCallbackInt)(const char*, const char*, int);
    void (*configCallbackIntArray)(const char*, const char*, int*, size_t);
    void (*configCallbackIpaddress)(const char*, const char*, IPAddress);
    void (*configCallbackFloat)(const char*, const char*, float);
    void (*configCallbackString)(const char*, const char*, const char*);

   public:
    Osc(){};
    ~Osc(){};

    void init(int port, IPAddress dstIp, int dstPort) {
        Udp.begin(port);
        this->dstIp = dstIp;
        this->dstPort = dstPort;
    }

    void update() { parseOsc(); }

    void parseOsc() {
        size_t size;
        if (0 < (size = Udp.parsePacket())) {
            // Serial.println(size);
            Udp.readBytes(oscBuffer, size);
            if (osc.parse(oscBuffer, size)) {
                uint8_t argCount = osc.getArgCount();
                // Serial.println("Good message: This should print.");
                Serial.printf("OSC: address: %s\n", osc.getAddress());
                // Serial.printf("  Arg count: %d\n", argCount);
                for (uint8_t i = 0; i < argCount; i++) {
                    Serial.printf("param(%d): '%c'\n", i, osc.getTag(i));
                }
                if (osc.match(0, "/rst")) {
                    if (rstCallback) {
                        (*rstCallback)();
                    }
                } else if (osc.match(0, "/ping")) {
                    bool bMatch = true;
                    for (uint8_t i = 0; i < 4; i++) {
                        if (dstIp[i] != Udp.remoteIP()[i]) {
                            bMatch = false;
                        }
                    }
                    osc.init("/pong");
                    osc.addInt(bMatch);
                    size_t size = osc.getMessageSize();
                    Udp.beginPacket(Udp.remoteIP(), dstPort);
                    Udp.write(osc.getMessageBuf(), size);
                    Udp.endPacket();
                } else if (osc.match(0, "/setDstIp")) {
                    bool bMatch = true;
                    for (uint8_t i = 0; i < 4; i++) {
                        if (dstIp[i] != Udp.remoteIP()[i]) {
                            bMatch = false;
                        }
                    }
                    this->dstIp = Udp.remoteIP();
                    osc.init("/replyDstIp");
                    osc.addInt(bMatch);
                    size_t size = osc.getMessageSize();
                    Udp.beginPacket(dstIp, dstPort);
                    Udp.write(osc.getMessageBuf(), size);
                    Udp.endPacket();
                } else if (osc.match(0, "/ledTest")) {
                    if (ledTestCallback) {
                        ledTestCallback();
                    }
                } else if (osc.match(0, "/print")) {
                    if (printCallback) {
                        printCallback();
                    }
                } else if (osc.match(0, "/clearConfig")) {
                    if (clearConfigCallback) {
                        clearConfigCallback();
                    }
                } else {
                    char source[64];
                    char* result[8];

                    strcpy(source, osc.getAddress());
                    size_t resultSize = split(source, "/", result, 8);

                    // Serial.printf("src: %s\n", source);
                    // Serial.printf("size: %d\n", resultSize);
                    // for (size_t i = 0; i < resultSize; ++i) {
                    //     Serial.printf("%d: %s\n", i, result[i]);
                    // }

                    if (osc.match(0, "/config")) {
                        // Serial.println("match /config");
                        if (3 <= resultSize) {
                            if (argCount == 1) {
                                if (osc.isInt(0)) {
                                    if (configCallbackInt) {
                                        configCallbackInt(result[1], result[2],
                                                          osc.getInt(0));
                                    }
                                } else if (osc.isFloat(0)) {
                                    if (configCallbackFloat) {
                                        configCallbackFloat(result[1],
                                                            result[2],
                                                            osc.getFloat(0));
                                    }
                                }
                            } else if (argCount == 4) {
                                if (osc.isInt(0) && osc.isInt(1) &&
                                    osc.isInt(2) && osc.isInt(3)) {
                                    if (configCallbackIpaddress) {
                                        configCallbackIpaddress(
                                            result[1], result[2],
                                            IPAddress(
                                                osc.getInt(0), osc.getInt(1),
                                                osc.getInt(2), osc.getInt(3)));
                                    }
                                }
                            } else if (argCount == 6) {
                                if (osc.isInt(0) && osc.isInt(1) &&
                                    osc.isInt(2) && osc.isInt(3) &&
                                    osc.isInt(4) && osc.isInt(5)) {
                                    if (configCallbackIntArray) {
                                        int arr[] = {
                                            osc.getInt(0), osc.getInt(1),
                                            osc.getInt(2), osc.getInt(3),
                                            osc.getInt(4), osc.getInt(5)};
                                        configCallbackIntArray(
                                            result[1], result[2], arr, 6);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    void setRstCallback(void (*fptr)()) { rstCallback = fptr; }
    void setLedTestCallback(void (*fptr)()) { ledTestCallback = fptr; }
    void setPrintCallback(void (*fptr)()) { printCallback = fptr; }
    void setClearConfigCallback(void (*fptr)()) { clearConfigCallback = fptr; }

    void setConfigCallbackInt(void (*fptr)(const char* category,
                                           const char* name, int value)) {
        configCallbackInt = fptr;
    }
    void setConfigCallbackIntArray(void (*fptr)(const char* category,
                                                const char* name, int* value,
                                                size_t size)) {
        configCallbackIntArray = fptr;
    }
    void setConfigCallbackIpaddress(void (*fptr)(const char* category,
                                                 const char* name,
                                                 IPAddress value)) {
        configCallbackIpaddress = fptr;
    }
    void setConfigCallbackFloat(void (*fptr)(const char* category,
                                             const char* name, float value)) {
        configCallbackFloat = fptr;
    }
    void sendConfigReply(const char* category, const char* name, bool result) {
        char buf[64];
        const char* separator = "/";
        strcpy(buf, "/config/");
        strcat(buf, category);
        strcat(buf, separator);
        strcat(buf, name);

        osc.init(buf);
        osc.addInt((int)result);
        size_t size = osc.getMessageSize();

        Udp.beginPacket(dstIp, dstPort);
        Udp.write(osc.getMessageBuf(), size);
        Udp.endPacket();  // mark the end of the OSC Packet
    }

    size_t split(char* source, const char* separator, char** result,
                 size_t resultSize) {
        if ((source == NULL) || (separator == NULL) || (result == NULL) ||
            (resultSize <= 0)) {
            return 0;
        };
        size_t i = 0;

        char* p = strtok(source, separator);
        while (p != NULL && i < resultSize) {
            result[i] = p;
            ++i;
            p = strtok(NULL, separator);
        }

        return i;
    }
};
}  // namespace apc
#endif