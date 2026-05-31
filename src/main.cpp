#include "setting.h"
#include "MainApp.h"
#include "mic/MicManager.h"

MainApp mainApp;

void setup() {
	mainApp.init();
	// Lance le micro sur le core 0 (loop() tourne sur core 1)
	MicManager::getInstance()->startTask();
}

void loop() {
	mainApp.step();
	//mainApp.testEffects();
}
