#pragma once
#include "Effect.h"

class EffetPacmanGame : public Effect {

		static const uint8_t SPRITE  = 8;
		static const uint8_t DOT_ROW = MATRIX_H / 2;

		// Sprites Pac-Man : [0]=bouche ouverte, [1]=bouche fermée
		static const bool _pacOpen[SPRITE][SPRITE];
		static const bool _pacClosed[SPRITE][SPRITE];

		// Sprite fantôme : 0=noir, 1=rouge, 2=blanc (yeux)
		static const uint8_t _ghost[SPRITE][SPRITE];

		bool    _dots[MATRIX_W];
		int16_t _pacX;
		int16_t _ghostX;
		bool    _mouthOpen;
		uint8_t _mouthTimer;
		uint8_t _phase;      // 0 = droite (sans fantôme), 1 = gauche (avec fantôme)

		void _initDots() {
			for (uint8_t x = 0; x < MATRIX_W; x++) {
				_dots[x] = (x % 3 == 1); // un point tous les 3 colonnes
			}
		}

		void _eatDots() {
			for (uint8_t dx = 0; dx < SPRITE; dx++) {
				int16_t mx = _pacX + dx;
				if (mx >= 0 && mx < MATRIX_W) _dots[mx] = false;
			}
		}

		void _drawPac(CRGB* leds, bool mirrorX) {
			const bool (*sprite)[SPRITE] = _mouthOpen ? _pacOpen : _pacClosed;
			for (uint8_t y = 0; y < SPRITE; y++) {
				for (uint8_t sx = 0; sx < SPRITE; sx++) {
					int16_t mx = _pacX + sx;
					if (mx < 0 || mx >= MATRIX_W) continue;
					uint8_t col = mirrorX ? (SPRITE - 1 - sx) : sx;
					if (sprite[y][col]) leds[XY(mx, y)] = CRGB::Yellow;
				}
			}
		}

		void _drawGhost(CRGB* leds) {
			for (uint8_t y = 0; y < SPRITE; y++) {
				for (uint8_t sx = 0; sx < SPRITE; sx++) {
					int16_t mx = _ghostX + sx;
					if (mx < 0 || mx >= MATRIX_W) continue;
					uint8_t v = _ghost[y][sx];
					if      (v == 1) leds[XY(mx, y)] = CRGB::Red;
					else if (v == 2) leds[XY(mx, y)] = CRGB::White;
				}
			}
		}

	public:
		EffetPacmanGame() { reset(); }

		void reset() override {
			_pacX      = -SPRITE;
			_ghostX    = MATRIX_W + 14;
			_mouthOpen = true;
			_mouthTimer = 0;
			_phase     = 0;
			_initDots();
		}

		void step(CRGB* leds) override {
			fill_solid(leds, NUM_LEDS, CRGB::Black);

			// Animation bouche (toggle toutes les 4 frames)
			if (++_mouthTimer >= 4) {
				_mouthOpen  = !_mouthOpen;
				_mouthTimer = 0;
			}

			// Déplacement
			if (_phase == 0) {
				_pacX++;
				_eatDots();
				if (_pacX > MATRIX_W) {
					// Sort à droite → revient à gauche poursuivi
					_phase  = 1;
					_pacX   = MATRIX_W;
					_ghostX = MATRIX_W + 14;
				}
			} else {
				_pacX--;
				_ghostX--;
				if (_ghostX < -SPRITE) {
					// Fantôme sorti → reset
					_phase = 0;
					_pacX  = -SPRITE;
					_initDots();
				}
			}

			// Points
			for (uint8_t x = 0; x < MATRIX_W; x++) {
				if (_dots[x]) leds[XY(x, DOT_ROW)] = CRGB(180, 180, 0);
			}

			// Fantôme (phase 1 seulement)
			if (_phase == 1) _drawGhost(leds);

			// Pac-Man (miroir en phase 1 → face gauche)
			_drawPac(leds, _phase == 1);
		}

		void stepStripsTop(CRGB* strip, uint8_t len) override {
			// Pac-Man sur la strip — LED jaune qui suit sa position X
			fill_solid(strip, len, CRGB::Black);
			int16_t clampedX = constrain(_pacX, 0, MATRIX_W - 1);
			uint8_t pos = clampedX * (len - 1) / (MATRIX_W - 1);
			strip[pos] = CRGB::Yellow;
			if (pos > 0)       strip[pos - 1] = CRGB(80, 80, 0);
			if (pos < len - 1) strip[pos + 1] = CRGB(80, 80, 0);
		}

		void stepStripsBot(CRGB* strip, uint8_t len) override {
			// Bleu arcade en phase 0, rouge quand le fantôme chasse
			CRGB c = (_phase == 0) ? CRGB(0, 0, 80) : CRGB(80, 0, 0);
			fill_solid(strip, len, c);
		}

		String name() override { return "PacmanGame"; }
};

// ─── Sprites ─────────────────────────────────────────────────────────────────

// Pac-Man bouche ouverte, face droite
const bool EffetPacmanGame::_pacOpen[8][8] = {
	{ 0, 0, 1, 1, 1, 1, 0, 0 },
	{ 0, 1, 1, 1, 1, 1, 1, 0 },
	{ 1, 1, 0, 1, 1, 1, 0, 0 },
	{ 1, 1, 1, 1, 1, 0, 0, 0 },
	{ 1, 1, 1, 1, 0, 0, 0, 0 },
	{ 1, 1, 1, 1, 1, 0, 0, 0 },
	{ 0, 1, 1, 1, 1, 1, 1, 0 },
	{ 0, 0, 1, 1, 1, 1, 0, 0 },
};

// Pac-Man bouche fermée
const bool EffetPacmanGame::_pacClosed[8][8] = {
	{ 0, 0, 1, 1, 1, 1, 0, 0 },
	{ 0, 1, 1, 1, 1, 1, 1, 0 },
	{ 1, 1, 0, 1, 1, 1, 1, 1 },
	{ 1, 1, 1, 1, 1, 1, 1, 1 },
	{ 1, 1, 1, 1, 1, 1, 1, 1 },
	{ 1, 1, 1, 1, 1, 1, 1, 1 },
	{ 0, 1, 1, 1, 1, 1, 1, 0 },
	{ 0, 0, 1, 1, 1, 1, 0, 0 },
};

// Fantôme rouge (1=rouge, 2=blanc yeux, 0=noir)
const uint8_t EffetPacmanGame::_ghost[8][8] = {
	{ 0, 0, 1, 1, 1, 1, 0, 0 },
	{ 0, 1, 1, 1, 1, 1, 1, 0 },
	{ 1, 2, 2, 1, 1, 2, 2, 1 },
	{ 1, 2, 2, 1, 1, 2, 2, 1 },
	{ 1, 1, 1, 1, 1, 1, 1, 1 },
	{ 1, 1, 1, 1, 1, 1, 1, 1 },
	{ 1, 0, 1, 1, 1, 1, 0, 1 },
	{ 1, 0, 1, 0, 0, 1, 0, 1 },
};
