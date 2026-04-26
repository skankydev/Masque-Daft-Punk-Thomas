#pragma once
#include "Effect.h"

class EffetKawaii : public Effect {

		// Sprites 7×6 — 0=noir, 1=pupille, 2=reflet blanc
		static const uint8_t SPR_OPEN  [6][7];
		static const uint8_t SPR_HALF  [6][7];
		static const uint8_t SPR_CLOSED[6][7];
		static const uint8_t SPR_HAPPY [6][7];

		enum Sprite { S_OPEN, S_HALF, S_CLOSED, S_HAPPY };
		enum Phase  { IDLE, BLINK, WINK_L, WINK_R, HAPPY };

		Phase   _phase;
		uint8_t _animStep;  // step dans l'animation courante
		uint8_t _idleTimer; // ticks avant prochain évènement
		int8_t  _lookX;     // décalage du regard : -1=gauche, 0=centre, 1=droite

		CRGB _pupilColor;

		void _drawEye(CRGB* leds, uint8_t cx, uint8_t cy, Sprite s, int8_t offX = 0) {
			const uint8_t (*spr)[7];
			switch (s) {
				case S_OPEN:   spr = SPR_OPEN;   break;
				case S_HALF:   spr = SPR_HALF;   break;
				case S_CLOSED: spr = SPR_CLOSED; break;
				case S_HAPPY:  spr = SPR_HAPPY;  break;
				default:       spr = SPR_OPEN;   break;
			}
			for (int8_t sy = 0; sy < 6; sy++) {
				for (int8_t sx = 0; sx < 7; sx++) {
					int8_t px = cx - 3 + sx + offX;
					int8_t py = cy - 3 + sy;
					if (px < 0 || px >= MATRIX_W || py < 0 || py >= MATRIX_H) continue;
					uint8_t v = spr[sy][sx];
					if      (v == 1) leds[XY(px, py)] = _pupilColor;
					else if (v == 2) leds[XY(px, py)] = CRGB::White;
				}
			}
		}

	public:
		EffetKawaii() : _pupilColor(CRGB(255, 20, 120)) { reset(); }

		void setColor(CRGB c) override { _pupilColor = c; }

		void reset() override {
			_phase     = IDLE;
			_animStep  = 0;
			_idleTimer = random(30, 80);
			_lookX     = 0;
		}

		void step(CRGB* leds) override {
			fill_solid(leds, NUM_LEDS, CRGB::Black);

			// Centres des yeux — adaptés à MATRIX_W=32, MATRIX_H=8
			const uint8_t LX = MATRIX_W / 4;         // ~8
			const uint8_t RX = MATRIX_W * 3 / 4 - 1; // ~23
			const uint8_t EY = MATRIX_H / 2;     // ~3

			Sprite sprL = S_OPEN, sprR = S_OPEN;
			int8_t offL = _lookX, offR = _lookX;

			switch (_phase) {

				case IDLE:
					if (--_idleTimer == 0) {
						uint8_t r = random(10);
						if      (r < 5) _phase = BLINK;
						else if (r < 7) _phase = WINK_L;
						else if (r < 9) _phase = WINK_R;
						else            _phase = HAPPY;
						_animStep  = 0;
						_idleTimer = random(30, 80);
					}
					// Changement de regard occasionnel
					if (random(100) == 0) _lookX = (int8_t)(random(3)) - 1;
					break;

				case BLINK:
					// open(0-1) → half(2) → closed(3) → half(4) → open(5-6) → IDLE
					if      (_animStep == 2) { sprL = sprR = S_HALF;   }
					else if (_animStep == 3) { sprL = sprR = S_CLOSED; }
					else if (_animStep == 4) { sprL = sprR = S_HALF;   }
					if (++_animStep > 6) _phase = IDLE;
					break;

				case WINK_L:
					// Œil gauche cligne, droit reste ouvert
					if      (_animStep == 1) sprL = S_HALF;
					else if (_animStep == 2) sprL = S_CLOSED;
					else if (_animStep == 3) sprL = S_HALF;
					if (++_animStep > 5) _phase = IDLE;
					break;

				case WINK_R:
					// Œil droit cligne, gauche reste ouvert
					if      (_animStep == 1) sprR = S_HALF;
					else if (_animStep == 2) sprR = S_CLOSED;
					else if (_animStep == 3) sprR = S_HALF;
					if (++_animStep > 5) _phase = IDLE;
					break;

				case HAPPY:
					sprL = sprR = S_HAPPY;
					offL = offR = 0; // pas de décalage en mode heureux
					if (++_animStep > 40) _phase = IDLE;
					break;
			}

			_drawEye(leds, LX, EY, sprL, offL);
			_drawEye(leds, RX, EY, sprR, offR);
		}

		String name() override { return "Kawaii"; }
};

// ── Sprites ───────────────────────────────────────────────────────────────────

const uint8_t EffetKawaii::SPR_OPEN[6][7] = {
	{ 0, 1, 1, 1, 1, 1, 0 },
	{ 1, 1, 1, 1, 1, 1, 1 },
	{ 1, 1, 2, 1, 1, 1, 1 }, // reflet en haut à gauche
	{ 1, 1, 1, 1, 1, 1, 1 },
	{ 1, 1, 1, 1, 1, 1, 1 },
	{ 0, 1, 1, 1, 1, 1, 0 },
};

const uint8_t EffetKawaii::SPR_HALF[6][7] = {
	{ 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 1, 1, 1, 1, 1, 0 },
	{ 1, 1, 1, 1, 1, 1, 1 },
	{ 0, 1, 1, 1, 1, 1, 0 },
	{ 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0 },
};

const uint8_t EffetKawaii::SPR_CLOSED[6][7] = {
	{ 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 1, 1, 1, 1, 1, 0 },
	{ 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0 },
};

const uint8_t EffetKawaii::SPR_HAPPY[6][7] = {
	{ 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 1, 1, 1, 0, 0 },
	{ 0, 1, 0, 0, 0, 1, 0 },
	{ 1, 0, 0, 0, 0, 0, 1 },
	{ 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0 },
};
