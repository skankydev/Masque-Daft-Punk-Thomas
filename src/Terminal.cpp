#include "Terminal.h"

Terminal* Terminal::instance = nullptr;

Terminal* Terminal::getInstance() {
    if (!instance) {
        instance = new Terminal;
    }
    return instance;
}

Terminal::Terminal() {
    _leds = LedManager::getInstance();
    setupMap();
}

void Terminal::setupMap() {
    _command["help"]    = &Terminal::help;
    _command["print"]   = &Terminal::print;
    _command["reboot"]  = &Terminal::reboot;
    _command["default"] = &Terminal::setDefault;
    _command["next"]    = &Terminal::next;
    _command["b+"]      = &Terminal::upBrightness;
    _command["b-"]      = &Terminal::downBrightness;
    _command["auto"]    = &Terminal::toggleAutoMode;

    _commandParam["setEffect"]     = &Terminal::setEffect;
    _commandParam["setBrightness"] = &Terminal::setBrightness;
    _commandParam["setSpeed"]      = &Terminal::setSpeed;
    _commandParam["setAutoDelay"]  = &Terminal::setAutoDelay;
}

void Terminal::step() {
    if (Serial.available() > 0) {
        String cmd = Serial.readStringUntil('\n');
        cmd.trim();
        if (cmd.length() > 0) {
            doCommand(cmd);
        }
    }
}

void Terminal::printLigne(const String& commande, const String& params, const String& aide, const String& couleur) {
    String cmdColored = commande;
    if      (couleur == "bleu")  cmdColored = bleu(commande);
    else if (couleur == "rouge") cmdColored = rouge(commande);
    else if (couleur == "vert")  cmdColored = vert(commande);
    else if (couleur == "jaune") cmdColored = jauneVif(commande);

    String cmdPart = violet("║ ") + cmdColored;
    int espacesCmd = 15 - commande.length();
    for (int i = 0; i < espacesCmd; i++) cmdPart += " ";

    String paramsPart = "";
    if (params.length() > 0) paramsPart = "{" + jauneVif(params) + "}";
    int espacesParams = 20 - params.length() - 4;
    for (int i = 0; i < espacesParams; i++) paramsPart += " ";

    Serial.println(cmdPart + paramsPart + ": " + aide);
}

void Terminal::doCommand(String cmd) {
    cmd.trim();
    int spaceIdx  = cmd.indexOf(' ');
    String baseCmd = (spaceIdx == -1) ? cmd : cmd.substring(0, spaceIdx);
    String params  = (spaceIdx == -1) ? "" : cmd.substring(spaceIdx + 1);

    auto it = _command.find(baseCmd);
    if (it != _command.end()) {
        (this->*it->second)();
        return;
    }

    auto itParam = _commandParam.find(baseCmd);
    if (itParam != _commandParam.end()) {
        (this->*itParam->second)(params);
        return;
    }

    Serial.println(jaune("━━━━━━━━ ") + "Cmd inconnue " + rougeVif(baseCmd) + jaune(" ━━━━━━━━"));
}

void Terminal::help() {
    Serial.println(violet("╔══════════ ") + vertVif("Matrix Terminal") + violet(" ══════════"));
    printLigne("help",          "",        "Affiche cette aide",              "bleu");
    printLigne("print",         "",        "Etat courant",                    "bleu");
    printLigne("reboot",        "",        "Redémarre l'ESP32",               "rouge");
    printLigne("default",       "",        "Reset config",                    "vert");
    printLigne("next",          "",        "Effet suivant",                   "vert");
    printLigne("auto",          "",        "Active/désactive le mode auto",   "vert");
    printLigne("b+",            "",        "Luminosité +10",                  "vert");
    printLigne("b-",            "",        "Luminosité -10",                  "vert");
    printLigne("setEffect",     "0-N",     "Choisit un effet",                "vert");
    printLigne("setBrightness", "0-255",   "Définit la luminosité",           "vert");
    printLigne("setSpeed",      "ms",      "Délai entre frames (défaut 30)",  "vert");
    printLigne("setAutoDelay",  "ms",      "Délai auto en ms",                "vert");

    Serial.println(violet("║"));
    Serial.println(violet("║ ") + "Effets disponibles :");
    for (uint8_t i = 0; i < _leds->effectCount(); i++) {
        Serial.println(violet("║   ") + rouge(String(i)) + " - " + jaune(_leds->effectName(i)));
    }
    Serial.println(violet("╚══════════════════════════════════════"));
}

void Terminal::print() {
    _leds->print();
}

void Terminal::reboot() {
    Serial.println(jauneVif("Redémarrage..."));
    delay(500);
    ESP.restart();
}

void Terminal::setDefault() {
    _leds->setDefault();
}

void Terminal::next() {
    _leds->setNextEffect();
}

void Terminal::upBrightness() {
    uint8_t b = _leds->getBrightness() + 10;
    if (b > 250) b = 10;
    _leds->setBrightness(b);
}

void Terminal::downBrightness() {
    int b = (int)_leds->getBrightness() - 10;
    if (b < 10) b = 250;
    _leds->setBrightness((uint8_t)b);
}

void Terminal::toggleAutoMode() {
    _leds->toggleAutoMode();
}

void Terminal::setEffect(String params) {
    uint8_t idx = params.toInt();
    if (idx >= _leds->effectCount()) {
        return error("Effet inconnu. Max : " + String(_leds->effectCount() - 1));
    }
    _leds->setEffect(idx);
}

void Terminal::setBrightness(String params) {
    _leds->setBrightness((uint8_t)params.toInt());
}

void Terminal::setSpeed(String params) {
    _leds->setSpeed((uint32_t)params.toInt());
}

void Terminal::setAutoDelay(String params) {
    _leds->setAutoDelay((uint32_t)params.toInt());
}
