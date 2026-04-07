#pragma once
#include "Effect.h"

class EffetPacman : public Effect {

    // Bouche ouverte
    static const bool _open[7][7];
    // Bouche fermée
    static const bool _closed[7][7];

    bool          _bouche;
    unsigned long _lastBlink;
    uint16_t      _interval;

public:
    EffetPacman() : _bouche(true), _lastBlink(0), _interval(300) {}

    void reset() override {
        _bouche   = true;
        _lastBlink = 0;
    }

    void step(CRGB* leds) override {
        unsigned long now = millis();
        if (now - _lastBlink >= _interval) {
            _bouche    = !_bouche;
            _lastBlink = now;
        }

        const bool (*frame)[7] = _bouche ? _open : _closed;

        fill_solid(leds, NUM_LEDS, CRGB::Black);

        for (uint8_t y = 0; y < 7; y++) {
            for (uint8_t x = 0; x < 7; x++) {
                if (frame[y][x]) {
                    leds[XY(x, y)] = CRGB::Yellow;
                }
            }
        }
    }

    String name() override { return "Pacman"; }
};

// Bouche ouverte
const bool EffetPacman::_open[7][7] = {
    { 0, 0, 1, 1, 1, 1, 0 },
    { 0, 1, 0, 0, 1, 0, 0 },
    { 1, 0, 0, 1, 0, 0, 0 },
    { 1, 0, 1, 0, 0, 0, 0 },
    { 1, 0, 0, 1, 0, 0, 0 },
    { 0, 1, 0, 0, 1, 0, 0 },
    { 0, 0, 1, 1, 1, 1, 0 },
};

// Bouche fermée
const bool EffetPacman::_closed[7][7] = {
    { 0, 0, 1, 1, 1, 0, 0 },
    { 0, 1, 0, 0, 0, 1, 0 },
    { 1, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 1 },
    { 0, 1, 0, 0, 0, 1, 0 },
    { 0, 0, 1, 1, 1, 0, 0 },
};
