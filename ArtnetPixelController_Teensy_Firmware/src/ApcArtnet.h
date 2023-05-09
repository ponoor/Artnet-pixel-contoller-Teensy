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
    u_int16_t net;
    uint8_t subnet;
    uint8_t universe;
    uint16_t startUniverse;
   public:
    Artnet(){};
    ~Artnet(){};
    void init(int port, IPAddress ip, IPAddress sub, uint16_t net=0, uint8_t subnet=0, uint8_t uni=0) {
        this->net = net;
        this->subnet = subnet;
        this->universe = universe;
        this->startUniverse = universe + 16 * subnet + 256 * net;
        artnet.setSocketSize(1024 * 16);
        // artnet.setBroadcast(variables::broadcastIP);
        artnet.setBroadcastAuto(ip, sub);
        artnet.begin(port);
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
            if (artSyncCallback) {
                (*artSyncCallback)(artnet.getRemoteIP());
            }
        }
    }
};
}  // namespace apc

#endif  // APC_ARTNET_H