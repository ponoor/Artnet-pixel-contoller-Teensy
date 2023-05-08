#ifndef APC_LED_H
#define APC_LED_H
#include <Arduino.h>
#include <OctoWS2811.h>

#include "ApcVariables.h"

namespace apc {
DMAMEM int displayMemory[variables::maxPixels * 6];
int drawingMemory[variables::maxPixels * 6];

// OctoWS2811 leds(
//             variables::maxPixels, 
//             displayMemory, 
//             drawingMemory, 
//             WS2811_GRB | WS2811_800kHz, 
//             variables::numStrips, 
//             pinList);
class Led {
   private:
    OctoWS2811* leds;
    
    // int drawingMemory[variables::maxPixels * 6];
    // DMAMEM int * displayMemory;
    // int * drawingMemory;
    int numLeds;
    int numLedsPerStrip;
   public:
    Led(){};
    ~Led(){
        // delete[] displayMemory;
        // delete[] drawingMemory;
        delete leds;
    };
    void init(int numPixels, uint8_t colorOrder, uint8_t protocol) {
        // displayMemory = new int[numPixels * 6];
        // drawingMemory = new int[numPixels * 6];
        numLeds = numPixels * variables::numStrips;
        numLedsPerStrip = numPixels;
        leds = new OctoWS2811(
            numPixels, 
            displayMemory, 
            drawingMemory, 
            colorOrder | protocol, 
            variables::numStrips, 
            variables::ledPinList);
        leds->begin();
    }
    void initTest() {
        // Serial.println("led test !!!");
        for (int i = 0; i < numLeds; i++) {
            leds->setPixel(i, 127, 0, 0);
        }
        leds->show();
        // Serial.println("R !!!");
        delay(500);
        for (int i = 0; i < numLeds; i++) {
            leds->setPixel(i, 0, 127, 0);
        }
        leds->show();
        // Serial.println("G !!!");
        delay(500);
        for (int i = 0; i < numLeds; i++) {
            leds->setPixel(i, 0, 0, 127);
        }
        leds->show();
        // Serial.println("B !!!");
        delay(500);
        for (int i = 0; i < numLeds; i++) {
            leds->setPixel(i, 0, 0, 0);
        }
        leds->show();
    }
    
    void setPixels(uint8_t stripIndex, uint16_t startIndex, uint8_t* data, uint16_t length) {
        for (int i = 0; i < length / 3; i++) {
            uint16_t index = stripIndex * numLedsPerStrip + startIndex + i;
            if (index < numLeds && startIndex + i < numLedsPerStrip) {
                leds->setPixel(index, data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
            }
        }
    }
    void show() {
        if (leds->busy()) {
            Serial.println("busy !!!");
        } else {
            leds->show(); 
        }
    }
};
}  // namespace apc

#endif  // APC_LED_H