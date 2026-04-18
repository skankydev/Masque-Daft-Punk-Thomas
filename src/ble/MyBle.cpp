#include "ble/MyBle.h"
#include "Terminal.h"
#include "LedManager.h"


class BleServerCallbacks : public BLEServerCallbacks
{
	void onConnect(BLEServer* pServer){
		MyBle* ble = MyBle::getInstance();
		ble->setConnected();
	}

	void onDisconnect(BLEServer* pServer){
		MyBle* ble = MyBle::getInstance();
		ble->setDisconnected();
	}
};

class BleAction: public BLECharacteristicCallbacks{
	void onWrite(BLECharacteristic *action) {
		std::string tmp = action->getValue();
		MyBle* ble = MyBle::getInstance();
		ble->setAction(tmp.c_str());
	}
};


MyBle* MyBle::instance = nullptr;

MyBle* MyBle::getInstance(){
	if(!instance){
		instance = new MyBle;
	}
	return instance;
}


MyBle::MyBle(){
	_isConnected = false;
	_isNotified = false;
	_isInit = false;

}

void MyBle::init(){
	if(_isInit){
		Serial.println("BLE init deja init");
		return;
	}
	String n = "Daft Punk";
	BLEDevice::init(n.c_str());

	

	_server = BLEDevice::createServer();
	_server->setCallbacks(new BleServerCallbacks());

	_service = _server->createService(BLEUUID(SERVICE_UUID));
	
	_notifieur = _service->createCharacteristic(NOTIFIEUR_UUID,
		BLECharacteristic::PROPERTY_READ |
		BLECharacteristic::PROPERTY_WRITE  |
		BLECharacteristic::PROPERTY_NOTIFY |
		BLECharacteristic::PROPERTY_INDICATE);
	_notifieur->addDescriptor(new BLE2902());
	_notifieur->setCallbacks(new BleAction());

	_service->start();
	_advertising = _server->getAdvertising();
	_advertising->start();
	success("BLE init");
	_isInit = true;

}


void MyBle::sendNotif(String message){
	Serial.println(message);
	_notifieur->setValue(message.c_str());
	_notifieur->notify();
	delay(100);
}

bool MyBle::setConnected(){
	success("Coucou connect");
	_isConnected = true;
	return _isConnected;
}

bool MyBle::setDisconnected(){
	warning("Bye bye connect");
	_isConnected = false;
	_server->getAdvertising()->start();
	return _isConnected;
}

bool MyBle::isConnected(){
	return _isConnected;
}

void MyBle::setAction(String value){
	_actionList.push_back(value);
}

void MyBle::doAction(){
	if( _timeOut != 0 && _timeOut < millis()){
		_timeOut = 0;
	}
	if(_actionList.empty()){
		return;
	}

	String action = _actionList[0];
	_actionList.erase(_actionList.begin());
	Serial.println("BLE action : " + action);

	// Découpe commande:valeur
	int sep       = action.indexOf(':');
	String cmd    = (sep == -1) ? action : action.substring(0, sep);
	String param  = (sep == -1) ? ""     : action.substring(sep + 1);
	Serial.println("BLE cmd : " + cmd);

	LedManager* leds = LedManager::getInstance();

	if (cmd == "getEffects") {
		// Envoie chaque effet en notif séparée avec délai court
		uint8_t count = leds->effectCount();
		this->sendNotif(("{\"count\":" + String(count) + "}").c_str());
		delay(50);
		for (uint8_t i = 0; i < count; i++) {
			String json = leds->effectJson(i);
			this->sendNotif(json.c_str());
		}
	} else if (cmd == "PRINT") {
		Terminal::getInstance()->help();
	} else if (cmd == "REBOOT") {
		ESP.restart();
	} else if (cmd == "next") {
		leds->setNextEffect();
	} else if (cmd == "default") {
		leds->setDefault();
	} else if (cmd == "setEffectFull") {
		// Format : id|speed|color|text  (color et text optionnels)
		int p1 = param.indexOf('|');
		int p2 = p1 == -1 ? -1 : param.indexOf('|', p1 + 1);
		int p3 = p2 == -1 ? -1 : param.indexOf('|', p2 + 1);

		uint8_t  idx   = param.toInt();
		uint32_t speed = p1 == -1 ? 0 : param.substring(p1 + 1).toInt();
		String   color = p2 == -1 ? "" : param.substring(p2 + 1, p3 == -1 ? param.length() : p3);
		String   text  = p3 == -1 ? "" : param.substring(p3 + 1);

		leds->setEffect(idx);
		if (speed > 0)        leds->setSpeed(speed);
		if (color.length() > 0) {
			uint32_t hex = strtol(color.c_str(), NULL, 16);
			leds->setColor(CRGB((hex >> 16) & 0xFF, (hex >> 8) & 0xFF, hex & 0xFF));
		}
		if (text.length() > 0) leds->setText(text);

	} else if (cmd == "setEffect") {
		leds->setEffect((uint8_t)param.toInt());
	} else if (cmd == "setBrightness") {
		leds->setBrightness((uint8_t)param.toInt());
	} else if (cmd == "setSpeed") {
		leds->setSpeed((uint32_t)param.toInt());
	} else if (cmd == "setText") {
		leds->setText(param);
	} else if (cmd == "setColor") {
		uint32_t hex = strtol(param.c_str(), NULL, 16);
		leds->setColor(CRGB((hex >> 16) & 0xFF, (hex >> 8) & 0xFF, hex & 0xFF));
	} else {
		warning("BLE commande inconnue : " + cmd);
	}

}

void MyBle::updateEffectList() {
    // Réservé — liste envoyée via notifs sur commande getEffects
}

void MyBle::cancelConnect(){
	_server->removePeerDevice(_server->getConnId(), true);
}

