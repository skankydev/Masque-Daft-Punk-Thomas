#include "ble/MyBle.h"
#include "Terminal.h"
#include "LedManager.h"


class BleServerCallbacks : public BLEServerCallbacks
{
	void onConnect(BLEServer* pServer){
		MyBle* ble = MyBle::getInstance();
		ble->setConnected();
		// Met à jour la liste des effets lisible par le tel
		ble->updateEffectList();
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
	delay(500);
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

	LedManager* leds = LedManager::getInstance();

	if (cmd == "PRINT") {
		Terminal::getInstance()->help();
	} else if (cmd == "REBOOT") {
		ESP.restart();
	} else if (cmd == "next") {
		leds->setNextEffect();
	} else if (cmd == "default") {
		leds->setDefault();
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
    String json = LedManager::getInstance()->effectListJson();
    _notifieur->setValue(json.c_str());
}

void MyBle::cancelConnect(){
	_server->removePeerDevice(_server->getConnId(), true);
}

