#pragma once

#include "setting.h"
#include "Terminal.h"
#include "LedManager.h"
#include "ble/MyBle.h"

class MainApp {

	private:

		LedManager* _leds;
		Terminal* _terminal;
		MyBle* _myBle;

	public:
		MainApp();
		void init();
		void step();
};
