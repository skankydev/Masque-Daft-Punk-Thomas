#pragma once
#include "Effect.h"

class EffetFire : public Effect {
    uint8_t _heat[MATRIX_W][MATRIX_H] = {};

public:
    void reset() override {
        memset(_heat, 0, sizeof(_heat));
    }

    void step(CRGB* leds) override {
        // Refroidissement
        for (uint8_t x = 0; x < MATRIX_W; x++) {
            for (uint8_t y = 0; y < MATRIX_H; y++) {
                _heat[x][y] = qsub8(_heat[x][y], random8(0, 20));
            }
        }

        // Propagation vers le haut
        for (uint8_t x = 0; x < MATRIX_W; x++) {
            for (uint8_t y = MATRIX_H - 1; y > 0; y--) {
                _heat[x][y] = (_heat[x][y - 1] + _heat[x][max(0, y - 2)] * 2) / 3;
            }
        }

        // Ignition sur la ligne du bas
        for (uint8_t x = 0; x < MATRIX_W; x++) {
            if (random8() < 120) {
                _heat[x][0] = qadd8(_heat[x][0], random8(160, 255));
            }
        }

        // Rendu (ligne 0 = bas visuel)
        // scale8 plafonne à 200/255 → pas de blanc, max jaune
        for (uint8_t x = 0; x < MATRIX_W; x++) {
            for (uint8_t y = 0; y < MATRIX_H; y++) {
                leds[XY(x, MATRIX_H - 1 - y)] = HeatColor(scale8(_heat[x][y], 200));
            }
        }
    }

    String name() override { return "Fire"; }
};
