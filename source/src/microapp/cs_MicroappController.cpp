/**
 *
 * Microapp protocol.
 *
 * Author: Crownstone Team
 * Copyright: Crownstone (https://crownstone.rocks)
 * Date: April 4, 2020
 * License: LGPLv3+, Apache License 2.0, and/or MIT (triple-licensed)
 */

#include <cfg/cs_AutoConfig.h>
#include <common/cs_Types.h>
#include <cs_MemoryLayout.h>
#include <ipc/cs_IpcRamData.h>
#include <logging/cs_Logger.h>
#include <microapp/cs_MicroappController.h>
#include <microapp/cs_MicroappRequestHandler.h>
#include <microapp/cs_MicroappStorage.h>
#include <protocol/cs_ErrorCodes.h>

#define LogMicroappControllerInfo LOGi
#define LogMicroappControllerDebug LOGd
#define LogMicroappControllerVerbose LOGvv

extern "C" {

void updateIoBuffers(uint8_t appIndex, bluenet_io_buffers_t* ioBuffers) {
	// This runs in microapp context, so no logs and no variables on stack.
	switch (appIndex) {
		case 0: {
			reinterpret_cast<microapp_coroutine_args_t*>(getCoroutineArgumentBuffer())->ioBuffers = ioBuffers;
			break;
		}
		// potentially more microapps here
		default: {
			break;
		}
	}
}

/*
 * This function is the only function called by the microapp. It is called from the coroutine context and just yields.
 * The argument is placed outside of the stack so it can be obtained by bluenet after the coroutine context switch.
 *
 * @param[in] ioBuffers     Pointer to buffers provided by the microapp for communication between bluenet and microapp.
 */
microapp_sdk_result_t microappCallback(uint8_t opcode, bluenet_io_buffers_t* ioBuffers) {
	switch (opcode) {
		case CS_MICROAPP_CALLBACK_UPDATE_IO_BUFFER: {
			updateIoBuffers(0, ioBuffers);
			break;
		}
		case CS_MICROAPP_CALLBACK_SIGNAL: {
			break;
		}
		default: {
			LOGe("Unknown opcode %u", opcode);
		}
	}
	yieldCoroutine();
	return CS_MICROAPP_SDK_ACK_SUCCESS;
}

void jumpToAddress(uintptr_t address) {
	// This runs in microapp context, so no logs and no variables on stack.
	goto *(void*)address;
	LOGe("Shouldn't end up here");
}

/*
 * Jump into the microapp (this function should be called as a coroutine). It obtains the very first instruction from
 * the coroutine arguments. It then considers that instruction to be a method without arguments and calls it. An
 * incorrectly written microapp might crash the firmware here. Henceforth, before this moment it must be written to
 * flash that we try to start the microapp. If we get a reboot and see the "try to start" state, we can disable the
 * microapp forever. Alternatively, if the function never yields, it will trip the watchdog. If the watchdog is
 * triggered, we might presume that a microapp was the reason for it and disable it.
 *
 * @param[in] void *args    Arguments provided when starting the coroutine.
 */
void goIntoMicroapp(void* args) {
	// This runs in microapp context, so no logs and no variables on stack.
	jumpToAddress(reinterpret_cast<microapp_coroutine_args_t*>(args)->entry);

	// The coroutine should never return. Incorrectly written microapp!
	LOGe("Coroutine should never return. We should not come here!");
}

}  // extern C

/*
 * MicroappController constructor zero-initializes most fields and makes sure the instance can receive messages through
 * deriving from EventListener and adding itself to the EventDispatcher as listener.
 */
MicroappController::MicroappController() {
	LogMicroappControllerDebug("Microapp ram start=%p end=%p", microappRamSection._start, microappRamSection._end);
}

/*
 * Set the microappCallback in the IPC ram data bank. At a later time it can be used by the microapp to find the
 * address of microappCallback to call back into the bluenet code.
 *
 * The bluenet2microapp object can be stored on the stack because setRamData copies the data.
 */
void MicroappController::setIpcRam() {
	LogMicroappControllerDebug("Set IPC from bluenet for microapp");
	bluenet_ipc_data_cpp_t ipcData;
	ipcData.bluenet2microappData.dataProtocol     = MICROAPP_IPC_DATA_PROTOCOL;
	ipcData.bluenet2microappData.microappCallback = microappCallback;

	LogMicroappControllerDebug("Set callback to %p", ipcData.bluenet2microappData.microappCallback);

	uint32_t retCode = setRamData(IPC_INDEX_BLUENET_TO_MICROAPP, ipcData.raw, sizeof(bluenet2microapp_ipcdata_t));
	if (retCode != ERR_SUCCESS) {
		LOGw("Microapp IPC RAM data error, retCode=%u", retCode);
		return;
	}
	LogMicroappControllerDebug("Set ram data for microapp");
}

/*
 * Checks flash boundaries (for single microapp).
 */
cs_ret_code_t MicroappController::checkFlashBoundaries(uint8_t appIndex, uintptr_t address) {
	if (appIndex >= g_MICROAPP_COUNT) {
		return ERR_UNSAFE;
	}
	uint32_t microappSize          = microappFlashSection._size / g_MICROAPP_COUNT;
	uintptr_t memoryMicroappOffset = microappSize * appIndex;
	uintptr_t addressLow           = microappFlashSection._start + memoryMicroappOffset;
	uintptr_t addressHigh          = addressLow + microappSize;
	if (address < addressLow) {
		return ERR_UNSAFE;
	}
	if (address > addressHigh) {
		return ERR_UNSAFE;
	}
	return ERR_SUCCESS;
}

/**
 * Clear memory. Should be done in ResetHandler, but we don't want to rely on it.
 *
 * TODO: If this is actually necessary, also check if .data is actually properly copied.
 * TODO: This should be initialized per microapp.
 */
uint16_t MicroappController::initMemory(uint8_t appIndex) {
	LOGi("Init memory: clear 0x%p to 0x%p", microappRamSection._start, microappRamSection._end);
	for (uint32_t i = 0; i < microappRamSection._size; ++i) {
		uint32_t* const val = (uint32_t*)(uintptr_t)(microappRamSection._start + i);
		*val                = 0;
	}
	return ERR_SUCCESS;
}

/*
 * Gets the first instruction for the microapp (this is written in its header). We correct for thumb and check its
 * boundaries. Then we call it from a coroutine context and expect it to yield.
 */
void MicroappController::startMicroapp(uint8_t appIndex) {
	LogMicroappControllerInfo("startMicroapp index=%u", appIndex);

	initMemory(appIndex);

	uintptr_t address = MicroappStorage::getInstance().getStartInstructionAddress(appIndex);
	LogMicroappControllerDebug("Microapp: start at %p", address);

	if (checkFlashBoundaries(appIndex, address) != ERR_SUCCESS) {
		LOGe("Address not within microapp flash boundaries");
		return;
	}

	// The entry function is this immediate address (no correction for thumb mode)
	microapp_coroutine_args_t coroutineArg = {
			.entry = address,
			.ioBuffers = nullptr,
	};

	// Write coroutine as argument in the struct so we can yield from it in the context of the microapp stack
	LogMicroappControllerInfo("Init coroutine");
	int result = initCoroutine(goIntoMicroapp, &coroutineArg, sizeof(coroutineArg), microappRamSection._end);
	if (result != 0) {
		LOGe("Failed to init coroutine");
	}
}

uint8_t* MicroappController::getInputMicroappBuffer() {
	uint8_t* payload = reinterpret_cast<microapp_coroutine_args_t*>(getCoroutineArgumentBuffer())->ioBuffers->microapp2bluenet.payload;
	return payload;
}

uint8_t* MicroappController::getOutputMicroappBuffer() {
	uint8_t* payload = reinterpret_cast<microapp_coroutine_args_t*>(getCoroutineArgumentBuffer())->ioBuffers->bluenet2microapp.payload;
	return payload;
}

void MicroappController::setOperatingState(uint8_t appIndex, MicroappOperatingState state) {
	LogMicroappControllerVerbose("setOperatingState appIndex=%u state=%u", appIndex, state);
	if (appIndex > 0) {
		LOGi("Multiple apps not supported yet");
		return;
	}
	uint8_t runFlag = (state == MicroappOperatingState::CS_MICROAPP_RUNNING) ? 1 : 0;
	bluenet_ipc_data_payload_t ipcData;

	// We can just overwrite all, as a newer IPC version will be written and read by bluenet only.
	ipcData.bluenetRebootData.ipcDataMajor = BLUENET_IPC_BLUENET_REBOOT_DATA_MAJOR;
	ipcData.bluenetRebootData.ipcDataMinor = BLUENET_IPC_BLUENET_REBOOT_DATA_MINOR;

	memset(ipcData.bluenetRebootData.microapp, 0, sizeof(ipcData.bluenetRebootData.microapp));
	ipcData.bluenetRebootData.microapp[appIndex].running = runFlag;

	IpcRetCode ipcCode = setRamData(IPC_INDEX_BLUENET_TO_BLUENET, ipcData.raw, sizeof(ipcData.bluenetRebootData));
	if (ipcCode != IPC_RET_SUCCESS) {
		LOGw("Failed to set IPC data: ipcCode=%i", ipcCode);
	}
}

MicroappOperatingState MicroappController::getOperatingState(uint8_t appIndex) {
	LOGd("getOperatingState appIndex=%u", appIndex);
	MicroappOperatingState state = MicroappOperatingState::CS_MICROAPP_NOT_RUNNING;
	if (appIndex > 0) {
		LOGi("Multiple apps not supported yet");
		return state;
	}
	bluenet_ipc_data_payload_t ipcData;
	uint8_t dataSize   = 0;

	// We might read the IPC data of a previous bluenet version.
	IpcRetCode ipcCode = getRamData(IPC_INDEX_BLUENET_TO_BLUENET, ipcData.raw, &dataSize, sizeof(ipcData.raw));
	if (ipcCode != IPC_RET_SUCCESS) {
		LOGi("Failed to get IPC data: ipcCode=%i", ipcCode);
		return state;
	}
	if (ipcData.bluenetRebootData.ipcDataMajor != BLUENET_IPC_BLUENET_REBOOT_DATA_MAJOR) {
		LOGw("Incorrect major version: major=%u required=%u",
			 ipcData.bluenetRebootData.ipcDataMajor,
			 BLUENET_IPC_BLUENET_REBOOT_DATA_MAJOR);
		return state;
	}

	// We need to ignore this warning, it's triggered because the minimum minor is 0, making the statement always false.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
	if (ipcData.bluenetRebootData.ipcDataMinor < BLUENET_IPC_BLUENET_REBOOT_DATA_MINOR) {
		LOGw("Minor version too low: minor=%u minimum=%u",
			 ipcData.bluenetRebootData.ipcDataMinor,
			 BLUENET_IPC_BLUENET_REBOOT_DATA_MINOR);
		return state;
	}
#pragma GCC diagnostic pop

	switch (ipcData.bluenetRebootData.microapp[appIndex].running) {
		case 1: {
			state = MicroappOperatingState::CS_MICROAPP_RUNNING;
			break;
		}
		default: {
			// non-running state as default
		}
	}
	return state;
}

void MicroappController::callMicroapp() {
	const uint8_t appIndex = 0;
	setOperatingState(appIndex, MicroappOperatingState::CS_MICROAPP_RUNNING);
	if (resumeCoroutine()) {
		setOperatingState(appIndex, MicroappOperatingState::CS_MICROAPP_NOT_RUNNING);
		return;
	}

	// Should only happen if microapp actually ends (and does not yield anymore).
	LOGe("End of coroutine. Should not happen.")
}

bool MicroappController::handleAck() {
	uint8_t* outputBuffer                 = getOutputMicroappBuffer();
	microapp_sdk_header_t* outgoingHeader = reinterpret_cast<microapp_sdk_header_t*>(outputBuffer);
	LogMicroappControllerVerbose("handleAck: [ack %u]", outgoingHeader->ack);
	bool inInterruptContext = (outgoingHeader->ack != CS_MICROAPP_SDK_ACK_NO_REQUEST);
	if (!inInterruptContext) {
		return true;
	}
	bool interruptDone = (outgoingHeader->ack != CS_MICROAPP_SDK_ACK_IN_PROGRESS);
	if (!interruptDone) {
		return true;
	}
	bool interruptDropped = (outgoingHeader->ack == CS_MICROAPP_SDK_ACK_ERR_BUSY);
	if (interruptDropped) {
		LogMicroappControllerVerbose("Microapp is busy, drop interrupt");
		// Also prevent new interrupts since apparently the microapp has no more space
		setEmptySoftInterruptSlots(0);
	}
	else {
		LogMicroappControllerVerbose("Finished interrupt with return code %u", outgoingHeader->ack);
		// Increment number of empty interrupt slots since we just finished one
		incrementEmptySoftInterruptSlots();
	}
	// If interrupt finished, we do not call again and we also don't handle the microapp request
	_consecutiveMicroappCallCounter = 0;
	return false;
}

bool MicroappController::handleRequest() {
	uint8_t* inputBuffer                  = getInputMicroappBuffer();
	microapp_sdk_header_t* incomingHeader = reinterpret_cast<microapp_sdk_header_t*>(inputBuffer);
	MicroappRequestHandler& microappRequestHandler = MicroappRequestHandler::getInstance();
	cs_ret_code_t result                           = microappRequestHandler.handleMicroappRequest(incomingHeader);
	LogMicroappControllerVerbose("  ack=%u", incomingHeader->ack);

	// TODO: put result in ack, instead of letting the handler(s) set the ack.
	switch (result) {
		case ERR_SUCCESS:
		case ERR_SUCCESS_NO_CHANGE: {
			break;
		}
		case ERR_WAIT_FOR_SUCCESS: {
			LOGi("Handling request of type %u is in progress", incomingHeader->messageType);
			break;
		}
		default: {
			LOGi("Handling request of type %u failed with return code %u", incomingHeader->messageType, result);
			break;
		}
	}
	bool callAgain = !stopAfterMicroappRequest(incomingHeader);
	if (!callAgain) {
		LogMicroappControllerVerbose("Do not call again");
		_consecutiveMicroappCallCounter = 0;
		return false;
	}
	// Also check if the max number of consecutive nonyielding calls is reached
	if (_consecutiveMicroappCallCounter >= MICROAPP_MAX_NUMBER_CONSECUTIVE_CALLS) {
		_consecutiveMicroappCallCounter = 0;
		LOGi("Stop because we've reached a max # of consecutive calls");
		return false;
	}
	_consecutiveMicroappCallCounter++;
	return true;
}

bool MicroappController::stopAfterMicroappRequest(microapp_sdk_header_t* incomingHeader) {
	switch (incomingHeader->messageType) {
		case CS_MICROAPP_SDK_TYPE_LOG:
		case CS_MICROAPP_SDK_TYPE_PIN:
		case CS_MICROAPP_SDK_TYPE_SWITCH:
		case CS_MICROAPP_SDK_TYPE_SERVICE_DATA:
		case CS_MICROAPP_SDK_TYPE_TWI:
		case CS_MICROAPP_SDK_TYPE_BLE:
		case CS_MICROAPP_SDK_TYPE_MESH:
		case CS_MICROAPP_SDK_TYPE_POWER_USAGE:
		case CS_MICROAPP_SDK_TYPE_PRESENCE:
		case CS_MICROAPP_SDK_TYPE_CONTROL_COMMAND: {
			return false;
		}
		case CS_MICROAPP_SDK_TYPE_NONE:
		case CS_MICROAPP_SDK_TYPE_YIELD:
		case CS_MICROAPP_SDK_TYPE_CONTINUE: {
			return true;
		}
		default: {
			LOGi("Unknown request type: %u", incomingHeader->messageType);
			return true;
		}
	}
}

/*
 * Called from cs_Microapp every time tick. The microapp does not set anything in RAM but will only read from RAM and
 * call a handler.
 *
 * There's no load failure detection. When the call fails bluenet hangs and should reboot.
 */
void MicroappController::tickMicroapp(uint8_t appIndex) {
	_tickCounter++;
	if (_tickCounter < MICROAPP_LOOP_FREQUENCY) {
		return;
	}
	_tickCounter                           = 0;
	// Reset interrupt counter every microapp tick
	_softInterruptCounter                  = 0;
	// Indicate to the microapp that this is a tick entry by writing in outgoing message header
	uint8_t* outputBuffer                  = getOutputMicroappBuffer();
	microapp_sdk_header_t* outgoingMessage = reinterpret_cast<microapp_sdk_header_t*>(outputBuffer);

	outgoingMessage->messageType           = CS_MICROAPP_SDK_TYPE_CONTINUE;
	outgoingMessage->ack                   = CS_MICROAPP_SDK_ACK_NO_REQUEST;
	bool callAgain                         = false;
	bool ignoreRequest                     = false;
	int8_t repeatCounter                   = 0;
	do {
		LogMicroappControllerVerbose("tickMicroapp [call %i]", repeatCounter);
		callMicroapp();
		ignoreRequest = !handleAck();
		if (ignoreRequest) {
			break;
		}
		callAgain = handleRequest();
		repeatCounter++;
	} while (callAgain);
	LogMicroappControllerVerbose("tickMicroapp end");
}

/*
 * Write the callback to the microapp and have it execute it. We can have calls to bluenet within the interrupt.
 * Hence, we call handleRequest after this if the interrupt is not finished.
 * Note that although we do throttle the number of consecutive calls, this does not throttle the callbacks themselves.
 */
void MicroappController::generateSoftInterrupt() {
	// This is probably already checked before this function call, but let's do it anyway to be sure
	if (!allowSoftInterrupts()) {
		return;
	}
	if (_softInterruptCounter == MICROAPP_MAX_SOFT_INTERRUPTS_WITHIN_A_TICK - 1) {
		LogMicroappControllerVerbose("Last callback (next one in next tick)");
	}
	_softInterruptCounter++;

	uint8_t* outputBuffer                    = getOutputMicroappBuffer();
	microapp_sdk_header_t* outgoingInterrupt = reinterpret_cast<microapp_sdk_header_t*>(outputBuffer);

	// Request an acknowledgement by the microapp indicating status of interrupt
	outgoingInterrupt->ack                   = CS_MICROAPP_SDK_ACK_REQUEST;
	bool callAgain                           = false;
	bool ignoreRequest                       = false;
	int8_t repeatCounter                     = 0;
	do {
		LogMicroappControllerVerbose(
				"generateSoftInterrupt [call %i, %i interrupts within tick]", repeatCounter, _softInterruptCounter);
		callMicroapp();
		ignoreRequest = !handleAck();
		if (ignoreRequest) {
			break;
		}
		callAgain = handleRequest();
		repeatCounter++;
	} while (callAgain);
	LogMicroappControllerVerbose("generateSoftInterrupt end");
}

/*
 * Attempt registration of an interrupt. An interrupt registration is uniquely identified
 * by a type (=messageType, see enum MicroappSdkMessageType) and an id which identifies the interrupt
 * registration within the scope of the type.
 */
cs_ret_code_t MicroappController::registerSoftInterrupt(MicroappSdkMessageType type, uint8_t id) {
	// Check if interrupt registration already exists
	if (isSoftInterruptRegistered(type, id)) {
		LOGi("Interrupt [%i, %u] already registered", type, id);
		return ERR_ALREADY_EXISTS;
	}
	// Look for first empty slot, if it exists
	int emptySlotIndex = -1;
	for (int i = 0; i < MICROAPP_MAX_SOFT_INTERRUPT_REGISTRATIONS; ++i) {
		if (!_softInterruptRegistrations[i].registered) {
			emptySlotIndex = i;
			break;
		}
	}
	if (emptySlotIndex < 0) {
		LOGw("No empty interrupt registration slots left");
		return ERR_NO_SPACE;
	}
	// Register the interrupt
	_softInterruptRegistrations[emptySlotIndex].registered = true;
	_softInterruptRegistrations[emptySlotIndex].type       = type;
	_softInterruptRegistrations[emptySlotIndex].id         = id;

	LogMicroappControllerVerbose("Registered soft interrupt of type %i, id %u", type, id);

	return ERR_SUCCESS;
}

/*
 * Check whether an interrupt registration already exists
 */
bool MicroappController::isSoftInterruptRegistered(MicroappSdkMessageType type, uint8_t id) {
	for (int i = 0; i < MICROAPP_MAX_SOFT_INTERRUPT_REGISTRATIONS; ++i) {
		if (_softInterruptRegistrations[i].registered) {
			if (_softInterruptRegistrations[i].type == type && _softInterruptRegistrations[i].id == id) {
				return true;
			}
		}
	}
	return false;
}

/*
 * Check whether new interrupts can be generated
 */
bool MicroappController::allowSoftInterrupts() {
	// if the microapp dropped the last one and hasn't finished an interrupt,
	// we won't try to call it with a new interrupt
	if (_emptySoftInterruptSlots == 0) {
		LogMicroappControllerDebug("No empty interrupt slots");
		return false;
	}
	// Check if we already exceeded the max number of interrupts in this tick
	if (_softInterruptCounter >= MICROAPP_MAX_SOFT_INTERRUPTS_WITHIN_A_TICK) {
		LogMicroappControllerDebug("Too many soft interrupts");
		return false;
	}
	return true;
}

/*
 * Set the number of empty interrupt slots.
 * This function can be used upon microapp yield requests, which contain an emptyInterruptSlots field
 */
void MicroappController::setEmptySoftInterruptSlots(uint8_t emptySlots) {
	_emptySoftInterruptSlots = emptySlots;
}

/*
 * Increment the number of empty interrupt slots.
 * This function can be used when the microapp finishes handling an interrupt.
 * That means a slot will have been freed at the microapp side.
 */
void MicroappController::incrementEmptySoftInterruptSlots() {
	// Make sure we don't overflow to zero in extreme cases
	if (_emptySoftInterruptSlots == 0xFF) {
		return;
	}
	_emptySoftInterruptSlots++;
}

void MicroappController::clear(uint8_t appIndex) {
	LOGi("Clear appIndex=%u", appIndex);
	for (int i = 0; i < MICROAPP_MAX_SOFT_INTERRUPT_REGISTRATIONS; ++i) {
		_softInterruptRegistrations[i] = {};
	}
	_emptySoftInterruptSlots = 1;

	microappData.isScanning = false;

}
