#pragma once

#include <map>
#include "setting.h"
#include "LedManager.h"

class Terminal {
public:
    static Terminal* getInstance();
    void step();
    
    void help();

private:
    Terminal();
    static Terminal* instance;
    LedManager* _leds;

    void printLigne(const String& commande, const String& params, const String& aide, const String& couleur = "bleu");

    typedef void (Terminal::*CommandHandler)();
    typedef void (Terminal::*CommandHandlerWithParam)(String);
    std::map<String, CommandHandler> _command;
    std::map<String, CommandHandlerWithParam> _commandParam;

    void setupMap();
    void doCommand(String cmd);

    void print();
    void reboot();
    void system();
    void setDefault();
    void next();
    void upBrightness();
    void downBrightness();
    void toggleAutoMode();

    void setEffect(String params);
    void setBrightness(String params);
    void setSpeed(String params);
    void setAutoDelay(String params);
    void setText(String params);
    void setColor(String params);
};
