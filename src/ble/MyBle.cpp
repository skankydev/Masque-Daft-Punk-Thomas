#include "ble/MyBle.h"
#include "Terminal.h"


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
	Serial.println("Action : " + action);
	if(action == "PRINT"){
		Terminal::getInstance()->help();
	}else if(action == "REBOOT"){
		ESP.restart();
	}

}

void MyBle::cancelConnect(){
	_server->removePeerDevice(_server->getConnId(), true);
}

