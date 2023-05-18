#ifndef APC_LED_SERIAL_H
#define APC_LED_SERIAL_H
#include <Arduino.h>
#include <WS2812Serial.h>

#include "ApcVariables.h"

namespace apc {
DMAMEM int displayMemory[variables::numStrips][variables::maxPixels * 3];
int drawingMemory[variables::numStrips][variables::maxPixels * 12];

// OctoWS2811 leds(
//             variables::maxPixels, 
//             displayMemory, 
//             drawingMemory, 
//             WS2811_GRB | WS2811_800kHz, 
//             variables::numStrips, 
//             pinList);
class LedSerial {
   private:
    WS2812Serial* leds[variables::numStrips];
    
    // int drawingMemory[variables::maxPixels * 6];
    // DMAMEM int * displayMemory;
    // int * drawingMemory;
    
    int numLeds;
    int numLedsPerStrip;
   public:
    LedSerial(){};
    ~LedSerial(){
        // delete[] displayMemory;
        // delete[] drawingMemory;
        // delete leds;
        for (int i = 0; i < variables::numStrips; i++) {
            delete leds[i]; 
        }
    };
    void init(int numPixels, uint8_t colorOrder, uint8_t protocol) {
        // displayMemory = new int[numPixels * 6];
        // drawingMemory = new int[numPixels * 6];
        
        numLeds = numPixels * variables::numStrips;
        numLedsPerStrip = numPixels;

        for (int i = 0; i < variables::numStrips; i++) {
            leds[i] = new WS2812Serial(
                numPixels, 
                displayMemory[i], 
                drawingMemory[i], 
                variables::ledPinList[i], 
                colorOrder),
            leds[i]->begin();
        }
    }
    void initTest() {
    for (int h = 0; h < variables::numStrips; h++) {
        for (int i = 0; i < numLedsPerStrip; i++) {
            leds[h]->setPixel(i, 127, 0, 0);
        }
        leds[h]->show();
    }
    delay(500);
    for (int h = 0; h < variables::numStrips; h++) {
        for (int i = 0; i < numLedsPerStrip; i++) {
            leds[h]->setPixel(i, 0, 127, 0);
        }
        leds[h]->show();
    }
    delay(500);
    for (int h = 0; h < variables::numStrips; h++) {
        for (int i = 0; i < numLedsPerStrip; i++) {
            leds[h]->setPixel(i, 0, 0, 127);
        }
        leds[h]->show();
    }
    delay(500);
    for (int h = 0; h < variables::numStrips; h++) {
        for (int i = 0; i < numLedsPerStrip; i++) {
            leds[h]->setPixel(i, 0, 0, 0);
        }
        leds[h]->show();
    }
}
    
    void setPixels(uint8_t stripIndex, uint16_t startIndex, uint8_t* data, uint16_t length) {
        for (int i = 0; i < length / 3; i++) {

            int pinLedIndex = startIndex + i;
        // int ledIndex = pinIndex * ledsPerStrip + pinLedIndex;
            if (pinLedIndex < numLeds) {
                leds[stripIndex]->setPixel(pinLedIndex, data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
            }
        }
    }
    void show() {
        for (int i = 0; i < variables::numStrips; i++) {
            if (leds[i]->busy()) {
                // Serial.printf("led[%d] busy !!!\n", i);
            } else {
                leds[i]->show();
            }
        }
    }
};
}  // namespace apc

#endif  // APC_LED_H