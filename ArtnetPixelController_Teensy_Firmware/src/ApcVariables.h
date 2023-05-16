#ifndef APC_VARIABLES_H
#define APC_VARIABLES_H
#include <Arduino.h>
#include <OctoWS2811.h>

namespace apc {
    namespace variables {
        namespace defaultSettings {
            namespace network {
                static const IPAddress ip(10, 0, 1, 0);
                static const IPAddress gateway(10, 0, 0, 1);
                static const IPAddress subnet(255, 255, 0, 0);
                static const uint8_t mac[6] = {0x04, 0xE4, 0xE6, 0x04, 0x68, 0x00};
                static const uint8_t ipAddId = 1;
                static const uint8_t macAddId = 1;
                
            }
            namespace artnet {
                static const int port = 6454;
                static const int net = 0;
                static const int subnet = 0;
                static const int universe = 0;
                static const float fps = 44.0f;
                static const uint8_t forceSync = 0;
            }
            namespace osc {
                static const int dstPort = 50000;
                static const IPAddress dstIp(10, 0, 0, 10);
                static const int srcPort = 50000;
                static const uint8_t dstPortAddId = 0;
            }
            namespace led {
                static const int numPixels = 680;
                static const uint8_t protocol = WS2811_800kHz;
                static const uint8_t colorOrder = WS2811_GRB;
                static const uint8_t initTest = 1;
            }
        }
        static const int numStrips = 8;
        static const uint8_t ledPinList[8] = {35, 29, 24, 20, 17, 14, 8, 1};
        static const uint8_t dipPinList[8] = {33, 34, 41, 36, 37, 38, 39, 40};
        static const int maxPixels = 680;
    }
} // namespace apc

#endif // APC_VARIABLES_H