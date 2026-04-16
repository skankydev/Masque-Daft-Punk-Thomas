#pragma once
#include "Effect.h"

class EffetCylon : public Effect {

    int16_t _pos;
    int8_t  _dir;
    CRGB    _color;

    static const uint8_t ROW_A = MATRIX_H / 2 - 1; // row 3 sur 8
    static const uint8_t ROW_B = MATRIX_H / 2;      // row 4 sur 8

public:
    EffetCylon() : _pos(0), _dir(1), _color(CRGB::Red) {}

    void setColor(CRGB color) override { _color = color; }

    void reset() override {
        _pos = 0;
        _dir = 1;
    }

    void step(CRGB* leds) override {
        fill_solid(leds, NUM_LEDS, CRGB::Black);

        // Ligne horizontale pleine largeur (2px au milieu)
        CRGB dim = _color; dim.nscale8(60);
        for (uint8_t x = 0; x < MATRIX_W; x++) {
            leds[XY(x, ROW_A)] = dim;
            leds[XY(x, ROW_B)] = dim;
        }

        // Blob vertical 3 colonnes de large, pleine hauteur
        CRGB med = _color; med.nscale8(130);
        for (uint8_t y = 0; y < MATRIX_H; y++) {
            if (_pos - 1 >= 0)        leds[XY(_pos - 1, y)] = med;
            if (_pos >= 0 && _pos < MATRIX_W) leds[XY(_pos, y)] = _color;
            if (_pos + 1 < MATRIX_W)  leds[XY(_pos + 1, y)] = med;
        }

        _pos += _dir;
        if (_pos >= MATRIX_W - 1 || _pos <= 0) _dir = -_dir;
    }

    String name() override { return "Cylon"; }
};
