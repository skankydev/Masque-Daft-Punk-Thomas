#pragma once
#include "Effect.h"

class EffetVisorFire : public Effect {

    uint8_t _canvas[MATRIX_W * MATRIX_H];

    // Palette 100 couleurs : noir → rouge → orange → jaune
    // Portée depuis le projet DPTv2 (sans l'application de brightness, FastLED s'en charge)
    static const CRGB _palette[100];

public:
    EffetVisorFire() {
        memset(_canvas, 0, sizeof(_canvas));
    }

    void reset() override {
        memset(_canvas, 0, sizeof(_canvas));
    }

    void step(CRGB* leds) override {
        const unsigned int heat  = MATRIX_W / 5;
        const unsigned int focus = 9;
        const unsigned int cool  = 26;

        unsigned int i, c, n, x, y;

        // Étape 1 : remonte tout le canvas d'une ligne
        memmove(_canvas + MATRIX_W, _canvas, MATRIX_W * (MATRIX_H - 1));
        memset(_canvas, 0, MATRIX_W);

        // Étape 2 : points chauds aléatoires sur la ligne du bas
        i = heat;
        if (i > MATRIX_W - 8) i = MATRIX_W - 8;
        while (i > 0) {
            x = random(MATRIX_W - 2) + 1;
            if (_canvas[x] == 0) {
                _canvas[x] = 99;
                i--;
            }
        }

        // Étape 3 : interpolation + refroidissement
        for (y = 0; y < MATRIX_H; y++) {
            for (x = 0; x < MATRIX_W; x++) {
                c = _canvas[y * MATRIX_W + x] * focus;
                n = focus;
                if (x > 0)            { c += _canvas[y * MATRIX_W + (x - 1)]; n++; }
                if (x < MATRIX_W - 1) { c += _canvas[y * MATRIX_W + (x + 1)]; n++; }
                if (y > 0)            { c += _canvas[(y - 1) * MATRIX_W + x]; n++; }
                if (y < MATRIX_H - 1) { c += _canvas[(y + 1) * MATRIX_W + x]; n++; }
                c = (c + (n / 2)) / n;
                i = (random(1000) * cool) / 10000;
                _canvas[y * MATRIX_W + x] = (c > i) ? c - i : 0;
            }
        }

        // Étape 4 : rendu — canvas[0] = bas chaud, inversé sur la matrice
        for (y = 0; y < MATRIX_H; y++) {
            for (x = 0; x < MATRIX_W; x++) {
                c = _canvas[((MATRIX_H - 1) - y) * MATRIX_W + x];
                leds[XY(x, y)] = _palette[c];
            }
        }
    }

    String name() override { return "VisorFire"; }
};

// Palette exacte du projet DPTv2 (100 niveaux, noir → rouge → orange → jaune)
const CRGB EffetVisorFire::_palette[100] = {
    CRGB(  0,  0,  0), CRGB(  1,  0,  0), CRGB(  2,  0,  0), CRGB(  3,  0,  0),
    CRGB(  5,  0,  0), CRGB(  7,  0,  0), CRGB(  9,  0,  0), CRGB( 11,  0,  0),
    CRGB( 13,  0,  0), CRGB( 15,  0,  0), CRGB( 18,  0,  0), CRGB( 20,  0,  0),
    CRGB( 23,  0,  0), CRGB( 26,  0,  0), CRGB( 28,  0,  0), CRGB( 31,  0,  0),
    CRGB( 34,  0,  0), CRGB( 37,  0,  0), CRGB( 41,  0,  0), CRGB( 44,  0,  0),
    CRGB( 47,  0,  0), CRGB( 52,  0,  0), CRGB( 55,  0,  0), CRGB( 59,  0,  0),
    CRGB( 63,  0,  0), CRGB( 66,  0,  0), CRGB( 70,  0,  0), CRGB( 74,  0,  0),
    CRGB( 78,  0,  0), CRGB( 83,  0,  0), CRGB( 88,  0,  0), CRGB( 92,  0,  0),
    CRGB( 96,  0,  0), CRGB(100,  0,  0), CRGB(105,  0,  0), CRGB(109,  0,  0),
    CRGB(114,  0,  0), CRGB(119,  0,  0), CRGB(123,  0,  0), CRGB(128,  0,  0),
    CRGB(133,  0,  0), CRGB(138,  0,  0), CRGB(142,  0,  0), CRGB(147,  0,  0),
    CRGB(152,  0,  0), CRGB(158,  0,  0), CRGB(164,  0,  0), CRGB(169,  0,  0),
    CRGB(174,  0,  0), CRGB(180,  0,  0), CRGB(184,  0,  0), CRGB(187,  1,  0),
    CRGB(189,  2,  0), CRGB(192,  3,  0), CRGB(195,  5,  0), CRGB(197,  6,  0),
    CRGB(200,  8,  0), CRGB(203, 10,  0), CRGB(206, 13,  0), CRGB(209, 15,  0),
    CRGB(211, 18,  0), CRGB(214, 20,  0), CRGB(217, 23,  0), CRGB(220, 26,  0),
    CRGB(223, 28,  0), CRGB(226, 31,  0), CRGB(228, 34,  0), CRGB(231, 37,  0),
    CRGB(234, 41,  0), CRGB(237, 44,  0), CRGB(240, 47,  0), CRGB(245, 52,  0),
    CRGB(248, 55,  0), CRGB(251, 59,  0), CRGB(254, 63,  0), CRGB(255, 67,  0),
    CRGB(255, 73,  0), CRGB(255, 79,  1), CRGB(255, 85,  1), CRGB(255, 93,  2),
    CRGB(255, 99,  2), CRGB(255,106,  3), CRGB(255,113,  4), CRGB(255,120,  4),
    CRGB(255,127,  5), CRGB(255,134,  6), CRGB(255,141,  7), CRGB(255,150,  8),
    CRGB(255,158,  9), CRGB(255,165, 10), CRGB(255,173, 11), CRGB(255,181, 12),
    CRGB(255,189, 13), CRGB(255,197, 14), CRGB(255,206, 15), CRGB(255,214, 16),
    CRGB(255,224, 18), CRGB(255,233, 20), CRGB(255,242, 21), CRGB(255,251, 22),
};
