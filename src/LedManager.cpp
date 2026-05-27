#include "LedManager.h"
#include "effects/EffetRainbow.h"
#include "effects/EffetFire.h"
#include "effects/EffetScanner.h"
#include "effects/EffetRain.h"
#include "effects/EffetText.h"
#include "effects/EffetCylon.h"
#include "effects/EffetHeartBeat.h"
#include "effects/EffetVisorFire.h"
#include "effects/EffetPacmanGame.h"
#include "effects/EffetGameOfLife.h"
#include "effects/EffetBounce.h"
#include "effects/EffetMatrix.h"
#include "effects/EffetFireworks.h"
#include "effects/EffetSauron.h"
#include "effects/EffetKawaii.h"
#include "effects/EffetAudio.h"
#include "effects/EffetAudio2.h"
#include "effects/EffetAudio3.h"
#include "effects/EffetAudio4.h"

// ─── Registre des effets ─────────────────────────────────────────────────────
// Pour ajouter un effet : une ligne ici, rien d'autre à toucher.
// defaultColor : couleur CSS "#RRGGBB", nullptr = pas de couleur
// defaultText  : texte par défaut, nullptr = pas de texte

struct EffetEntry {
	const char* nom;
	Effect*     (*create)();
	uint32_t    defaultSpeed;  // ms par frame
	const char* defaultColor;  // nullptr = non supporté
	const char* defaultText;   // nullptr = non supporté
};

static const EffetEntry EFFETS[] = {
	{ "Rainbow",    []() -> Effect* { return new EffetRainbow();    }, 50,  nullptr,    nullptr   },
	{ "Text",       []() -> Effect* { return new EffetText();       }, 80,  "#FF0000",  "Daft Punk"  },
	{ "Fire",       []() -> Effect* { return new EffetFire();       }, 30,  "#FF5000",  nullptr   },
	{ "VisorFire",  []() -> Effect* { return new EffetVisorFire();  }, 30,  nullptr,    nullptr   },
	{ "Rain",       []() -> Effect* { return new EffetRain();       }, 50,  "#1E64FF",  nullptr   },
	{ "Matrix",     []() -> Effect* { return new EffetMatrix();     }, 40,  "#00C800",  nullptr   },
	{ "Fireworks",  []() -> Effect* { return new EffetFireworks();  }, 30,  nullptr,    nullptr   },
	{ "Scanner",    []() -> Effect* { return new EffetScanner();    }, 30,  "#FF0000",  nullptr   },
	{ "Cylon",      []() -> Effect* { return new EffetCylon();      }, 40,  "#FF0000",  nullptr   },
	{ "HeartBeat",  []() -> Effect* { return new EffetHeartBeat();  }, 50,  "#FF0000",  nullptr   },
	{ "Bounce",     []() -> Effect* { return new EffetBounce();     }, 50,  "#FF0000",  nullptr   },
	{ "GameOfLife", []() -> Effect* { return new EffetGameOfLife(); }, 200, "#00FF00",  nullptr   },
	{ "PacmanGame", []() -> Effect* { return new EffetPacmanGame(); }, 80,  nullptr,    nullptr   },
	{ "Sauron",     []() -> Effect* { return new EffetSauron();     }, 40,  nullptr,    nullptr   },
	{ "Kawaii",     []() -> Effect* { return new EffetKawaii();     }, 60,  "#FF1478",  nullptr   },
	{ "Audio",      []() -> Effect* { return new EffetAudio();     }, 30,  "#00C8FF",  nullptr   },
	{ "Audio2",     []() -> Effect* { return new EffetAudio2();    }, 30,  nullptr,    nullptr   },
	{ "Audio3",     []() -> Effect* { return new EffetAudio3();    }, 30,  "#00C8FF",  nullptr   },
	{ "Audio4",     []() -> Effect* { return new EffetAudio4();    }, 30,  "#00C8FF",  nullptr   },
};

static const uint8_t NB_EFFETS = sizeof(EFFETS) / sizeof(EFFETS[0]);

// ─── Singleton ───────────────────────────────────────────────────────────────

LedManager* LedManager::instance = nullptr;

LedManager* LedManager::getInstance() {
	if (!instance) {
		instance = new LedManager;
	}
	return instance;
}

LedManager::LedManager() : _current(nullptr) {
	FastLED.addLeds<WS2812B, PIN_LEDS,            GRB>(_leds,     NUM_LEDS);
	FastLED.addLeds<WS2812B, PIN_STRIP_RIGHT_TOP, GRB>(_stripRT,  NUM_STRIP_RIGHT_TOP);
	FastLED.addLeds<WS2812B, PIN_STRIP_RIGHT_BOT, GRB>(_stripRB,  NUM_STRIP_RIGHT_BOT);
	FastLED.addLeds<WS2812B, PIN_STRIP_LEFT_TOP,  GRB>(_stripLT,  NUM_STRIP_LEFT_TOP);
	FastLED.addLeds<WS2812B, PIN_STRIP_LEFT_BOT,  GRB>(_stripLB,  NUM_STRIP_LEFT_BOT);
	setDefault();
}

// ─── Defaults ────────────────────────────────────────────────────────────────

void LedManager::setDefault() {
	_brightness = 10;
	_speed      = 50;
	_lastFrame  = 0;
	_autoMode   = false;
	_lastChange = 0;
	_autoDelay  = 5000;

	FastLED.setBrightness(_brightness);
	_setEffectByIndex(0);
}

// ─── Gestion effets ──────────────────────────────────────────────────────────

void LedManager::_setEffectByIndex(uint8_t index) {
	if (index >= NB_EFFETS) index = 0;
	delete _current;
	_effectIndex = index;
	_current = EFFETS[index].create();

	// Applique les valeurs par défaut de l'effet
	_speed = EFFETS[index].defaultSpeed;
	FastLED.setBrightness(_brightness);

	if (EFFETS[index].defaultColor != nullptr) {
		uint32_t hex = strtol(EFFETS[index].defaultColor + 1, nullptr, 16); // +1 pour sauter le '#'
		_current->setColor(CRGB((hex >> 16) & 0xFF, (hex >> 8) & 0xFF, hex & 0xFF));
	}
	if (EFFETS[index].defaultText != nullptr) {
		_current->setText(String(EFFETS[index].defaultText));
	}
}

void LedManager::setEffect(uint8_t index) {
	_setEffectByIndex(index);
	println(bleu("Effet") + " : " + jaune(_current->name()));
}

void LedManager::setNextEffect() {
	setEffect((_effectIndex + 1) % NB_EFFETS);
}

uint8_t LedManager::getEffectIndex() {
	return _effectIndex;
}

String LedManager::getEffectName() {
	return _current ? _current->name() : "";
}

uint8_t LedManager::effectCount() {
	return NB_EFFETS;
}

String LedManager::effectName(uint8_t index) {
	if (index >= NB_EFFETS) return "";
	return EFFETS[index].nom;
}

String LedManager::effectJson(uint8_t index) {
	if (index >= NB_EFFETS) return "";
	const EffetEntry& e = EFFETS[index];
	String json = "{";
	json += "\"idx\":"      + String(index);
	json += ",\"name\":\""  + String(e.nom) + "\"";
	json += ",\"speed\":"   + String(e.defaultSpeed);
	if (e.defaultColor != nullptr)
		json += ",\"color\":\"" + String(e.defaultColor) + "\"";
	if (e.defaultText != nullptr)
		json += ",\"text\":\"" + String(e.defaultText) + "\"";
	json += "}";
	return json;
}

String LedManager::effectListJson() {
	String json = "[";
	for (uint8_t i = 0; i < NB_EFFETS; i++) {
		if (i > 0) json += ",";
		json += effectJson(i);
	}
	json += "]";
	return json;
}

// ─── Loop ────────────────────────────────────────────────────────────────────

void LedManager::step() {
	unsigned long now = millis();

	if (_autoMode && (now - _lastChange > _autoDelay)) {
		setNextEffect();
		_lastChange = now;
	}

	if (now - _lastFrame >= _speed) {
		_lastFrame = now;
		_current->step(_leds);
		_current->stepStripsTop(_stripRT, NUM_STRIP_RIGHT_TOP);
		_current->stepStripsTop(_stripLT, NUM_STRIP_LEFT_TOP);
		_current->stepStripsBot(_stripRB, NUM_STRIP_RIGHT_BOT);
		_current->stepStripsBot(_stripLB, NUM_STRIP_LEFT_BOT);
		FastLED.show();
	}
}

// ─── Paramètres effets ───────────────────────────────────────────────────────

void LedManager::setText(String text) {
	_current->setText(text);
}

void LedManager::setColor(CRGB color) {
	_current->setColor(color);
}

// ─── Luminosité ──────────────────────────────────────────────────────────────

void LedManager::setBrightness(uint8_t brightness) {
	_brightness = brightness;
	FastLED.setBrightness(brightness);
	println(jaune("Brightness") + " : " + String(brightness));
}

uint8_t LedManager::getBrightness() {
	return _brightness;
}

// ─── Vitesse ─────────────────────────────────────────────────────────────────

void LedManager::setSpeed(uint32_t ms) {
	_speed = ms;
	Serial.println(jaune("Speed") + " : " + String(ms) + "ms");
}

uint32_t LedManager::getSpeed() {
	return _speed;
}

// ─── Auto mode ───────────────────────────────────────────────────────────────

void LedManager::toggleAutoMode() {
	_autoMode = !_autoMode;
	if (_autoMode) {
		_lastChange = millis();
		success("Auto mode activé");
	} else {
		warning("Auto mode désactivé");
	}
}

bool LedManager::getAutoMode() {
	return _autoMode;
}

void LedManager::setAutoDelay(uint32_t delayMs) {
	_autoDelay = delayMs;
	Serial.println(bleu("Auto delay") + " : " + String(delayMs) + "ms");
}

// ─── Debug ───────────────────────────────────────────────────────────────────

void LedManager::print() {
	Serial.println(violet("╭───────────────────────"));
	Serial.print(violet("│ Effet")      + "       : "); Serial.println(jaune(getEffectName()));
	Serial.print(violet("│ Brightness") + " : ");       Serial.println(jaune(String(_brightness)));
	Serial.print(violet("│ Speed")      + "       : "); Serial.println(jaune(String(_speed) + "ms"));
	Serial.print(violet("│ Auto")       + "       : "); Serial.println(_autoMode ? vert("on") : rouge("off"));
	Serial.print(violet("│ Auto delay") + " : ");       Serial.println(jaune(String(_autoDelay) + "ms"));
	Serial.println(violet("╰───────────────────────"));
}
