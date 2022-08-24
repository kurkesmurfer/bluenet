/*
 * Microapp structs.
 *
 * Author: Crownstone Team
 * Copyright: Crownstone (https://crownstone.rocks)
 * Date: Dec 10, 2020
 * License: LGPLv3+, Apache License 2.0, and/or MIT (triple-licensed)
 */
#pragma once

#ifdef __cplusplus
#include <cstdint>
#else
#include <stdint.h>
#endif

/*
 * Externally determined constant sizes
 */

// Standard MAC address length
const uint8_t MAC_ADDRESS_LENGTH                            = 6;
// Defined by the BLE SIG
const uint8_t MAX_BLE_ADV_DATA_LENGTH                       = 31;
// Defined by the mesh protocol
const uint8_t MAX_MICROAPP_MESH_PAYLOAD_SIZE                = 7;
// Defined by the service data packet service_data_encrypted_microapp_t
const uint8_t MICROAPP_SDK_MAX_SERVICE_DATA_LENGTH          = 8;
/*
 * Payload and header sizes (may need to be updated when structs are changed)
 */

// Maximum total payload (somewhat arbitrary, should be able to contain most-used data structures e.g. BLE
// advertisements)
const uint8_t MICROAPP_SDK_MAX_PAYLOAD                      = 48;
// messageType [1] + ack [1]
const uint8_t MICROAPP_SDK_HEADER_SIZE                      = 2;
// header + type [1] + flags [1] + size [1]
const uint8_t MICROAPP_SDK_LOG_HEADER_SIZE                  = MICROAPP_SDK_HEADER_SIZE + 3;
// max total - (header + twiType [1] + twiAddress [1] + twiFlags [1] + twiPayloadSize [1])
const uint8_t MICROAPP_SDK_MAX_TWI_PAYLOAD_SIZE             = MICROAPP_SDK_MAX_PAYLOAD - (MICROAPP_SDK_HEADER_SIZE + 4);
// max total - log header
const uint8_t MICROAPP_SDK_MAX_STRING_LENGTH                = MICROAPP_SDK_MAX_PAYLOAD - MICROAPP_SDK_LOG_HEADER_SIZE;
// max total - log header
const uint8_t MICROAPP_SDK_MAX_ARRAY_SIZE                   = MICROAPP_SDK_MAX_PAYLOAD - MICROAPP_SDK_LOG_HEADER_SIZE;
// max total - (header + protocol [1] + type [2] + size [2])
const uint8_t MICROAPP_SDK_MAX_CONTROL_COMMAND_PAYLOAD_SIZE = MICROAPP_SDK_MAX_PAYLOAD - (MICROAPP_SDK_HEADER_SIZE + 5);

// Call loop every 10 ticks. The ticks are every 100 ms so this means every second.
#define MICROAPP_LOOP_FREQUENCY 10

#ifndef TICK_INTERVAL_MS
#define TICK_INTERVAL_MS 100
#endif

#define MICROAPP_LOOP_INTERVAL_MS (TICK_INTERVAL_MS * MICROAPP_LOOP_FREQUENCY)

/**
 * Arguments for the opcode as first argument in the callback from the microapp to bluenet.
 */
enum CallbackMicroappOpcode {
	CS_MICROAPP_CALLBACK_NONE             = 0x00,
	CS_MICROAPP_CALLBACK_SIGNAL           = 0x01,
	CS_MICROAPP_CALLBACK_UPDATE_IO_BUFFER = 0x02,
};

/**
 * Acknowledgments from microapp to bluenet or the other way around.
 */
enum MicroappSdkAck {
	//! Ack successfull return value
	CS_MICROAPP_SDK_ACK_SUCCESS             = 0x00,

	// Ack requests (should not be interpreted as a return value)

	//! Explicitly do not ask for an acknowledgement
	CS_MICROAPP_SDK_ACK_NO_REQUEST          = 0x01,
	//! Request for other process (microapp or bluenet) to overwrite this field
	CS_MICROAPP_SDK_ACK_REQUEST             = 0x02,

	// Ack return values

	//! So far so good, but not done yet
	CS_MICROAPP_SDK_ACK_IN_PROGRESS         = 0x03,
	//! Unspecified error
	CS_MICROAPP_SDK_ACK_ERROR               = 0x04,
	//! A requested entity could not be found
	CS_MICROAPP_SDK_ACK_ERR_NOT_FOUND       = 0x05,
	//! The request cannot be interpreted fully
	CS_MICROAPP_SDK_ACK_ERR_UNDEFINED       = 0x06,
	//! There is no space to fulfill a request
	CS_MICROAPP_SDK_ACK_ERR_NO_SPACE        = 0x07,
	//! The request can be interpreted but is not implemented yet
	CS_MICROAPP_SDK_ACK_ERR_NOT_IMPLEMENTED = 0x08,
	//! The request cannot be fulfilled because of other ongoing requests
	CS_MICROAPP_SDK_ACK_ERR_BUSY            = 0x09,
	//! A parameter in the request is out of range
	CS_MICROAPP_SDK_ACK_ERR_OUT_OF_RANGE    = 0x0A,
	//! Request requires functionality that is disabled
	CS_MICROAPP_SDK_ACK_ERR_DISABLED        = 0x0B,
	//! Request or its parameters are empty
	CS_MICROAPP_SDK_ACK_ERR_EMPTY           = 0x0C,
	//! Request or its parameters are too large
	CS_MICROAPP_SDK_ACK_ERR_TOO_LARGE       = 0x0D,
};

typedef MicroappSdkAck microapp_sdk_result_t;

/**
 * The main opcodes for microapp commands.
 */
enum MicroappSdkMessageType {
	//! No meaning, should not be used
	CS_MICROAPP_SDK_TYPE_NONE            = 0x00,
	//! Microapp logs
	CS_MICROAPP_SDK_TYPE_LOG             = 0x01,
	//! GPIO related
	CS_MICROAPP_SDK_TYPE_PIN             = 0x02,
	//! Switch and dimmer commands
	CS_MICROAPP_SDK_TYPE_SWITCH          = 0x03,
	//! Microapp service data updates
	CS_MICROAPP_SDK_TYPE_SERVICE_DATA    = 0x04,
	//! TWI related
	CS_MICROAPP_SDK_TYPE_TWI             = 0x05,
	//! BLE related (excluding mesh)
	CS_MICROAPP_SDK_TYPE_BLE             = 0x06,
	//! Mesh related
	CS_MICROAPP_SDK_TYPE_MESH            = 0x07,
	//! Power usage related
	CS_MICROAPP_SDK_TYPE_POWER_USAGE     = 0x08,
	//! Presence related
	CS_MICROAPP_SDK_TYPE_PRESENCE        = 0x09,
	//! Generic control command according to the control command protocol
	CS_MICROAPP_SDK_TYPE_CONTROL_COMMAND = 0x0A,
	//! Microapp yielding to bluenet without expecting a direct return call
	CS_MICROAPP_SDK_TYPE_YIELD           = 0x0B,
	//! Bluenet calling the microapp on a tick or subsequent call
	CS_MICROAPP_SDK_TYPE_CONTINUE        = 0x0C,
};

/**
 * Type of log indicating how to interpret the log payload
 */
enum MicroappSdkLogType {
	//! Char or byte
	CS_MICROAPP_SDK_LOG_CHAR   = 0x01,
	//! Signed int (32-bit)
	CS_MICROAPP_SDK_LOG_INT    = 0x02,
	//! String or char array, same as arr
	CS_MICROAPP_SDK_LOG_STR    = 0x03,
	//! Byte array, same as str
	CS_MICROAPP_SDK_LOG_ARR    = 0x04,
	//! Float
	CS_MICROAPP_SDK_LOG_FLOAT  = 0x05,
	//! Double
	CS_MICROAPP_SDK_LOG_DOUBLE = 0x06,
	//! Unsigned int (32-bit)
	CS_MICROAPP_SDK_LOG_UINT   = 0x07,
	//! Unsigned short (16-bit)
	CS_MICROAPP_SDK_LOG_SHORT  = 0x08,
};

/**
 * Flags for logging. Currently only using a newline flag
 */
enum MicroappSdkLogFlags {
	//! Can be used to clear all flags
	CS_MICROAPP_SDK_LOG_FLAG_CLEAR   = 0,
	//! Add a newline character
	CS_MICROAPP_SDK_LOG_FLAG_NEWLINE = (1 << 0),
};

/**
 * Indicates the GPIO pins of the hardware.
 * Pin functionality can be used for crownstones that have exposed GPIO pins only
 */
enum MicroappSdkPin {
	CS_MICROAPP_SDK_PIN_GPIO0   = 0x00,
	CS_MICROAPP_SDK_PIN_GPIO1   = 0x01,
	CS_MICROAPP_SDK_PIN_GPIO2   = 0x02,
	CS_MICROAPP_SDK_PIN_GPIO3   = 0x03,
	CS_MICROAPP_SDK_PIN_GPIO4   = 0x04,
	CS_MICROAPP_SDK_PIN_GPIO5   = 0x05,
	CS_MICROAPP_SDK_PIN_GPIO6   = 0x06,
	CS_MICROAPP_SDK_PIN_GPIO7   = 0x07,
	CS_MICROAPP_SDK_PIN_GPIO8   = 0x08,
	CS_MICROAPP_SDK_PIN_GPIO9   = 0x09,
	CS_MICROAPP_SDK_PIN_BUTTON1 = 0x0A,
	CS_MICROAPP_SDK_PIN_BUTTON2 = 0x0B,
	CS_MICROAPP_SDK_PIN_BUTTON3 = 0x0C,
	CS_MICROAPP_SDK_PIN_BUTTON4 = 0x0D,
	CS_MICROAPP_SDK_PIN_LED1    = 0x0E,
	CS_MICROAPP_SDK_PIN_LED2    = 0x0F,
	CS_MICROAPP_SDK_PIN_LED3    = 0x10,
	CS_MICROAPP_SDK_PIN_LED4    = 0x11,
};

/**
 * Indicates whether the pin is to be initialized (MODE) or perform an action (ACTION)
 */
enum MicroappSdkPinType {
	//! Initialize the pin with a polarity and a direction and register an interrupt
	CS_MICROAPP_SDK_PIN_INIT   = 0x01,
	//! An action such as read the value of a pin or write to it
	CS_MICROAPP_SDK_PIN_ACTION = 0x02,
};

/**
 * Directionality of the GPIO pin (input or output)
 */
enum MicroappSdkPinDirection {
	//! Set pin as input, but do not use a pulling resistor
	CS_MICROAPP_SDK_PIN_INPUT        = 0x01,
	//! Set pin as input using a pullup resistor
	CS_MICROAPP_SDK_PIN_INPUT_PULLUP = 0x02,
	//! Set pin as output
	CS_MICROAPP_SDK_PIN_OUTPUT       = 0x03,
};

/**
 * Polarity of pin for initializing pin interrupts (only for input pins)
 */
enum MicroappSdkPinPolarity {
	//! Not sensing for a specific event
	CS_MICROAPP_SDK_PIN_NO_POLARITY = 0x01,
	//! Low to high or High to low
	CS_MICROAPP_SDK_PIN_CHANGE      = 0x02,
	//! Low to high
	CS_MICROAPP_SDK_PIN_RISING      = 0x03,
	//! High to low
	CS_MICROAPP_SDK_PIN_FALLING     = 0x04,
};

/**
 * Type of action to perform on a pin, either read or write
 */
enum MicroappSdkPinActionType {
	CS_MICROAPP_SDK_PIN_READ  = 0x01,
	CS_MICROAPP_SDK_PIN_WRITE = 0x02,
};

/**
 * Value to either read from the pin or write to the pin
 */
enum MicroappSdkPinValue {
	CS_MICROAPP_SDK_PIN_OFF = 0x00,
	CS_MICROAPP_SDK_PIN_ON  = 0x01,
};

/**
 * Switch value according to same protocol as switch command value over BLE and UART
 * Values between 0 and 100 can be used for dimming
 */
enum MicroappSdkSwitchValue {
	//! 0 = fully off
	CS_MICROAPP_SDK_SWITCH_OFF       = 0x00,
	//! 100 = fully on
	CS_MICROAPP_SDK_SWITCH_ON        = 0x64,
	//! Switch off when currently on, switch to smart on when currently off
	CS_MICROAPP_SDK_SWITCH_TOGGLE    = 0xFD,
	//! Switch to the value according to behaviour rules
	CS_MICROAPP_SDK_SWITCH_BEHAVIOUR = 0xFE,
	//! Switch on, the value will be determined by behaviour rules
	CS_MICROAPP_SDK_SWITCH_SMART_ON  = 0xFF,
};

/**
 * Type of TWI request
 */
enum MicroappSdkTwiType {
	CS_MICROAPP_SDK_TWI_READ  = 0x01,
	CS_MICROAPP_SDK_TWI_WRITE = 0x02,
	CS_MICROAPP_SDK_TWI_INIT  = 0x03,
};

/**
 * Flags for TWI requests
 */
enum MicroappSdkTwiFlags {
	//! Can be used to clear all flags
	CS_MICROAPP_SDK_TWI_FLAG_CLEAR = 0,
	//! Stop bit
	CS_MICROAPP_SDK_TWI_FLAG_STOP  = (1 << 0),
};

/**
 * Type of BLE request, indicating how to interpret the rest of the request
 */
enum MicroappSdkBleType {
	//! Invalid type
	CS_MICROAPP_SDK_BLE_NONE                          = 0x00,

	// Scan related message types

	//! Start forwarding scanned devices to the microapp
	CS_MICROAPP_SDK_BLE_SCAN_START                    = 0x01,
	//! Stop forwarding scanned devices to the microapp
	CS_MICROAPP_SDK_BLE_SCAN_STOP                     = 0x02,
	//! Register an interrupt for incoming scanned devices
	CS_MICROAPP_SDK_BLE_SCAN_REGISTER_INTERRUPT       = 0x03,
	//! Bluenet has scanned a device. Used for interrupts
	CS_MICROAPP_SDK_BLE_SCAN_SCANNED_DEVICE           = 0x04,

	// Connection related message types

	//! Request a connection to a peripheral
	CS_MICROAPP_SDK_BLE_CONNECTION_REQUEST_CONNECT    = 0x05,
	//! Bluenet -> microapp when connected to a peripheral
	CS_MICROAPP_SDK_BLE_CONNECTION_CONNECTED          = 0x06,
	//! Request disconnecting from a peripheral
	CS_MICROAPP_SDK_BLE_CONNECTION_REQUEST_DISCONNECT = 0x07,
	//! Bluenet -> microapp when disconnected from a peripheral
	CS_MICROAPP_SDK_BLE_CONNECTION_DISCONNECTED       = 0x08,
};

/**
 * Mesh request types
 */
enum MicroappSdkMeshType {
	//! Send a mesh message from the microapp
	CS_MICROAPP_SDK_MESH_SEND        = 0x01,
	//! Start listening for mesh messages of the microapp type, and register an interrupt on the bluenet side
	CS_MICROAPP_SDK_MESH_LISTEN      = 0x02,
	//! Request for information about the mesh configuration. For now consists only of the own stone ID
	CS_MICROAPP_SDK_MESH_READ_CONFIG = 0x03,
	//! Received a mesh message. Used for interrupts from bluenet
	CS_MICROAPP_SDK_MESH_READ        = 0x04,
};

/**
 * Types of power usage to reqeust
 */
enum MicroappSdkPowerUsageType {
	//! Get filtered power data in milliWatt
	CS_MICROAPP_SDK_POWER_USAGE_POWER   = 0x01,
	//! Not implemented yet
	CS_MICROAPP_SDK_POWER_USAGE_CURRENT = 0x02,
	//! Not implemented yet
	CS_MICROAPP_SDK_POWER_USAGE_VOLTAGE = 0x03,
};

/**
 * Type of yield from the microapp to bluenet
 */
enum MicroappSdkYieldType {
	//! End of setup
	CS_MICROAPP_SDK_YIELD_SETUP = 0x01,
	//! End of loop
	CS_MICROAPP_SDK_YIELD_LOOP  = 0x02,
	//! The microapp is doing something asynchronous like a delay call
	CS_MICROAPP_SDK_YIELD_ASYNC = 0x03,
};

/**
 * A single buffer (can be either input or output).
 */
struct __attribute__((packed)) io_buffer_t {
	uint8_t payload[MICROAPP_SDK_MAX_PAYLOAD];
};

/**
 * Combined input and output buffer.
 */
struct __attribute__((packed)) bluenet_io_buffers_t {
	io_buffer_t microapp2bluenet;
	io_buffer_t bluenet2microapp;
};

typedef microapp_sdk_result_t (*microappCallbackFunc)(uint8_t opcode, bluenet_io_buffers_t*);

/*
 * The layout of the struct in ramdata. We set for the microapp a protocol version so it can check itself if it is
 * compatible. The length parameter functions as a extra possible check. The callback can be used by the microapp to
 * call back into bluenet. The pointer to the coargs struct can be used to switch back from the used coroutine and
 * needs to stored somewhere accessible.
 */
struct __attribute__((packed)) bluenet2microapp_ipcdata_t {
	uint8_t protocol;
	uint8_t length;
	microappCallbackFunc microappCallback;
	bool valid;
};

/**
 * Header for io buffers shared between bluenet and microapp. The payload of the io buffer always starts with this
 * header.
 */
struct __attribute__((packed)) microapp_sdk_header_t {
	/**
	 * Specifies the type of message, and how to interpret the rest of the payload.
	 * See MicroappSdkMessageType.
	 */
	uint8_t messageType;

	/**
	 * Used for requesting and receiving acks. Can be used for identifying requests and interrupts.
	 * See MicroappSdkAck.
	 */
	int8_t ack;
};

static_assert(sizeof(microapp_sdk_header_t) == MICROAPP_SDK_HEADER_SIZE);

/**
 * Header for log commands. Excludes the actual log payload, which is different for every log type.
 */
struct __attribute__((packed)) microapp_sdk_log_header_t {
	microapp_sdk_header_t header;

	//! Specifies what type of payload it carries. See MicroappSdkLogType.
	uint8_t type;

	//! Flags for logging. See MicroappSdkLogFlags.
	uint8_t flags;

	//! Length of the payload for type STR or ARR.
	uint8_t size;
};

static_assert(sizeof(microapp_sdk_log_header_t) <= MICROAPP_SDK_LOG_HEADER_SIZE);

// Char
struct __attribute__((packed)) microapp_sdk_log_char_t {
	microapp_sdk_log_header_t logHeader;
	uint8_t value;
};

static_assert(sizeof(microapp_sdk_log_char_t) <= MICROAPP_SDK_MAX_PAYLOAD);

// Short
struct __attribute__((packed)) microapp_sdk_log_short_t {
	microapp_sdk_log_header_t logHeader;
	uint16_t value;
};

static_assert(sizeof(microapp_sdk_log_short_t) <= MICROAPP_SDK_MAX_PAYLOAD);

// Uint
struct __attribute__((packed)) microapp_sdk_log_uint_t {
	microapp_sdk_log_header_t logHeader;
	uint32_t value;
};

static_assert(sizeof(microapp_sdk_log_uint_t) <= MICROAPP_SDK_MAX_PAYLOAD);

// Int
struct __attribute__((packed)) microapp_sdk_log_int_t {
	microapp_sdk_log_header_t logHeader;
	int32_t value;
};

static_assert(sizeof(microapp_sdk_log_int_t) <= MICROAPP_SDK_MAX_PAYLOAD);

// Float
struct __attribute__((packed)) microapp_sdk_log_float_t {
	microapp_sdk_log_header_t logHeader;
	float value;
};

static_assert(sizeof(microapp_sdk_log_float_t) <= MICROAPP_SDK_MAX_PAYLOAD);

// Double
struct __attribute__((packed)) microapp_sdk_log_double_t {
	microapp_sdk_log_header_t logHeader;
	double value;
};

static_assert(sizeof(microapp_sdk_log_double_t) <= MICROAPP_SDK_MAX_PAYLOAD);

// String
struct __attribute__((packed)) microapp_sdk_log_string_t {
	microapp_sdk_log_header_t logHeader;
	char str[MICROAPP_SDK_MAX_STRING_LENGTH];
};

static_assert(sizeof(microapp_sdk_log_string_t) <= MICROAPP_SDK_MAX_PAYLOAD);

// Char array
struct __attribute__((packed)) microapp_sdk_log_array_t {
	microapp_sdk_log_header_t logHeader;
	char arr[MICROAPP_SDK_MAX_ARRAY_SIZE];
};

static_assert(sizeof(microapp_sdk_log_array_t) <= MICROAPP_SDK_MAX_PAYLOAD);

/**
 * Struct to control GPIO pins. Pins can be initialized, read or written.
 */
struct __attribute__((packed)) microapp_sdk_pin_t {
	microapp_sdk_header_t header;

	//! Specifies the GPIO pin, button or led. See MicroappSdkPin.
	uint8_t pin;

	//! Specifies whether to initialize (INIT) or read/write (ACTION). See MicroappSdkPinType.
	uint8_t type;

	//!  Specifies whether to set the pin as input or output. Only used with type INIT. See MicroappSdkPinDirection.
	uint8_t direction;

	//! Specifies which pin events to watch. Only used with type INIT. See MicroappSdkPinPolarity.
	uint8_t polarity;

	//! Specifies whether to read from or write to pin. Only used with type ACTION. See MicroappSdkPinActionType.
	uint8_t action;

	/**
	 * Specifies value to write to pin, or field to put read value. Only used with type ACTION.
	 * See MicroappSdkPinValue.
	 */
	uint8_t value;
};

static_assert(sizeof(microapp_sdk_pin_t) <= MICROAPP_SDK_MAX_PAYLOAD);

/**
 * Struct for switching and dimming the crownstone. Conforms to the general control command protocol
 */
struct __attribute__((packed)) microapp_sdk_switch_t {
	microapp_sdk_header_t header;

	//!  Specifies what action to take. See MicroappSdkSwitchValue.
	uint8_t value;
};

static_assert(sizeof(microapp_sdk_switch_t) <= MICROAPP_SDK_MAX_PAYLOAD);

/**
 * Struct for microapp service data to be advertised by bluenet
 */
struct __attribute__((packed)) microapp_sdk_service_data_t {
	microapp_sdk_header_t header;

	//! Unique app identifier that will be advertised along with the payload.
	uint16_t appUuid;

	//! Size of the payload.
	uint8_t size;

	//! The payload.
	uint8_t data[MICROAPP_SDK_MAX_SERVICE_DATA_LENGTH];
};

static_assert(sizeof(microapp_sdk_service_data_t) <= MICROAPP_SDK_MAX_PAYLOAD);

/**
 * Struct for i2c/twi initialization, writes, and reads.
 */
struct __attribute__((packed)) microapp_sdk_twi_t {
	microapp_sdk_header_t header;

	//! Specifies what action to take. See MicroappSdkTwiType.
	uint8_t type;

	//! Slave address to write to.
	uint8_t address;

	//! Flags for the command. See MicroappSdkTwiFlags.
	uint8_t flags;

	//! Size of the payload.
	uint8_t size;

	//! The payload.
	uint8_t buf[MICROAPP_SDK_MAX_TWI_PAYLOAD_SIZE];
};

static_assert(sizeof(microapp_sdk_twi_t) <= MICROAPP_SDK_MAX_PAYLOAD);

/**
 * Struct for Bluetooth Low Energy related messages, excluding mesh. Includes scanning and connecting
 */
struct __attribute__((packed)) microapp_sdk_ble_t {
	microapp_sdk_header_t header;

	//! Specifies the type of message. See MicroappSdkBleType.
	uint8_t type;

	//! Type of address
	uint8_t address_type;

	//! Big-endian MAC address. Context depends on type field
	uint8_t address[MAC_ADDRESS_LENGTH];

	//! Received signal strength. For type SCANNED_DEVICE, this is the RSSI to the device.
	int8_t rssi;

	//! Size of the payload.
	uint8_t size;

	//! The payload. For type SCANNED_DEVICE, this is the advertisement data.
	uint8_t data[MAX_BLE_ADV_DATA_LENGTH];
};

static_assert(sizeof(microapp_sdk_ble_t) <= MICROAPP_SDK_MAX_PAYLOAD);

/**
 * Struct for mesh message from microapp.
 */
struct __attribute__((packed)) microapp_sdk_mesh_t {
	microapp_sdk_header_t header;

	//! Specifies the type of message. See MicroappSdkMeshType.
	uint8_t type;

	//! Indicates the stone id to send to/read from or own stone ID. Use ID 0 for a broadcast.
	uint8_t stoneId;

	//! Size of the payload.
	uint8_t size;

	//! The payload.
	uint8_t data[MAX_MICROAPP_MESH_PAYLOAD_SIZE];
};

static_assert(sizeof(microapp_sdk_mesh_t) <= MICROAPP_SDK_MAX_PAYLOAD);

/**
 * Struct for microapp power usage requests
 */
struct __attribute__((packed)) microapp_sdk_power_usage_t {
	microapp_sdk_header_t header;

	//! Specifies the type of power usage requested. See MicroappSdkPowerUsageType.
	uint8_t type;

	//!  The power usage. Units vary based on type
	int32_t powerUsage;
};

static_assert(sizeof(microapp_sdk_power_usage_t) <= MICROAPP_SDK_MAX_PAYLOAD);

/**
 * Struct for microapp presence requests
 */
struct __attribute__((packed)) microapp_sdk_presence_t {
	microapp_sdk_header_t header;

	//! Specifies the profile for which the presence is requested.
	uint8_t profileId;

	//! A bitmask where each bit indicates the presence of the profile in a specific location.
	uint64_t presenceBitmask;
};

static_assert(sizeof(microapp_sdk_presence_t) <= MICROAPP_SDK_MAX_PAYLOAD);

/**
 * Struct with payload conforming to control command protocol for direct handling by command handler
 * See https://github.com/crownstoen/bluenet/blob/master/docs/protocol/PROTOCOL.md#control-packet
 */
struct __attribute__((packed)) microapp_sdk_control_command_t {
	microapp_sdk_header_t header;

	//! Control command protocol.
	uint8_t protocol;

	//! The type of control command.
	uint16_t type;

	//! Size of the payload.
	uint16_t size;

	//! The payload.
	uint8_t payload[MICROAPP_SDK_MAX_CONTROL_COMMAND_PAYLOAD_SIZE];
};

static_assert(sizeof(microapp_sdk_control_command_t) <= MICROAPP_SDK_MAX_PAYLOAD);

/**
 * Struct for microapp yielding to bluenet, e.g. upon completing a setup or loop call, or within an async call (e.g.
 * delay)
 */
struct __attribute__((packed)) microapp_sdk_yield_t {
	microapp_sdk_header_t header;

	//! The type of yield. See the MicroappSdkYieldType.
	uint8_t type;

	//! Number of empty slots for interrupts the microapp has. If zero, block new interrupts.
	uint8_t emptyInterruptSlots;
};

static_assert(sizeof(microapp_sdk_yield_t) <= MICROAPP_SDK_MAX_PAYLOAD);
