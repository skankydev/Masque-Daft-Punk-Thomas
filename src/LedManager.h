#pragma once

#include "setting.h"
#include <FastLED.h>
#include "effects/Effect.h"

class LedManager {

	private:
		static LedManager* instance;
		LedManager();

		CRGB    _leds[NUM_LEDS];
		CRGB    _stripRT[NUM_STRIP_RIGHT_TOP];
		CRGB    _stripRB[NUM_STRIP_RIGHT_BOT];
		CRGB    _stripLT[NUM_STRIP_LEFT_TOP];
		CRGB    _stripLB[NUM_STRIP_LEFT_BOT];
		Effect* _current;
		uint8_t _effectIndex;

		uint8_t       _brightness;
		uint32_t      _speed;
		unsigned long _lastFrame;

		bool          _autoMode;
		unsigned long _lastChange;
		uint32_t      _autoDelay;

		void _setEffectByIndex(uint8_t index);

	public:
		static LedManager* getInstance();

		void step();
		void print();
		void setDefault();

		void    setBrightness(uint8_t brightness);
		uint8_t getBrightness();

		void     setSpeed(uint32_t ms);
		uint32_t getSpeed();

		void    setNextEffect();
		void    setRandomEffect();
		void    setEffect(uint8_t index);
		uint8_t getEffectIndex();
		String  getEffectName();

		uint8_t effectCount();
		String  effectName(uint8_t index);
		String  effectJson(uint8_t index);
		String  effectListJson();

		void setText(String text);
		void setColor(CRGB color);

		void toggleAutoMode();
		bool getAutoMode();
		void setAutoDelay(uint32_t delayMs);
};
