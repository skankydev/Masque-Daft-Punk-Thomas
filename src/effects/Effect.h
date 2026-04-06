#pragma once

#include <FastLED.h>
#include <Arduino.h>
#include "../setting.h"

// Fonction libre XY — accessible par tous les effets
inline uint16_t XY(uint8_t x, uint8_t y) {
    if (y & 0x01) {
        return (y * MATRIX_W) + (MATRIX_W - 1 - x);
    }
    return (y * MATRIX_W) + x;
}

class Effect {
public:
    virtual void   step(CRGB* leds) = 0;
    virtual String name()            = 0;
    virtual void   reset()          {}
    virtual ~Effect() {}
};
