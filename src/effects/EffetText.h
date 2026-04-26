#pragma once
#include "Effect.h"
#include "../assets/font5x7.h"

class EffetText : public Effect {

		String   _text;
		CRGB     _color;
		uint16_t _offset;   // colonne courante du scroll (0 = texte hors écran à droite)

	public:
		EffetText()
			: _text("Buy it, use it, break it, fix it"),
			  _color(CRGB::Red),
			  _offset(0)
		{}

		void reset() override {
			_offset = 0;
		}

		void setText(String text) override {
			_text   = text;
			_offset = 0;
		}

		void setColor(CRGB color) override {
			_color = color;
		}

		void step(CRGB* leds) override {
			fill_solid(leds, NUM_LEDS, CRGB::Black);

			int totalWidth = _text.length() * 6; // 5px caractère + 1px espace

			for (uint8_t x = 0; x < MATRIX_W; x++) {
				// Colonne du texte correspondant à cette colonne de la matrice
				int textCol = (int)_offset + x - MATRIX_W;

				if (textCol < 0 || textCol >= totalWidth) continue;

				uint8_t charIdx  = textCol / 6;
				uint8_t colInChar = textCol % 6;

				if (colInChar >= 5) continue; // colonne d'espacement entre caractères

				char c = _text.charAt(charIdx);
				if (c < 0x20 || c > 0x7E) continue;

				// Chaque byte = une colonne, bit 0 = ligne du haut
				uint8_t fontByte = pgm_read_byte(&Font5x7[(c - 0x20) * 5 + colInChar]);

				for (uint8_t y = 0; y < 7; y++) {
					if (fontByte & (1 << y)) {
						uint8_t py = y + 1; // décalage vertical d'1px vers le bas
						if (py < MATRIX_H) leds[XY(x, py)] = _color;
					}
				}
			}

			// Avance d'une colonne, boucle quand le texte est sorti à gauche
			_offset++;
			if (_offset > (uint16_t)(MATRIX_W + totalWidth)) {
				_offset = 0;
			}
		}

		String name() override { return "Text"; }
};
