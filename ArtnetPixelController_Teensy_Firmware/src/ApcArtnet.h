#ifndef APC_ARTNET_H
#define APC_ARTNET_H
#include <Arduino.h>
#include <Artnet.h>

namespace apc {

class Artnet {
   private:
    ::Artnet artnet;
    void (*artDmxCallback)(uint16_t universe, uint16_t length, uint8_t sequence,
                           uint8_t* data, IPAddress remoteIP);
    void (*artSyncCallback)(IPAddress remoteIP);
    void (*nonSyncCallback)();

    u_int16_t net;
    uint8_t subnet;
    uint8_t universe;
    uint16_t startUniverse;
    bool bSync;
    bool bForceSync;
    uint32_t interval;
    uint32_t timestamp;
   public:
    Artnet(){};
    ~Artnet(){};
    void init(int port, IPAddress ip, IPAddress sub, uint16_t net=0, uint8_t subnet=0, uint8_t uni=0, float fps = 44.0f, bool forceSync = 0) {
        this->net = net;
        this->subnet = subnet;
        this->universe = universe;
        this->startUniverse = universe + 16 * subnet + 256 * net;
        this-> bForceSync = forceSync;
        interval = 1000000 / fps;
        artnet.setSocketSize(1024 * 16);
        // artnet.setBroadcast(variables::broadcastIP);
        artnet.setBroadcastAuto(ip, sub);
        artnet.begin(port);

        if (bForceSync) {
            bSync = true;
        } else {
            bSync = false;
        }
        // artnet.setArtDmxCallback(&Artnet::onDmxFrameListener);
        // artnet.setArtSyncCallback(&Artnet::onSyncListener);
    }
    void setArtDmxCallback(void (*fptr)(uint16_t universe, uint16_t length,
                                        uint8_t sequence, uint8_t* data,
                                        IPAddress remoteIP)) {
        artDmxCallback = fptr;
    }

    void setArtSyncCallback(void (*fptr)(IPAddress remoteIP)) {
        artSyncCallback = fptr;
    }
    void setNonSyncCallback(void (*fptr)()) {
        nonSyncCallback = fptr;
    }
    // void onDmxFrameListener(uint16_t universe, uint16_t length,
    //                         uint8_t sequence, uint8_t* data,
    //                         IPAddress remoteIP) {
    //     if (artDmxCallback) {
    //         (*artDmxCallback)(universe, length, sequence, data, remoteIP);
    //     }
    // }
    // void onSyncListener(IPAddress remoteIP) {
    //     if (artSyncCallback) {
    //         (*artSyncCallback)(remoteIP);
    //     }
    // }
    void update() {
        uint32_t currentTime = micros();
        uint16_t r = artnet.read();
        if (r == ART_POLL) {
            Serial.println("POLL");
        }
        if (r == ART_DMX) {
            int uni = artnet.getUniverse() - startUniverse;
            if (0 <= uni && uni < 32) {
                if (artDmxCallback) {
                    (*artDmxCallback)(
                        artnet.getUniverse() - startUniverse, 
                        artnet.getLength(), 
                        artnet.getSequence(), 
                        artnet.getDmxFrame(), 
                        artnet.getRemoteIP()
                    );
                }
            }
        }
        if (r == ART_SYNC) {
            bSync = true;
            timestamp = currentTime;
            if (artSyncCallback) {
                (*artSyncCallback)(artnet.getRemoteIP());
            }
        }
        if (!bForceSync) {
            if (bSync) {
                if ((uint32_t)4000000 <= currentTime - timestamp) {
                    bSync = false;
                }
            }

            if (!bSync) {
                if (interval <= currentTime - timestamp) {
                    timestamp = currentTime;
                    if (nonSyncCallback) {
                        nonSyncCallback();
                    }
                }
            }
        }
    }
};
}  // namespace apc

#endif  // APC_ARTNET_H