#pragma once

#include <FastLED.h>
#include <Arduino.h>
#include "../setting.h"

// Fonction libre XY — accessible par tous les effets
// Serpentin vertical : colonne paire haut→bas, colonne impaire bas→haut
inline uint16_t XY(uint8_t x, uint8_t y) {
	if (x & 0x01) {
		return (x * MATRIX_H) + (MATRIX_H - 1 - y);
	}
	return (x * MATRIX_H) + y;
}

class Effect {
	public:
		virtual void   step(CRGB* leds) = 0;
		virtual String name()            = 0;
		virtual void   reset()          {}
		// Paramètres optionnels — no-op par défaut, les effets qui en ont besoin les overrident
		virtual void   setText(String text)  {}
		virtual void   setColor(CRGB color)  {}
		virtual ~Effect() {}
};
