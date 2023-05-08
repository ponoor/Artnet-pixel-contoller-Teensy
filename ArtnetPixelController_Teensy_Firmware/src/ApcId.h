#ifndef APC_ID_H
#define APC_ID_H
#include <Arduino.h>

#include "ApcVariables.h"

namespace apc {
class Id {
private:
    uint8_t id;
public:
    Id(){};
    ~Id(){};

    void init() {
        for (uint8_t i = 0; i < 8; i++) {
            pinMode(variables::dipPinList[i], INPUT_PULLUP);
        }
    }
    uint8_t getId() {
        id = 0;
        for (uint8_t i = 0; i < 8; i++) {
            id += (!digitalRead(variables::dipPinList[i])) << i;
        }
        return id;
    }
};
}
#endif