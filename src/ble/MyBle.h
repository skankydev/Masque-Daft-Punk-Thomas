#ifndef DEF_BLE_MANAGER
#define DEF_BLE_MANAGER

#include "setting.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <vector>
#include <iostream>

#include "esp_ota_ops.h"

#define SERVICE_UUID   "64dc919f-4c32-4b0d-a594-65334e0a72c3"
#define NOTIFIEUR_UUID    "d21d93c2-18bd-402b-8614-b0ca683932ca"
#define FULL_PACKET    512

class MyBle{

	private:
		static MyBle* instance;
		MyBle();

		bool _isConnected;
		bool _isNotified;
		bool _isInit;

		String _value;

		BLESecurity *_security;

		BLEServer *_server;
		BLEService *_service;
		BLECharacteristic* _notifieur;
		BLEAdvertising* _advertising;

		unsigned long _timeOut;

		std::vector<String> _actionList;

	public:
		static MyBle* getInstance();
		void init();
		
		void sendNotif(String message);
		bool setConnected();
		bool setDisconnected();
		bool isConnected();

		void setAction(String value);
		void doAction();
		void updateEffectList();

		void cancelConnect();
};

#endif