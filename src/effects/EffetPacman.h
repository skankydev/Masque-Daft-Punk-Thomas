#pragma once
#include "Effect.h"

class EffetPacman : public Effect {

    // Pacman plein 8x8, face à droite, oeil inclus (0 = noir, 1 = jaune)
    static const bool _open[8][8];
    static const bool _closed[8][8];

    bool    _bouche;
    uint8_t _frameCount;
    uint8_t _interval;  // nombre de frames entre chaque toggle

public:
    EffetPacman() : _bouche(true), _frameCount(0), _interval(10) {}

    void reset() override {
        _bouche     = true;
        _frameCount = 0;
    }

    void step(CRGB* leds) override {
        if (++_frameCount >= _interval) {
            _bouche     = !_bouche;
            _frameCount = 0;
        }

        const bool (*frame)[8] = _bouche ? _open : _closed;

        fill_solid(leds, NUM_LEDS, CRGB::Black);

        for (uint8_t y = 0; y < 8; y++) {
            for (uint8_t x = 0; x < 8; x++) {
                if (frame[y][x]) {
                    leds[XY(x, y)] = CRGB::Yellow;
                }
            }
        }
    }

    String name() override { return "Pacman"; }
};

// Bouche ouverte — corps plein 8x8, wedge à droite, oeil en (2,2)
const bool EffetPacman::_open[8][8] = {
    { 0, 0, 1, 1, 1, 1, 0, 0 },
    { 0, 1, 1, 1, 1, 1, 1, 0 },
    { 1, 1, 0, 1, 1, 1, 0, 0 },
    { 1, 1, 1, 1, 1, 0, 0, 0 },
    { 1, 1, 1, 1, 0, 0, 0, 0 },
    { 1, 1, 1, 1, 1, 0, 0, 0 },
    { 0, 1, 1, 1, 1, 1, 1, 0 },
    { 0, 0, 1, 1, 1, 1, 0, 0 },
};

// Bouche fermée — cercle plein 8x8, oeil en (2,2)
const bool EffetPacman::_closed[8][8] = {
    { 0, 0, 1, 1, 1, 1, 0, 0 },
    { 0, 1, 1, 1, 1, 1, 1, 0 },
    { 1, 1, 0, 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 1, 1, 1, 1 },
    { 0, 1, 1, 1, 1, 1, 1, 0 },
    { 0, 0, 1, 1, 1, 1, 0, 0 },
};
