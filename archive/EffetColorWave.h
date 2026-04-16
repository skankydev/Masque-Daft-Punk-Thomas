#pragma once
#include "Effect.h"

class EffetColorWave : public Effect {
public:
    void step(CRGB* leds) override {
        uint8_t t = millis() >> 5;
        for (uint8_t y = 0; y < MATRIX_H; y++) {
            for (uint8_t x = 0; x < MATRIX_W; x++) {
                leds[XY(x, y)] = CHSV(t + (x + y) * 10, 240, 200);
            }
        }
    }

    String name() override { return "Color Wave"; }
};
