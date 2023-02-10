/*
 * Author: Crownstone Team
 * Copyright: Crownstone (https://crownstone.rocks)
 * Date: Aug 20, 2018
 * Triple-license: LGPLv3+, Apache License, and/or MIT
 */

/* Hardware board configuration.
 *
 * This file stores information that is different per type of hardware. Not every type of hardware is available to
 * customers. It contains the following information:
 *   - pin assignment
 *   - existence of e.g. dimming/switch hardware (a Guidestone or dongle does not have relays or IGBTs)
 *   - circuit parameters (e.g. the measurement circuit might have different values)
 *   - calibration values (e.g. the threshold for triggering a high temperature warning depends on board layout, or
 *     the value with which we want to trigger a tap-to-toggle action depends on the antenna)
 *
 * For information on how to add a new board see:
 *    - https://github.com/crownstone/bluenet/blob/master/docs/ADD_BOARD.md
 *
 * The hardware that is available to customers:
 *
 *    - ACR01B1D, the Crownstone Built-in Zero
 *    - ACR01B10B, the Crownstone Built-in One
 *    - ACR01B10D, the Crownstone Built-in One
 *    - ACR01B2C, the Crownstone Plug
 *    - ACR01B2G, the Crownstone Plug with some electronic improvements
 *    - Guidestone, one version
 *    - USB dongle, one version
 *
 * The hardware that is in development:
 *
 *    - ACR01B15A, the Crownstone Built-in Two
 */

#include <boards/cs_ACR01B10B.h>
#include <boards/cs_ACR01B10D.h>
#include <boards/cs_ACR01B11A.h>
#include <boards/cs_ACR01B13B.h>
#include <boards/cs_ACR01B15A.h>
#include <boards/cs_ACR01B1D.h>
#include <boards/cs_ACR01B2C.h>
#include <boards/cs_ACR01B2G.h>
#include <boards/cs_BoardMap.h>
#include <boards/cs_CR01R02v4.h>
#include <boards/cs_GuideStone.h>
#include <boards/cs_PCA10040.h>
#include <boards/cs_PCA10056.h>
#include <boards/cs_PCA10059.h>
#include <boards/cs_UsbDongle.h>
#include <cfg/cs_AutoConfig.h>
#include <cfg/cs_Boards.h>
#include <cfg/cs_DeviceTypes.h>
#include <drivers/cs_Uicr.h>
#include <protocol/cs_ErrorCodes.h>
#include <protocol/cs_UicrPacket.h>

/**
 * Initialize conservatively (as if given pins are not present).
 */
void init(boards_config_t* config) {
	config->hardwareBoard                     = 0;
	config->pinDimmer                         = PIN_NONE;
	config->pinEnableDimmer                   = PIN_NONE;
	config->pinRelayDebug                     = PIN_NONE;
	config->pinRelayOn                        = PIN_NONE;
	config->pinRelayOff                       = PIN_NONE;
	config->pinAinZeroRef                     = PIN_NONE;
	config->pinAinDimmerTemp                  = PIN_NONE;
	config->pinCurrentZeroCrossing            = PIN_NONE;
	config->pinVoltageZeroCrossing            = PIN_NONE;
	config->pinRx                             = PIN_NONE;
	config->pinTx                             = PIN_NONE;
	config->deviceType                        = DEVICE_UNDEF;
	config->powerOffsetMilliWatt              = 0;
	config->minTxPower                        = 0;

	// Set an interval that's not in sync with advertising interval.
	// And a scan window of 75% of the interval, in case the board cannot provide enough power.
	config->scanIntervalUs                    = 140 * 1000;
	config->scanWindowUs                      = 3 * config->scanIntervalUs / 4;
	config->tapToToggleDefaultRssiThreshold   = 0;

	config->flags.enableUart                  = false;
	config->flags.enableLeds                  = false;
	config->flags.usesNfcPins                 = false;
	config->flags.hasAccuratePowerMeasurement = false;
	config->flags.canTryDimmingOnBoot         = false;
	config->flags.canDimOnWarmBoot            = false;
	config->flags.dimmerOnWhenPinsFloat       = true;

	for (uint8_t i = 0; i < GAIN_COUNT; ++i) {
		config->pinAinVoltage[i]              = PIN_NONE;
		config->pinAinCurrent[i]              = PIN_NONE;
		config->pinAinVoltageAfterLoad[i]     = PIN_NONE;
		config->voltageMultiplier[i]          = 0.0;
		config->voltageAfterLoadMultiplier[i] = 0.0;
		config->currentMultiplier[i]          = 0.0;
		config->voltageOffset[i]              = 0;
		config->voltageAfterLoadOffset[i]     = 0;
		config->currentOffset[i]              = 0;
	}
	for (uint8_t i = 0; i < GPIO_INDEX_COUNT; ++i) {
		config->pinGpio[i] = PIN_NONE;
	}
	for (uint8_t i = 0; i < LED_COUNT; ++i) {
		config->pinLed[i] = PIN_NONE;
	}
	for (uint8_t i = 0; i < BUTTON_COUNT; ++i) {
		config->pinButton[i] = PIN_NONE;
	}

	config->pinFlash.cs     = PIN_NONE;
	config->pinFlash.clk    = PIN_NONE;
	config->pinFlash.dio[0] = PIN_NONE;
	config->pinFlash.dio[1] = PIN_NONE;
	config->pinFlash.dio[2] = PIN_NONE;
	config->pinFlash.dio[3] = PIN_NONE;
}

/**
 * Helper function to map GPIO pins to AIN indices.
 */
uint8_t GpioToAinOnChipset(uint8_t gpio, uint8_t chipset) {
	switch (chipset) {
		case CHIPSET_NRF52832:
		case CHIPSET_NRF52833:
		case CHIPSET_NRF52840:
			switch (gpio) {
				case 2: return 0;
				case 3: return 1;
				case 4: return 2;
				case 5: return 3;
				case 28: return 4;
				case 29: return 5;
				case 30: return 6;
				case 31: return 7;
				default: return PIN_NONE;
			}
		default: return PIN_NONE;
	}
}

// For now mapping is always the same, so this simplified function can be used.
uint8_t GpioToAin(uint8_t gpio) {
	return GpioToAinOnChipset(gpio, CHIPSET_NRF52832);
}

uint8_t GetGpioPin(uint8_t major, uint8_t minor) {
	return major * 32 + minor;
}

cs_ret_code_t configure_board(boards_config_t* config) {
	uint32_t hardwareBoard  = getHardwareBoard();
	//	return configure_board_from_hardware_board(hardwareBoard, config);

	// Create UICR data from hardware board.
	cs_uicr_data_t uicrData = mapBoardToUicrData(hardwareBoard);

	// Try to set uicr data, in case it's not set yet.
	// If it's set already, this will only write fields that were not set yet.
	setUicr(&uicrData, false);

	// Finally ,read the UICR.
	getUicr(&uicrData);

	return configure_board_from_uicr(&uicrData, config);
}

cs_ret_code_t configure_board_from_hardware_board(uint32_t hardwareBoard, boards_config_t* config) {
	init(config);

	switch (hardwareBoard) {
		case ACR01B1A:
		case ACR01B1B:
		case ACR01B1C:
		case ACR01B1D:
		case ACR01B1E: asACR01B1D(config); break;

		case ACR01B10B: asACR01B10B(config, NULL); break;
		case ACR01B10D: asACR01B10D(config); break;

		case ACR01B13B: asACR01B13B(config); break;

		case ACR01B15A: asACR01B15A(config); break;

		case ACR01B2A:
		case ACR01B2B:
		case ACR01B2C: asACR01B2C(config); break;

		case ACR01B2E:
		case ACR01B2G: asACR01B2G(config); break;
		case ACR01B11A: asACR01B11A(config); break;
		case CR01R02v4: asCR01R02v4(config); break;
		case GUIDESTONE: asGuidestone(config); break;

		case PCA10036:
		case PCA10040: asPca10040(config); break;
		case PCA10056: asPca10056(config); break;
		case PCA10059: asPca10059(config); break;
		case PCA10100:
			// should not be
			asPca10040(config);
			break;
		case CS_USB_DONGLE: asUsbDongle(config); break;

		default:
			// undefined board layout !!!
			return ERR_UNKNOWN_TYPE;
	}

	config->hardwareBoard = hardwareBoard;

	return ERR_SUCCESS;
}

cs_ret_code_t configure_board_from_uicr(const cs_uicr_data_t* uicrData, boards_config_t* config) {
	init(config);
	config->hardwareBoard = uicrData->board;

	// Be strict for major version: no default.
	// Do have a default for minor and patch version, so that future hardware will still be supported.
	switch (uicrData->productRegionFamily.fields.productType) {
		case PRODUCT_DEV_BOARD: {
			// For dev boards, we don't have useful UICR data, so just use hardware board.
			switch (uicrData->board) {
				case PCA10036:
				case PCA10040: {
					asPca10040(config);
					break;
				}
				case PCA10056: {
					asPca10056(config);
					break;
				}
				default: {
					// Fall back to pca10040
					asPca10040(config);
					break;
				}
			}
			return ERR_SUCCESS;
		}
		case PRODUCT_CROWNSTONE_PLUG_ZERO: {
			switch (uicrData->majorMinorPatch.fields.major) {
				case 0: {
					asACR01B2C(config);
					return ERR_SUCCESS;
				}
				case 1: {
					switch (uicrData->majorMinorPatch.fields.minor) {
						case 0: {
							asACR01B2C(config);
							return ERR_SUCCESS;
						}
						case 1:
						case 3:
						default: {
							asACR01B2G(config);
							return ERR_SUCCESS;
						}
					}
				}
			}
			break;
		}
		case PRODUCT_CROWNSTONE_PLUG_ONE: {
			switch (uicrData->majorMinorPatch.fields.major) {
				case 0: {
					asACR01B11A(config);
					return ERR_SUCCESS;
				}
			}
			break;
		}
		case PRODUCT_CROWNSTONE_BUILTIN_ZERO: {
			switch (uicrData->majorMinorPatch.fields.major) {
				case 0: {
					asACR01B1D(config);
					return ERR_SUCCESS;
				}
			}
			break;
		}
		case PRODUCT_CROWNSTONE_BUILTIN_ONE: {
			switch (uicrData->majorMinorPatch.fields.major) {
				case 0: {
					switch (uicrData->majorMinorPatch.fields.minor) {
						case 0: {
							asACR01B10B(config, uicrData);
							return ERR_SUCCESS;
						}
						case 1:
						default: {
							asACR01B10D(config);
							return ERR_SUCCESS;
						}
					}
				}
			}
			break;
		}
		case PRODUCT_CROWNSTONE_BUILTIN_TWO: {
			switch (uicrData->majorMinorPatch.fields.major) {
				case 0:
					switch (uicrData->majorMinorPatch.fields.minor) {
						case 1: {
							asACR01B13B(config);
							return ERR_SUCCESS;
						}
						case 2:
						default: {
							asACR01B15A(config);
							return ERR_SUCCESS;
						}
					}
			}
			break;
		}
		case PRODUCT_GUIDESTONE: {
			switch (uicrData->majorMinorPatch.fields.major) {
				case 0:
				case 1: {
					asGuidestone(config);
					return ERR_SUCCESS;
				}
			}
			break;
		}
		case PRODUCT_CROWNSTONE_USB_DONGLE: {
			switch (uicrData->majorMinorPatch.fields.major) {
				case 0: {
					asUsbDongle(config);
					return ERR_SUCCESS;
				}
			}
			break;
		}
		case PRODUCT_CROWNSTONE_OUTLET: {
			switch (uicrData->majorMinorPatch.fields.major) {
				case 0: {
					asCR01R02v4(config);
					return ERR_SUCCESS;
				}
			}
			break;
		}
	}
	return ERR_UNKNOWN_TYPE;
}
