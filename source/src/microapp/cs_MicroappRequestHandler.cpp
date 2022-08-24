/**
 * Microapp command handler.
 *
 * Author: Crownstone Team
 * Copyright: Crownstone (https://crownstone.rocks)
 * Date: March 16, 2022
 * License: LGPLv3+, Apache License 2.0, and/or MIT (triple-licensed)
 */

#include <cfg/cs_Boards.h>
#include <common/cs_Types.h>
#include <cs_MicroappStructs.h>
#include <drivers/cs_Gpio.h>
#include <events/cs_Event.h>
#include <ipc/cs_IpcRamData.h>
#include <logging/cs_Logger.h>
#include <microapp/cs_MicroappRequestHandler.h>
#include <microapp/cs_MicroappController.h>
#include <protocol/cs_CommandTypes.h>
#include <protocol/cs_ErrorCodes.h>
#include <protocol/cs_Packets.h>
#include <storage/cs_State.h>

int MicroappRequestHandler::interruptToDigitalPin(int interrupt) {
	return interrupt;
}

/*
 * Forwards requests from the microapp to the relevant handler
 */
cs_ret_code_t MicroappRequestHandler::handleMicroappRequest(microapp_sdk_header_t* header) {
	LogMicroappRequestHandlerDebug("handleMicroappRequest: [messageType %u, ack %i]", header->messageType, header->ack);
	uint8_t type = header->messageType;
	switch (type) {
		case CS_MICROAPP_SDK_TYPE_NONE: {
			// This should not be used
			LOGw("Microapp yields without setting messageType");
			break;
		}
		case CS_MICROAPP_SDK_TYPE_LOG: {
			microapp_sdk_log_header_t* log = reinterpret_cast<microapp_sdk_log_header_t*>(header);
			return handleRequestLog(log);
		}
		case CS_MICROAPP_SDK_TYPE_PIN: {
			microapp_sdk_pin_t* pin = reinterpret_cast<microapp_sdk_pin_t*>(header);
			return handleRequestPin(pin);
		}
		case CS_MICROAPP_SDK_TYPE_SWITCH: {
			microapp_sdk_switch_t* switchRequest = reinterpret_cast<microapp_sdk_switch_t*>(header);
			return handleRequestSwitch(switchRequest);
		}
		case CS_MICROAPP_SDK_TYPE_SERVICE_DATA: {
			microapp_sdk_service_data_t* serviceData = reinterpret_cast<microapp_sdk_service_data_t*>(header);
			return handleRequestServiceData(serviceData);
		}
		case CS_MICROAPP_SDK_TYPE_TWI: {
			microapp_sdk_twi_t* twi = reinterpret_cast<microapp_sdk_twi_t*>(header);
			return handleRequestTwi(twi);
		}
		case CS_MICROAPP_SDK_TYPE_BLE: {
			microapp_sdk_ble_t* ble = reinterpret_cast<microapp_sdk_ble_t*>(header);
			return handleRequestBle(ble);
		}
		case CS_MICROAPP_SDK_TYPE_MESH: {
			microapp_sdk_mesh_t* mesh = reinterpret_cast<microapp_sdk_mesh_t*>(header);
			return handleRequestMesh(mesh);
		}
		case CS_MICROAPP_SDK_TYPE_POWER_USAGE: {
			microapp_sdk_power_usage_t* powerUsage = reinterpret_cast<microapp_sdk_power_usage_t*>(header);
			return handleRequestPowerUsage(powerUsage);
		}
		case CS_MICROAPP_SDK_TYPE_PRESENCE: {
			microapp_sdk_presence_t* presence = reinterpret_cast<microapp_sdk_presence_t*>(header);
			return handleRequestPresence(presence);
		}
		case CS_MICROAPP_SDK_TYPE_CONTROL_COMMAND: {
			microapp_sdk_control_command_t* controlCommand = reinterpret_cast<microapp_sdk_control_command_t*>(header);
			return handleRequestControlCommand(controlCommand);
		}
		case CS_MICROAPP_SDK_TYPE_YIELD: {
			microapp_sdk_yield_t* yield = reinterpret_cast<microapp_sdk_yield_t*>(header);
			return handleRequestYield(yield);
		}
		default: {
			_log(SERIAL_INFO, true, "Unknown command %u", type);
			// set ack field so microapp will know something went wrong
			header->ack = CS_MICROAPP_SDK_ACK_ERR_UNDEFINED;
			return ERR_UNKNOWN_TYPE;
		}
	}
	return ERR_SUCCESS;
}

// TODO: establish a proper default log level for microapps
#define LOCAL_MICROAPP_LOG_LEVEL SERIAL_INFO

cs_ret_code_t MicroappRequestHandler::handleRequestLog(microapp_sdk_log_header_t* log) {
	__attribute__((unused)) bool newLine = false;
	if (log->flags & CS_MICROAPP_SDK_LOG_FLAG_NEWLINE) {
		newLine = true;
	}
	if (log->size == 0) {
		_log(LOCAL_MICROAPP_LOG_LEVEL, newLine, "");
		log->header.ack = CS_MICROAPP_SDK_ACK_SUCCESS;
		return ERR_SUCCESS;
	}
	switch (log->type) {
		case CS_MICROAPP_SDK_LOG_CHAR: {
			[[maybe_unused]] microapp_sdk_log_char_t* logChar = reinterpret_cast<microapp_sdk_log_char_t*>(log);
			[[maybe_unused]] uint32_t val                     = logChar->value;
			_log(LOCAL_MICROAPP_LOG_LEVEL, newLine, "%i", val);
			break;
		}
		case CS_MICROAPP_SDK_LOG_SHORT: {
			[[maybe_unused]] microapp_sdk_log_short_t* logShort = reinterpret_cast<microapp_sdk_log_short_t*>(log);
			[[maybe_unused]] uint32_t val                       = logShort->value;
			_log(LOCAL_MICROAPP_LOG_LEVEL, newLine, "%i", val);
			break;
		}
		case CS_MICROAPP_SDK_LOG_UINT: {
			[[maybe_unused]] microapp_sdk_log_uint_t* logUint = reinterpret_cast<microapp_sdk_log_uint_t*>(log);
			_log(LOCAL_MICROAPP_LOG_LEVEL, newLine, "%u", logUint->value);
			break;
		}
		case CS_MICROAPP_SDK_LOG_INT: {
			[[maybe_unused]] microapp_sdk_log_int_t* logInt = reinterpret_cast<microapp_sdk_log_int_t*>(log);
			_log(LOCAL_MICROAPP_LOG_LEVEL, newLine, "%i", logInt->value);
			break;
		}
		case CS_MICROAPP_SDK_LOG_FLOAT: {
			[[maybe_unused]] microapp_sdk_log_float_t* logFloat = reinterpret_cast<microapp_sdk_log_float_t*>(log);
			[[maybe_unused]] int32_t val                        = logFloat->value;
			[[maybe_unused]] int32_t decimal                    = abs(static_cast<int>(logFloat->value * 1000.0) % 1000);
			// We automatically cast to int because printf of floats is disabled due to size limitations
			_log(LOCAL_MICROAPP_LOG_LEVEL, newLine, "%i.%03i", val, decimal);
			break;
		}
		case CS_MICROAPP_SDK_LOG_DOUBLE: {
			[[maybe_unused]] microapp_sdk_log_double_t* logDouble = reinterpret_cast<microapp_sdk_log_double_t*>(log);
			[[maybe_unused]] int32_t val                          = logDouble->value;
			[[maybe_unused]] int32_t decimal                      = abs(static_cast<int>(logDouble->value * 1000.0) % 1000);
			// We automatically cast to int because printf of floats is disabled due to size limitations
			_log(LOCAL_MICROAPP_LOG_LEVEL, newLine, "%i.%03i", val);
			break;
		}
		case CS_MICROAPP_SDK_LOG_STR: {
			[[maybe_unused]] microapp_sdk_log_string_t* logString = reinterpret_cast<microapp_sdk_log_string_t*>(log);
			// Enforce a zero-byte at the end before we log
			uint8_t zeroByteIndex                                 = MICROAPP_SDK_MAX_STRING_LENGTH - 1;
			if (log->size < zeroByteIndex) {
				zeroByteIndex = log->size;
			}
			logString->str[zeroByteIndex] = 0;
			_log(LOCAL_MICROAPP_LOG_LEVEL, newLine, "%s", logString->str);
			break;
		}
		case CS_MICROAPP_SDK_LOG_ARR: {
			[[maybe_unused]] microapp_sdk_log_array_t* logArray = reinterpret_cast<microapp_sdk_log_array_t*>(log);
			if (log->size >= MICROAPP_SDK_MAX_ARRAY_SIZE) {
				// Truncate, but don't send an error
				log->size = MICROAPP_SDK_MAX_ARRAY_SIZE;
			}
			_logArray(LOCAL_MICROAPP_LOG_LEVEL, newLine, reinterpret_cast<int8_t*>(&logArray->arr[0]), log->size);
			break;
		}
		default: {
			LOGi("Unsupported microapp log type: %u", log->type);
			log->header.ack = CS_MICROAPP_SDK_ACK_ERR_UNDEFINED;
			return ERR_UNKNOWN_TYPE;
		}
	}
	log->header.ack = CS_MICROAPP_SDK_ACK_SUCCESS;
	return ERR_SUCCESS;
}

cs_ret_code_t MicroappRequestHandler::handleRequestPin(microapp_sdk_pin_t* pin) {
	MicroappSdkPin pinIndex = (MicroappSdkPin)pin->pin;
	LogMicroappRequestHandlerDebug("handleMicroappPinRequest: [pin %i, type %i]", pinIndex, pin->type);
	if (pinIndex > GPIO_INDEX_COUNT + BUTTON_COUNT + LED_COUNT) {
		LOGi("Pin %i out of range", pinIndex);
		pin->header.ack = CS_MICROAPP_SDK_ACK_ERR_OUT_OF_RANGE;
		return ERR_NOT_FOUND;
	}
	MicroappSdkPinType type = (MicroappSdkPinType)pin->type;
	switch (type) {
		case CS_MICROAPP_SDK_PIN_INIT: {
			// Initializing a pin
			MicroappSdkPinDirection direction = (MicroappSdkPinDirection)pin->direction;
			MicroappSdkPinPolarity polarity   = (MicroappSdkPinPolarity)pin->polarity;

			TYPIFY(EVT_GPIO_INIT) gpio;
			gpio.pin_index = interruptToDigitalPin(pinIndex);
			gpio.pull      = (direction == CS_MICROAPP_SDK_PIN_INPUT_PULLUP) ? 1 : 0;
			LogMicroappRequestHandlerDebug("Initializing GPIO pin %i with direction %u and polarity %u", gpio.pin_index, direction, polarity);

			switch (direction) {
				case CS_MICROAPP_SDK_PIN_INPUT:
				case CS_MICROAPP_SDK_PIN_INPUT_PULLUP: {
					// Initializing a pin as input
					switch (polarity) {
						case CS_MICROAPP_SDK_PIN_NO_POLARITY: {
							gpio.direction = INPUT;
							gpio.polarity  = NONE;
							break;
						}
						case CS_MICROAPP_SDK_PIN_RISING: {
							gpio.direction = SENSE;
							gpio.polarity  = LOTOHI;
							break;
						}
						case CS_MICROAPP_SDK_PIN_FALLING: {
							gpio.direction = SENSE;
							gpio.polarity  = HITOLO;
							break;
						}
						case CS_MICROAPP_SDK_PIN_CHANGE: {
							gpio.direction = SENSE;
							gpio.polarity  = TOGGLE;
							break;
						}
						default: {
							LOGw("Unknown pin polarity: %u", polarity);
							pin->header.ack = CS_MICROAPP_SDK_ACK_ERR_UNDEFINED;
							return ERR_UNKNOWN_TYPE;
						}
					}
					break;
				}
				case CS_MICROAPP_SDK_PIN_OUTPUT: {
					// Initializing a pin as output
					gpio.direction = OUTPUT;
					break;
				}
				default: {
					LOGw("Unknown pin direction: %u", direction);
					pin->header.ack = CS_MICROAPP_SDK_ACK_ERR_UNDEFINED;
					return ERR_UNKNOWN_TYPE;
				}
			}
			event_t event(CS_TYPE::EVT_GPIO_INIT, &gpio, sizeof(gpio));
			event.dispatch();

			if (gpio.direction == SENSE) {
				MicroappController& controller = MicroappController::getInstance();
				cs_ret_code_t result = controller.registerSoftInterrupt(CS_MICROAPP_SDK_TYPE_PIN, pinIndex);
				if (result != ERR_SUCCESS) {
					// Either already registered or no space
					pin->header.ack = CS_MICROAPP_SDK_ACK_ERROR;
					return ERR_UNSPECIFIED;
				}
			}
			break;
		}
		case CS_MICROAPP_SDK_PIN_ACTION: {
			// Performing a read or write on a pin
			MicroappSdkPinActionType action = (MicroappSdkPinActionType)pin->action;
			switch (action) {
				case CS_MICROAPP_SDK_PIN_READ: {
					// Read from a pin. Not implemented
					pin->header.ack = CS_MICROAPP_SDK_ACK_ERR_NOT_IMPLEMENTED;
					return ERR_NOT_IMPLEMENTED;
				}
				case CS_MICROAPP_SDK_PIN_WRITE: {
					// Write to a pin
					TYPIFY(EVT_GPIO_WRITE) gpio;
					gpio.pin_index            = interruptToDigitalPin(pinIndex);
					MicroappSdkPinValue value = (MicroappSdkPinValue)pin->value;
					switch (value) {
						case CS_MICROAPP_SDK_PIN_ON: {
							LogMicroappRequestHandlerDebug("Setting GPIO pin %i", gpio.pin_index);
							gpio.length = 1;
							uint8_t buf[1];
							buf[0]   = 1;
							gpio.buf = buf;
							event_t event(CS_TYPE::EVT_GPIO_WRITE, &gpio, sizeof(gpio));
							event.dispatch();
							break;
						}
						case CS_MICROAPP_SDK_PIN_OFF: {
							LogMicroappRequestHandlerDebug("Clearing GPIO pin %i", gpio.pin_index);
							gpio.length = 1;
							uint8_t buf[1];
							buf[0]   = 0;
							gpio.buf = buf;
							event_t event(CS_TYPE::EVT_GPIO_WRITE, &gpio, sizeof(gpio));
							event.dispatch();
							break;
						}
						default: {
							LOGw("Unknown pin value: %u", value);
							pin->header.ack = CS_MICROAPP_SDK_ACK_ERR_UNDEFINED;
							return ERR_UNKNOWN_TYPE;
						}
					}
					break;
				}
				default: {
					LOGw("Unknown pin action: %u", action);
					pin->header.ack = CS_MICROAPP_SDK_ACK_ERR_UNDEFINED;
					return ERR_UNKNOWN_TYPE;
				}
			}
			break;
		}
		default: {
			LOGw("Unknown pin request type");
			pin->header.ack = CS_MICROAPP_SDK_ACK_ERR_UNDEFINED;
			return ERR_UNKNOWN_TYPE;
		}
	}
	pin->header.ack = CS_MICROAPP_SDK_ACK_SUCCESS;
	return ERR_SUCCESS;
}

cs_ret_code_t MicroappRequestHandler::handleRequestSwitch(microapp_sdk_switch_t* switchRequest) {
	MicroappSdkSwitchValue value = (MicroappSdkSwitchValue)switchRequest->value;
	LogMicroappRequestHandlerDebug("handleMicroappSwitchRequest: [value %i]", value);
	TYPIFY(CMD_SWITCH) switchCommand;
	switchCommand.switchCmd = value;
	cmd_source_with_counter_t source(CS_CMD_SOURCE_MICROAPP);
	event_t event(CS_TYPE::CMD_SWITCH, &switchCommand, sizeof(switchCommand), source);
	event.dispatch();
	switchRequest->header.ack = CS_MICROAPP_SDK_ACK_SUCCESS;
	return ERR_SUCCESS;
}

cs_ret_code_t MicroappRequestHandler::handleRequestServiceData(microapp_sdk_service_data_t* serviceData) {
	LogMicroappRequestHandlerDebug("handleMicroappServiceDataRequest: [uuid %i, size %i]", serviceData->appUuid, serviceData->size);
	if (serviceData->size > MICROAPP_SDK_MAX_SERVICE_DATA_LENGTH) {
		LOGi("Payload size too large");
		serviceData->header.ack = CS_MICROAPP_SDK_ACK_ERR_TOO_LARGE;
		return ERR_WRONG_PAYLOAD_LENGTH;
	}

	TYPIFY(CMD_MICROAPP_ADVERTISE) eventData;
	eventData.version   = 0;  // TODO: define somewhere.
	eventData.type      = 0;  // TODO: define somewhere.
	eventData.appUuid   = serviceData->appUuid;
	eventData.data.len  = serviceData->size;
	eventData.data.data = serviceData->data;
	event_t event(CS_TYPE::CMD_MICROAPP_ADVERTISE, &eventData, sizeof(eventData));
	event.dispatch();
	serviceData->header.ack = CS_MICROAPP_SDK_ACK_SUCCESS;
	return ERR_SUCCESS;
}

cs_ret_code_t MicroappRequestHandler::handleRequestTwi(microapp_sdk_twi_t* twi) {
	MicroappSdkTwiType type = (MicroappSdkTwiType)twi->type;
	LogMicroappRequestHandlerDebug("handleMicroappTwiRequest: [type %i]", type);
	switch (type) {
		case CS_MICROAPP_SDK_TWI_INIT: {
			LogMicroappRequestHandlerDebug("Init i2c");
			TYPIFY(EVT_TWI_INIT) twiInit;
			// no need to write twi.config (is not under control of microapp)
			event_t event(CS_TYPE::EVT_TWI_INIT, &twiInit, sizeof(twiInit));
			event.dispatch();
			break;
		}
		case CS_MICROAPP_SDK_TWI_WRITE: {
			LogMicroappRequestHandlerDebug("Write over i2c to address: 0x%02x", twi->address);
			TYPIFY(EVT_TWI_WRITE) twiWrite;
			twiWrite.address = twi->address;
			twiWrite.buf     = twi->buf;
			twiWrite.length  = twi->size;
			twiWrite.stop    = (twi->flags & CS_MICROAPP_SDK_TWI_FLAG_STOP);
			event_t event(CS_TYPE::EVT_TWI_WRITE, &twiWrite, sizeof(twiWrite));
			event.dispatch();
			break;
		}
		case CS_MICROAPP_SDK_TWI_READ: {
			LogMicroappRequestHandlerDebug("Read from i2c address: 0x%02x", twi->address);
			// Create a synchronous event to retrieve twi data
			TYPIFY(EVT_TWI_READ) twiRead;
			twiRead.address = twi->address;
			twiRead.buf     = twi->buf;
			twiRead.length  = twi->size;
			twiRead.stop    = (twi->flags & CS_MICROAPP_SDK_TWI_FLAG_STOP);
			event_t event(CS_TYPE::EVT_TWI_READ, &twiRead, sizeof(twiRead));
			event.dispatch();

			// Get data back and prepare for microapp
			twi->header.ack = event.result.returnCode;
			twi->size       = twiRead.length;
			break;
		}
		default: {
			LOGw("Unknown TWI type: %i", type);
			twi->header.ack = CS_MICROAPP_SDK_ACK_ERR_UNDEFINED;
			return ERR_UNKNOWN_TYPE;
		}
	}
	twi->header.ack = CS_MICROAPP_SDK_ACK_SUCCESS;
	return ERR_SUCCESS;
}

cs_ret_code_t MicroappRequestHandler::handleRequestBle(microapp_sdk_ble_t* ble) {
	MicroappSdkBleType type = (MicroappSdkBleType)ble->type;
	LogMicroappRequestHandlerDebug("handleMicroappBleRequest: [type %i]", type);
#if BUILD_MESHING == 0
	if (type == CS_MICROAPP_SDK_BLE_SCAN_START || type == CS_MICROAPP_SDK_BLE_SCAN_STOP
		|| type == CS_MICROAPP_SDK_BLE_SCAN_REGISTER_INTERRUPT) {
		LOGw("Scanning is done within the mesh code. No scans will be received because mesh is disabled");
		ble->header.ack = CS_MICROAPP_SDK_ACK_ERR_DISABLED;
		return ERR_NOT_AVAILABLE;
	}
#endif

	switch (type) {
		case CS_MICROAPP_SDK_BLE_SCAN_REGISTER_INTERRUPT: {
			MicroappController& controller = MicroappController::getInstance();
			int result =
					controller.registerSoftInterrupt(CS_MICROAPP_SDK_TYPE_BLE, CS_MICROAPP_SDK_BLE_SCAN_SCANNED_DEVICE);
			if (result != ERR_SUCCESS) {
				LOGw("Registering an interrupt for incoming BLE scans failed with %i", result);
				ble->header.ack = CS_MICROAPP_SDK_ACK_ERROR;
				return result;
			}
			ble->header.ack = CS_MICROAPP_SDK_ACK_SUCCESS;
			break;
		}
		case CS_MICROAPP_SDK_BLE_SCAN_START: {
			LOGv("Start scanning");
			MicroappController& controller = MicroappController::getInstance();
			controller.setScanning(true);
			ble->header.ack = CS_MICROAPP_SDK_ACK_SUCCESS;
			break;
		}
		case CS_MICROAPP_SDK_BLE_SCAN_STOP: {
			LOGv("Stop scanning");
			MicroappController& controller = MicroappController::getInstance();
			controller.setScanning(false);
			ble->header.ack = CS_MICROAPP_SDK_ACK_SUCCESS;
			break;
		}
		case CS_MICROAPP_SDK_BLE_CONNECTION_REQUEST_CONNECT: {
			// Untested
			LOGv("Initiate BLE connection");
			TYPIFY(CMD_BLE_CENTRAL_CONNECT) bleConnectCommand;
			std::reverse_copy(ble->address, ble->address + MAC_ADDRESS_LENGTH, bleConnectCommand.address.address);
			event_t event(CS_TYPE::CMD_BLE_CENTRAL_CONNECT, &bleConnectCommand, sizeof(bleConnectCommand));
			event.dispatch();
			ble->header.ack = CS_MICROAPP_SDK_ACK_IN_PROGRESS;
			LOGi("BLE command result: %u", event.result.returnCode);
			return event.result.returnCode;
		}
		case CS_MICROAPP_SDK_BLE_CONNECTION_REQUEST_DISCONNECT: {
			// Not implemented
			ble->header.ack = CS_MICROAPP_SDK_ACK_ERR_NOT_IMPLEMENTED;
			return ERR_NOT_IMPLEMENTED;
		}
		default: {
			LOGi("Unknown BLE type: %u", type);
			ble->header.ack = CS_MICROAPP_SDK_ACK_ERR_UNDEFINED;
			return ERR_UNKNOWN_TYPE;
		}
	}
	return ERR_SUCCESS;
}

cs_ret_code_t MicroappRequestHandler::handleRequestMesh(microapp_sdk_mesh_t* mesh) {

#if BUILD_MESHING == 0
	LOGw("Mesh is disabled. Mesh-related microapp requests are ignored.");
	mesh->header.ack = CS_MICROAPP_SDK_ACK_ERR_DISABLED;
	return ERR_NOT_AVAILABLE;
#endif
	MicroappSdkMeshType type = (MicroappSdkMeshType)mesh->type;
	LogMicroappRequestHandlerDebug("handleMicroappMeshRequest: [type %i]", type);
	switch (type) {
		case CS_MICROAPP_SDK_MESH_SEND: {
			// microapp_mesh_send_cmd_t* cmd = reinterpret_cast<microapp_mesh_send_cmd_t*>(mesh);

			if (mesh->size == 0) {
				LOGi("No message");
				mesh->header.ack = CS_MICROAPP_SDK_ACK_ERR_EMPTY;
				return ERR_WRONG_PAYLOAD_LENGTH;
			}

			if (mesh->size > MAX_MICROAPP_MESH_PAYLOAD_SIZE) {
				LOGi("Message too large: %u > %u", mesh->size, MAX_MICROAPP_MESH_PAYLOAD_SIZE);
				mesh->header.ack = CS_MICROAPP_SDK_ACK_ERR_TOO_LARGE;
				return ERR_WRONG_PAYLOAD_LENGTH;
			}

			TYPIFY(CMD_SEND_MESH_MSG) eventData;
			bool broadcast = (mesh->stoneId == 0);
			if (!broadcast) {
				LogMicroappRequestHandlerDebug("Send mesh message to %i", mesh->stoneId);
				eventData.idCount   = 1;
				eventData.targetIds = &(mesh->stoneId);
			}
			else {
				LogMicroappRequestHandlerDebug("Broadcast mesh message");
			}
			eventData.flags.flags.broadcast   = broadcast;
			eventData.flags.flags.acked       = !broadcast;
			eventData.flags.flags.useKnownIds = false;
			eventData.flags.flags.doNotRelay  = false;
			eventData.type                    = CS_MESH_MODEL_TYPE_MICROAPP;
			eventData.payload                 = mesh->data;
			eventData.size                    = mesh->size;
			event_t event(CS_TYPE::CMD_SEND_MESH_MSG, &eventData, sizeof(eventData));
			event.dispatch();
			if (event.result.returnCode != ERR_SUCCESS) {
				LOGw("Failed to send mesh message, return code: %u", event.result.returnCode);
				mesh->header.ack = CS_MICROAPP_SDK_ACK_ERROR;
				return event.result.returnCode;
			}
			mesh->header.ack = CS_MICROAPP_SDK_ACK_SUCCESS;
			break;
		}
		case CS_MICROAPP_SDK_MESH_LISTEN: {
			LOGi("Start listening for microapp mesh messages");
			MicroappController& controller = MicroappController::getInstance();
			int result = controller.registerSoftInterrupt(CS_MICROAPP_SDK_TYPE_MESH, CS_MICROAPP_SDK_MESH_READ);
			if (result != ERR_SUCCESS) {
				LOGw("Registering an interrupt for incoming mesh messages failed with %i", result);
				mesh->header.ack = CS_MICROAPP_SDK_ACK_ERROR;
				return result;
			}
			mesh->header.ack = CS_MICROAPP_SDK_ACK_SUCCESS;
			break;
		}
		case CS_MICROAPP_SDK_MESH_READ_CONFIG: {
			LogMicroappRequestHandlerDebug("Microapp requesting mesh info");
			TYPIFY(CONFIG_CROWNSTONE_ID) id;
			State::getInstance().get(CS_TYPE::CONFIG_CROWNSTONE_ID, &id, sizeof(id));
			mesh->stoneId    = id;
			mesh->header.ack = CS_MICROAPP_SDK_ACK_SUCCESS;
			break;
		}
		case CS_MICROAPP_SDK_MESH_READ: {
			LOGi("Reading from mesh can only be done via interrupts");
			mesh->header.ack = CS_MICROAPP_SDK_ACK_ERR_UNDEFINED;
			return ERR_WRONG_OPERATION;
		}
		default: {
			LOGi("Unknown mesh type: %u", type);
			mesh->header.ack = CS_MICROAPP_SDK_ACK_ERR_UNDEFINED;
			return ERR_UNKNOWN_TYPE;
		}
	}
	return ERR_SUCCESS;
}

cs_ret_code_t MicroappRequestHandler::handleRequestPowerUsage(microapp_sdk_power_usage_t* powerUsage) {
	TYPIFY(STATE_POWER_USAGE) powerUsageState;
	State::getInstance().get(CS_TYPE::STATE_POWER_USAGE, &powerUsageState, sizeof(powerUsageState));
	powerUsage->powerUsage = powerUsageState;
	powerUsage->header.ack = CS_MICROAPP_SDK_ACK_SUCCESS;
	return ERR_SUCCESS;
}

cs_ret_code_t MicroappRequestHandler::handleRequestPresence(microapp_sdk_presence_t* presence) {
	if (presence->profileId >= MAX_NUMBER_OF_PRESENCE_PROFILES) {
		LOGi("Incorrect profileId");
		presence->header.ack = CS_MICROAPP_SDK_ACK_ERR_OUT_OF_RANGE;
		return ERR_NOT_FOUND;
	}

	presence_t resultBuf;
	event_t event(CS_TYPE::CMD_GET_PRESENCE);
	event.result.buf = cs_data_t(reinterpret_cast<uint8_t*>(&resultBuf), sizeof(resultBuf));
	event.dispatch();
	if (event.result.returnCode != ERR_SUCCESS) {
		LOGi("No success, result code: %u", event.result.returnCode);
		presence->header.ack = CS_MICROAPP_SDK_ACK_ERROR;
		return event.result.returnCode;
	}
	if (event.result.dataSize != sizeof(presence_t)) {
		LOGi("Result is of size %u, expected size %u", event.result.dataSize, sizeof(presence_t));
		presence->header.ack = CS_MICROAPP_SDK_ACK_ERROR;
		return ERR_WRONG_PAYLOAD_LENGTH;
	}

	presence->presenceBitmask = resultBuf.presence[presence->profileId];
	presence->header.ack      = CS_MICROAPP_SDK_ACK_SUCCESS;
	return ERR_SUCCESS;
}

cs_ret_code_t MicroappRequestHandler::handleRequestControlCommand(
		microapp_sdk_control_command_t* controlCommand) {
	if (controlCommand->size == 0) {
		LOGi("No control command");
		controlCommand->header.ack = CS_MICROAPP_SDK_ACK_ERR_EMPTY;
		return ERR_WRONG_PAYLOAD_LENGTH;
	}
	if (controlCommand->size > MICROAPP_SDK_MAX_CONTROL_COMMAND_PAYLOAD_SIZE) {
		LOGi("Control command too large: %u > %u", controlCommand->size, MICROAPP_SDK_MAX_CONTROL_COMMAND_PAYLOAD_SIZE);
		controlCommand->header.ack = CS_MICROAPP_SDK_ACK_ERR_TOO_LARGE;
		return ERR_WRONG_PAYLOAD_LENGTH;
	}
	LogMicroappRequestHandlerDebug("Dispatching control command of type %u", controlCommand->type);
	TYPIFY(CMD_CONTROL_CMD) eventData;
	eventData.protocolVersion = controlCommand->protocol;
	eventData.data            = controlCommand->payload;
	eventData.size            = controlCommand->size;
	eventData.type            = static_cast<CommandHandlerTypes>(controlCommand->type);
	eventData.accessLevel     = EncryptionAccessLevel::MEMBER;
	cmd_source_with_counter_t source(CS_CMD_SOURCE_MICROAPP);
	event_t event(CS_TYPE::CMD_CONTROL_CMD, &eventData, sizeof(eventData), source);
	event.dispatch();
	switch (event.result.returnCode) {
		case ERR_SUCCESS:
		case ERR_SUCCESS_NO_CHANGE:
		case ERR_WAIT_FOR_SUCCESS:
			break;
		default: {
			LOGi("Dispatched control command not successful, result code: %u", event.result.returnCode);
			controlCommand->header.ack = CS_MICROAPP_SDK_ACK_ERROR;
			return event.result.returnCode;
		}
	}
	controlCommand->header.ack = CS_MICROAPP_SDK_ACK_SUCCESS;
	return ERR_SUCCESS;
}

cs_ret_code_t MicroappRequestHandler::handleRequestYield(microapp_sdk_yield_t* yield) {
	LogMicroappRequestHandlerDebug("handleMicroappYieldRequest: [type %u, emptySlots %u]", yield->type, yield->emptyInterruptSlots);
	// Update number of empty interrupt slots the microapp has
	MicroappController& controller = MicroappController::getInstance();
	controller.setEmptySoftInterruptSlots(yield->emptyInterruptSlots);
	yield->header.ack = CS_MICROAPP_SDK_ACK_SUCCESS;
	return ERR_SUCCESS;
}
