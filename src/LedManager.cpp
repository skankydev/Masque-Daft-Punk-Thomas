#include "LedManager.h"
#include "effects/EffetRainbow.h"
#include "effects/EffetFire.h"
#include "effects/EffetColorWave.h"
#include "effects/EffetScanner.h"
#include "effects/EffetRain.h"
#include "effects/EffetPacman.h"
#include "effects/EffetText.h"
#include "effects/EffetCylon.h"
#include "effects/EffetHeartBeat.h"
#include "effects/EffetVisorFire.h"

// ─── Registre des effets ─────────────────────────────────────────────────────
// Pour ajouter un effet : une ligne ici, rien d'autre à toucher.

struct EffetEntry {
    const char*   nom;
    Effect*       (*create)();
};

static const EffetEntry EFFETS[] = {
    { "Rainbow",    []() -> Effect* { return new EffetRainbow();   } },
    { "Fire",       []() -> Effect* { return new EffetFire();      } },
    { "Color Wave", []() -> Effect* { return new EffetColorWave(); } },
    { "Scanner",    []() -> Effect* { return new EffetScanner();   } },
    { "Rain",       []() -> Effect* { return new EffetRain();      } },
    { "Pacman",     []() -> Effect* { return new EffetPacman();   } },
    { "Text",       []() -> Effect* { return new EffetText();      } },
    { "Cylon",      []() -> Effect* { return new EffetCylon();     } },
    { "HeartBeat",  []() -> Effect* { return new EffetHeartBeat(); } },
    { "VisorFire",  []() -> Effect* { return new EffetVisorFire(); } },
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
    FastLED.addLeds<WS2812B, PIN_LEDS, GRB>(_leds, NUM_LEDS);
    setDefault();
}

// ─── Defaults ────────────────────────────────────────────────────────────────

void LedManager::setDefault() {
    _brightness = 30;
    _speed      = 30;
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
}

void LedManager::setEffect(uint8_t index) {
    _setEffectByIndex(index);
    Serial.println(bleu("Effet") + " : " + jaune(_current->name()));
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
    Serial.println(jaune("Brightness") + " : " + String(brightness));
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
