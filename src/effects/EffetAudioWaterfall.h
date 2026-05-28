#pragma once

#include "../setting.h"
#include "Effect.h"
#include "../mic/MicManager.h"

// ─── Visualiseur audio — waterfall (spectrogramme défilant) ──────────────────
// Chaque colonne = une bande de fréquence (basses à gauche, aigus à droite).
// Chaque ligne = un instant dans le temps. Le spectre actuel apparaît en haut
// et défile vers le bas. Palette construite autour de la couleur choisie via
// setColor() : noir → teinte-15° → teinte → teinte+15° → blanc.

class EffetAudioWaterfall : public Effect {
	public:
		EffetAudioWaterfall() {
			memset(_history, 0, sizeof(_history));
			// Palette par défaut (cyan) — au cas où setColor() ne serait jamais appelée
			_buildPalette(CRGB(0, 200, 255));
		}

		String name() override { return "Waterfall"; }

		void setColor(CRGB color) override { _buildPalette(color); }

		void step(CRGB* leds) override {
			MicManager* mic = MicManager::getInstance();

			// Décale tout vers le bas — la ligne du bas disparaît
			for (uint8_t y = MATRIX_H - 1; y > 0; y--) {
				for (uint8_t x = 0; x < MATRIX_W; x++) {
					_history[y][x] = _history[y - 1][x];
				}
			}

			// Nouvelle ligne en haut (index 0) = spectre actuel
			for (uint8_t x = 0; x < MATRIX_W; x++) {
				_history[0][x] = mic->getBand(x);
			}

			// Rendu — index 0 du buffer s'affiche tout en haut
			for (uint8_t y = 0; y < MATRIX_H; y++) {
				for (uint8_t x = 0; x < MATRIX_W; x++) {
					uint8_t v = (uint8_t)(_history[y][x] * 255);
					leds[XY(x, y)] = ColorFromPalette(_palette, v);
				}
			}
		}

		void stepStripsTop(CRGB* strip, uint8_t len) override {
			MicManager* mic = MicManager::getInstance();
			// Pulse beat — couleur vive de la palette (index 240)
			float beat = mic->getBeatLevel();
			CRGB c = ColorFromPalette(_palette, 240);
			c.nscale8((uint8_t)(beat * 255));
			for (uint8_t i = 0; i < len; i++) strip[i] = c;
		}

		void stepStripsBot(CRGB* strip, uint8_t len) override {
			MicManager* mic = MicManager::getInstance();
			// VU global, coloré par la palette (cohérence visuelle)
			float avg = 0;
			for (uint8_t i = 0; i < MATRIX_W; i++) avg += mic->getBand(i);
			avg /= MATRIX_W;
			uint8_t filled = (uint8_t)(avg * len);
			for (uint8_t i = 0; i < len; i++) {
				if (i < filled) {
					uint8_t v = (uint8_t)((float)i / len * 255);
					strip[i] = ColorFromPalette(_palette, v);
				} else {
					strip[i] = CRGB::Black;
				}
			}
		}

	private:
		float          _history[MATRIX_H][MATRIX_W];
		CRGBPalette16  _palette;

		// Construit une palette : noir → teinte-15° sombre → teinte → teinte+15° → blanc
		// Hue FastLED : 256 unités pour 360° → 15° ≈ 11 unités
		void _buildPalette(CRGB color) {
			CHSV    hsv     = rgb2hsv_approximate(color);
			uint8_t hue     = hsv.hue;
			uint8_t hueLow  = hue - 35;  // teinte-15° (overflow naturel)
			uint8_t hueHigh = hue + 35;  // teinte+15°

			CRGB noir  = CRGB::Black;
			CRGB dim   = CHSV(hueLow,  255,  80);   // teinte-15° très sombre
			CRGB mid   = CHSV(hue,     255, 200);   // teinte de base, vif
			CRGB vif   = CHSV(hueHigh, 255, 255);   // teinte+15° plein
			CRGB pale  = CHSV(hueHigh, 100, 255);   // teinte+15° désaturée
			CRGB blanc = CRGB::White;

			_palette = CRGBPalette16(
				noir, noir, dim,  dim,
				mid,  mid,  mid,  vif,
				vif,  vif,  pale, pale,
				blanc, blanc, blanc, blanc
			);
		}
};
