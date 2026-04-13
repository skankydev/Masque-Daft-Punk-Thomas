#pragma once
#include "Effect.h"

class EffetCylon : public Effect {

    int16_t _pos;
    int8_t  _dir;

    void _drawBlob(CRGB* leds, int16_t x) {
        for (uint8_t y = 0; y < MATRIX_H; y++) {
            // Centre : orange vif
            if (x >= 0 && x < MATRIX_W)
                leds[XY(x, y)] = CRGB(255, 63, 0);
            // ±1 : rouge moyen
            if (x - 1 >= 0)
                leds[XY(x - 1, y)] = CRGB(200, 0, 0);
            if (x + 1 < MATRIX_W)
                leds[XY(x + 1, y)] = CRGB(200, 0, 0);
            // ±2 : rouge dim
            if (x - 2 >= 0)
                leds[XY(x - 2, y)] = CRGB(70, 0, 0);
            if (x + 2 < MATRIX_W)
                leds[XY(x + 2, y)] = CRGB(70, 0, 0);
        }
    }

public:
    EffetCylon() : _pos(0), _dir(1) {}

    void reset() override {
        _pos = 0;
        _dir = 1;
    }

    void step(CRGB* leds) override {
        fill_solid(leds, NUM_LEDS, CRGB::Black);
        _drawBlob(leds, _pos);

        _pos += _dir;
        if (_pos >= MATRIX_W - 1 || _pos <= 0) {
            _dir = -_dir;
        }
    }

    String name() override { return "Cylon"; }
};
