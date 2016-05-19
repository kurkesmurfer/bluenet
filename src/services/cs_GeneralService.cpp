/**
 * Author: Dominik Egger
 * Copyright: Distributed Organisms B.V. (DoBots)
 * Date: Oct 22, 2014
 * License: LGPLv3+, Apache License, or MIT, your choice
 */

#include "services/cs_GeneralService.h"

#include "drivers/cs_Timer.h"

#include "cfg/cs_UuidConfig.h"
#include "structs/buffer/cs_MasterBuffer.h"
#include "drivers/cs_Temperature.h"
//#include "common/cs_Config.h"
//#include "common/cs_Strings.h"
#include "drivers/cs_RTC.h"
//
#if CHAR_MESHING==1
#include <protocol/cs_MeshControl.h>
#endif

#include <cfg/cs_Settings.h>
#include <cfg/cs_StateVars.h>

#include <processing/cs_CommandHandler.h>

using namespace BLEpp;

GeneralService::GeneralService() :
		_temperatureCharacteristic(NULL),
		_resetCharacteristic(NULL),
		_selectConfiguration(0xFF),
		_streamBuffer(NULL)
{

	setUUID(UUID(GENERAL_UUID));
	setName(BLE_SERVICE_GENERAL);

	Settings::getInstance();
	StateVars::getInstance();

//	Storage::getInstance().getHandle(PS_ID_GENERAL_SERVICE, _storageHandle);
//	loadPersistentStorage();

	init();

	Timer::getInstance().createSingleShot(_appTimerId, (app_timer_timeout_handler_t)GeneralService::staticTick);
}

void GeneralService::init() {
	LOGi(MSG_SERVICE_GENERAL_INIT);

#if CHAR_TEMPERATURE==1 || DEVICE_TYPE==DEVICE_FRIDGE
	LOGi(MSG_CHAR_TEMPERATURE_ADD);
	addTemperatureCharacteristic();
#else
	LOGi(MSG_CHAR_TEMPERATURE_SKIP);
#endif

#if CHAR_RESET==1
	LOGi(MSG_CHAR_RESET_ADD);
	addResetCharacteristic();
#else
	LOGi(MSG_CHAR_RESET_SKIP);
#endif

#if CHAR_MESHING==1
	{
	LOGi(MSG_CHAR_MESH_ADD);

	_meshMessage = new MeshCharacteristicMessage();

	MasterBuffer& mb = MasterBuffer::getInstance();
	buffer_ptr_t buffer = NULL;
	uint16_t size = 0;
	mb.getBuffer(buffer, size);

	_meshMessage->assign(buffer, size);

	addMeshCharacteristic();

	_meshCharacteristic->setValue(buffer);
	_meshCharacteristic->setMaxLength(size);
	_meshCharacteristic->setDataLength(0);
	}
#else
	LOGi(MSG_CHAR_MESH_SKIP);
#endif

#if CHAR_CONFIGURATION==1 || DEVICE_TYPE==DEVICE_FRIDGE
	{
	LOGi(MSG_CHAR_CONFIGURATION_ADD);

	//! if we use configuration characteristics, set up a buffer
	_streamBuffer = new StreamBuffer<uint8_t>();
	MasterBuffer& mb = MasterBuffer::getInstance();
	buffer_ptr_t buffer = NULL;
	uint16_t size = 0;
	mb.getBuffer(buffer, size);

	LOGd("Assign buffer of size %i to stream buffer", size);
	_streamBuffer->assign(buffer, size);

	addSetConfigurationCharacteristic();
	addSelectConfigurationCharacteristic();
	addGetConfigurationCharacteristic();

	_setConfigurationCharacteristic->setValue(buffer);
	_setConfigurationCharacteristic->setMaxLength(size);
	_setConfigurationCharacteristic->setDataLength(size);

	_getConfigurationCharacteristic->setValue(buffer);
	_getConfigurationCharacteristic->setMaxLength(size);
	_getConfigurationCharacteristic->setDataLength(size);

	LOGd("Set both set/get charac to buffer at %p", buffer);
	}
#else
	LOGi(MSG_CHAR_CONFIGURATION_SKIP);
#endif

#if CHAR_STATE_VARIABLES==1
	{
	LOGi(MSG_CHAR_STATEVARIABLES_ADD);

	uint16_t size = 0;
	buffer_ptr_t buffer = NULL;

	//! if we don't use configuration characteristics, set up a buffer
	if (_streamBuffer == NULL) {
		_streamBuffer = new StreamBuffer<uint8_t>();

		LOGd("Assign buffer of size %i to stream buffer", size);
		_streamBuffer->assign(buffer, size);
	} else {
		//! otherwise use the same buffer
		_streamBuffer->getBuffer(buffer, size);
		size = _streamBuffer->getMaxLength();
	}

	addSelectStateVarCharacteristic();
	addReadStateVarCharacteristic();

	_selectStateVarCharacteristic->setValue(buffer);
	_selectStateVarCharacteristic->setMaxLength(size);
	_selectStateVarCharacteristic->setDataLength(size);

	_readStateVarCharacteristic->setValue(buffer);
	_readStateVarCharacteristic->setMaxLength(size);
	_readStateVarCharacteristic->setDataLength(size);

	LOGd("Set both set/get charac to buffer at %p", buffer);
	}
#else
	LOGi(MSG_CHAR_STATEVARIABLES_SKIP);
#endif
	
	addCharacteristicsDone();
}

//void GeneralService::startAdvertising(Nrf51822BluetoothStack* stack) {
//	Service::startAdvertising(stack);
//	std::string str = ConfigHelper::getInstance().getBLEName();
//	LOGi("setDeviceName");
//	BLEpp::Nrf51822BluetoothStack::getInstance().setDeviceName(str);
////	ps_configuration_t cfg = ConfigHelper::getInstance().getConfig();
////	Storage::getString(cfg.device_name, str, getBLEName());
////	setBLEName(str);
//}

void GeneralService::tick() {
//	LOGi("Tick: %d", RTC::now());

	if (_temperatureCharacteristic) {
		int32_t temp;
		temp = getTemperature();
		writeToTemperatureCharac(temp);
#ifdef MICRO_VIEW
		//! Update temperature at the display
		write("1 %i\r\n", temp);
#endif
	}

	if (_getConfigurationCharacteristic) {
		if (_selectConfiguration != 0xFF) {
			bool success = Settings::getInstance().readFromStorage(_selectConfiguration, _streamBuffer);
			if (success) {
				writeToConfigCharac();
			}
			//! only write once
			_selectConfiguration = 0xFF;
		}
	}

	scheduleNextTick();
}

void GeneralService::scheduleNextTick() {
	Timer::getInstance().start(_appTimerId, HZ_TO_TICKS(GENERAL_SERVICE_UPDATE_FREQUENCY), this);
}

//void GeneralService::loadPersistentStorage() {
//	Storage::getInstance().readStorage(_storageHandle, &_storageStruct, sizeof(_storageStruct));
//}

//void GeneralService::savePersistentStorage() {
//	Storage::getInstance().writeStorage(_storageHandle, &_storageStruct, sizeof(_storageStruct));
//}

void GeneralService::addTemperatureCharacteristic() {
	_temperatureCharacteristic = new Characteristic<int32_t>();
	addCharacteristic(_temperatureCharacteristic);

	_temperatureCharacteristic->setUUID(UUID(getUUID(), TEMPERATURE_UUID));
	_temperatureCharacteristic->setName(BLE_CHAR_TEMPERATURE);
	_temperatureCharacteristic->setDefaultValue(0);
	_temperatureCharacteristic->setNotifies(true);
}

void reset(void* p_context) {
	uint32_t cmd = *(int32_t*)p_context;
	LOGi("executing reset: %d", cmd);
	//! copy to make sure this is nothing more than one value
	uint8_t err_code;
	err_code = sd_power_gpregret_clr(0xFF);
	APP_ERROR_CHECK(err_code);
	err_code = sd_power_gpregret_set(cmd);
	APP_ERROR_CHECK(err_code);
	sd_nvic_SystemReset();
}

void GeneralService::addResetCharacteristic() {
	_resetCharacteristic = new Characteristic<int32_t>();
	addCharacteristic(_resetCharacteristic);

	_resetCharacteristic->setUUID(UUID(getUUID(), RESET_UUID));
	_resetCharacteristic->setName(BLE_CHAR_RESET);
	_resetCharacteristic->setDefaultValue(0);
	_resetCharacteristic->setWritable(true);
	_resetCharacteristic->onWrite([&](const int32_t& value) -> void {
		CommandHandler::getInstance().handleCommand(CMD_RESET, (buffer_ptr_t)&value, 4);
//			if (value != 0) {
//
//				static uint32_t cmd = value;
//				if (cmd == COMMAND_ENTER_RADIO_BOOTLOADER) {
//					LOGi(MSG_FIRMWARE_UPDATE);
//				} else {
//					LOGi(MSG_RESET);
//				}
//
//				app_timer_id_t resetTimer;
//				Timer::getInstance().createSingleShot(resetTimer, (app_timer_timeout_handler_t)reset);
//				Timer::getInstance().start(resetTimer, MS_TO_TICKS(100), &cmd);
//
//			} else {
//				LOGw("To reset write a nonzero value");
//			}
		});
}

#if CHAR_MESHING==1
void GeneralService::addMeshCharacteristic() {
	_meshCharacteristic = new Characteristic<buffer_ptr_t>();
	addCharacteristic(_meshCharacteristic);

	_meshCharacteristic->setUUID(UUID(getUUID(), MESH_CONTROL_UUID));
	_meshCharacteristic->setName(BLE_CHAR_MESH);
	_meshCharacteristic->setWritable(true);
	_meshCharacteristic->onWrite([&](const buffer_ptr_t& value) -> void {
			LOGi(MSG_MESH_MESSAGE_WRITE);


			uint8_t handle = _meshMessage->channel();

			uint8_t* p_data;
			uint16_t length;
			_meshMessage->data(p_data, length);

			MeshControl::getInstance().send(handle, p_data, length);

		});

}
#endif

void GeneralService::addSetConfigurationCharacteristic() {
	_setConfigurationCharacteristic = new Characteristic<buffer_ptr_t>();
	addCharacteristic(_setConfigurationCharacteristic);

	_setConfigurationCharacteristic->setUUID(UUID(getUUID(), SET_CONFIGURATION_UUID));
	_setConfigurationCharacteristic->setName(BLE_CHAR_CONFIG_SET);
	_setConfigurationCharacteristic->setWritable(true);
	_setConfigurationCharacteristic->onWrite([&](const buffer_ptr_t& value) -> void {

			if (!value) {
				LOGw(MSG_CHAR_VALUE_UNDEFINED);
			} else {
				LOGi(MSG_CHAR_VALUE_WRITE);
				MasterBuffer& mb = MasterBuffer::getInstance();
				if (!mb.isLocked()) {
					mb.lock();

					//! TODO: check lenght with actual payload length!
					uint8_t type = _streamBuffer->type();
					LOGi("Write configuration type: %i", (int)type);
					uint8_t *payload = _streamBuffer->payload();
					uint8_t length = _streamBuffer->length();
//					writeToStorage(type, length, payload);
//					Settings::getInstance().writeToStorage(type, _streamBuffer);

					Settings::getInstance().writeToStorage(type, payload, length);

					mb.unlock();
				} else {
					LOGe(MSG_BUFFER_IS_LOCKED);
				}
			}
		});
}

void GeneralService::writeToConfigCharac() {
	/*
	uint16_t len = _streamBuffer->getDataLength();
	uint8_t value = _streamBuffer->payload()[0];
	buffer_ptr_t pntr = _getConfigurationCharacteristic->getValue();
	LOGd("Write to config length %i and value %i", len, value);
	struct stream_t<uint8_t> *stream = (struct stream_t<uint8_t>*) pntr;
	LOGd("Write to config at %p with payload length %i and value %i", pntr, stream->length, stream->payload[0]);
	*/
	_getConfigurationCharacteristic->setDataLength(_streamBuffer->getDataLength());
	_getConfigurationCharacteristic->notify();
}

void GeneralService::addSelectConfigurationCharacteristic() {
	_selectConfigurationCharacteristic = new Characteristic<uint8_t>();
	addCharacteristic(_selectConfigurationCharacteristic);

	_selectConfigurationCharacteristic->setUUID(UUID(getUUID(), SELECT_CONFIGURATION_UUID));
	_selectConfigurationCharacteristic->setName(BLE_CHAR_CONFIG_SELECT);
	_selectConfigurationCharacteristic->setWritable(true);
	_selectConfigurationCharacteristic->onWrite([&](const uint8_t& value) -> void {
			if (value < CONFIG_TYPES) {
				LOGd("Select configuration type: %i", (int)value);
				_selectConfiguration = value;
			} else {
				LOGe("Cannot select %i", value);
			}
		});
}

void GeneralService::addGetConfigurationCharacteristic() {
	_getConfigurationCharacteristic = new Characteristic<buffer_ptr_t>();
	addCharacteristic(_getConfigurationCharacteristic);

	_getConfigurationCharacteristic->setUUID(UUID(getUUID(), GET_CONFIGURATION_UUID));
	_getConfigurationCharacteristic->setName(BLE_CHAR_CONFIG_GET);
	_getConfigurationCharacteristic->setWritable(false);
	_getConfigurationCharacteristic->setNotifies(true);
}

void GeneralService::writeToTemperatureCharac(int32_t temperature) {
	*_temperatureCharacteristic = temperature;
}


void GeneralService::addSelectStateVarCharacteristic() {
	_selectStateVarCharacteristic = new Characteristic<buffer_ptr_t>();
	addCharacteristic(_selectStateVarCharacteristic);

	_selectStateVarCharacteristic->setUUID(UUID(getUUID(), SELECT_STATEVAR_UUID));
	_selectStateVarCharacteristic->setName(BLE_CHAR_STATEVAR_SELECT);
	_selectStateVarCharacteristic->setWritable(true);
	_selectStateVarCharacteristic->onWrite([&](const buffer_ptr_t& value) -> void {
			if (!value) {
				LOGw(MSG_CHAR_VALUE_UNDEFINED);
			} else {
				LOGi(MSG_CHAR_VALUE_WRITE);
				MasterBuffer& mb = MasterBuffer::getInstance();
				if (!mb.isLocked()) {
					mb.lock();

					uint8_t type = _streamBuffer->type();

					LOGi("length: %d", _streamBuffer->length());
					LOGi("value: %p", value);

					if (_streamBuffer->length() == 0) {
						StateVars::getInstance().readFromStorage(type, _streamBuffer);

						_readStateVarCharacteristic->setDataLength(_streamBuffer->getDataLength());
						_readStateVarCharacteristic->notify();
					} else {
						LOGi("write to storage");

						StateVars::getInstance().writeToStorage(type, _streamBuffer->payload(), _streamBuffer->length());
					}

					mb.unlock();
				} else {
					LOGe(MSG_BUFFER_IS_LOCKED);
				}
			}
		});
}

void GeneralService::addReadStateVarCharacteristic() {
	_readStateVarCharacteristic = new Characteristic<buffer_ptr_t>();
	addCharacteristic(_readStateVarCharacteristic);

	_readStateVarCharacteristic->setUUID(UUID(getUUID(), READ_STATEVAR_UUID));
	_readStateVarCharacteristic->setName(BLE_CHAR_STATEVAR_READ);
	_readStateVarCharacteristic->setWritable(false);
	_readStateVarCharacteristic->setNotifies(true);
}

