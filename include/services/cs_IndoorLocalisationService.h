/*
 * Author: Dominik Egger
 * Copyright: Distributed Organisms B.V. (DoBots)
 * Date: Oct 21, 2014
 * License: LGPLv3+, Apache License, or MIT, your choice
 */
#pragma once

#include "structs/cs_ScanResult.h"
#include "structs/cs_TrackDevices.h"
#include <ble/cs_Service.h>
#include <ble/cs_Characteristic.h>

#include <drivers/cs_Storage.h>

#include <processing/cs_Scanner.h>

//! The update frequence of the Tick routine in this service
#define LOCALIZATION_SERVICE_UPDATE_FREQUENCY 10
//#define PWM_ON_RSSI

/** The IndoorLocalizationService handles scanning, signal strengths, tracked devices, etc.
 */
class IndoorLocalizationService : public BLEpp::Service, EventListener {

public:
//	typedef function<int8_t()> func_t;

protected:
	void addSignalStrengthCharacteristic();
	void addScanControlCharacteristic();
	void addPeripheralListCharacteristic();
	void addTrackedDeviceListCharacteristic();
	void addTrackedDeviceCharacteristic();

	/** Get a handle to the persistent storage struct and load it from FLASH.
	 *
	 * Persistent storage is implemented in FLASH. Just as with SSDs, it is important to realize that
	 * writing less than a minimal block strains the memory just as much as flashing the entire block.
	 * Hence, there is an entire struct that can be filled and flashed at once.
	 */
	void loadPersistentStorage();

	/** Save to FLASH.
	*/
	void savePersistentStorage();

	void writeTrackedDevices();
	void readTrackedDevices();

	void startTracking();
	void stopTracking();

	void handleEvent(uint16_t evt, void* p_data, uint16_t length);

public:
	IndoorLocalizationService();

	void tick();

	void scheduleNextTick();

	/** Initialize a IndoorLocalization object
	 * @stack Bluetooth Stack to attach this service to
	 *
	 * Add all characteristics and initialize them where necessary.
	 */
	void init();

	/** Sets the number of ticks the rssi of a device is not above threshold before a device is considered not nearby. */
	void setNearbyTimeout(uint16_t counts);

	/** Returns the number of ticks the rssi of a device is not above threshold before a device is considered not nearby. */
	uint16_t getNearbyTimeout();

	void on_ble_event(ble_evt_t * p_ble_evt);

	void onRSSIChanged(int8_t rssi);
	void setRSSILevel(int8_t RSSILevel);
//	void setRSSILevelHandler(func_t func);

#if(SOFTDEVICE_SERIES != 110)
	void onAdvertisement(ble_gap_evt_adv_report_t* p_adv_report);
#endif

private:
	BLEpp::Characteristic<int8_t>* _rssiCharac;
	BLEpp::Characteristic<uint8_t>* _scanControlCharac;
	BLEpp::Characteristic<buffer_ptr_t>* _peripheralCharac;
	BLEpp::Characteristic<buffer_ptr_t>* _trackedDeviceListCharac;
	BLEpp::Characteristic<buffer_ptr_t>* _trackedDeviceCharac;

//	func_t _rssiHandler;

	bool _trackMode;
	bool _trackIsNearby;

	bool _initialized;

#ifdef PWM_ON_RSSI
	int16_t _averageRssi;
#endif

//	ScanResult* _scanResult;
	TrackedDeviceList* _trackedDeviceList;

	pstorage_handle_t _storageHandle;
	ps_indoorlocalisation_service_t _storageStruct;
};
