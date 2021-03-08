/*
 * Author: Crownstone Team
 * Copyright: Crownstone (https://crownstone.rocks)
 * Date: Jun 3, 2020
 * License: LGPLv3+, Apache License 2.0, and/or MIT (triple-licensed)
 */

#pragma once

#include <cstdint>

/**
 * Max number of microapps.
 */
const uint8_t MAX_MICROAPPS = 1;

/**
 * Max allowed chunk size when uploading a microapp.
 *
 * We could calculate this from MTU or characteristic buffer size for BLE, and UART RX buffer size for UART.
 * But let's just start with a number that fits in both.
 */
const uint16_t MICROAPP_UPLOAD_MAX_CHUNK_SIZE = 256;

/**
 * Protocol version of the communication between the user and the firmware: the microapp command and result packets.
 */
const uint8_t MICROAPP_PROTOCOL = 1;

/**
 * Header of a microapp binary.
 *
 * Has to match section .firmware_header in linker file nrf_common.ld of the microapp repo.
 */
typedef struct __attribute__((__packed__)) microapp_binary_header_t {
	uint32_t startAddress;     // Address of first function to call.

	uint8_t sdkVersionMajor;   // Similar to microapp_sdk_version_t
	uint8_t sdkVersionMinor;
	uint16_t size;             // Size of the binary, including this header.

	uint16_t checksum;         // Checksum of the binary, after this header.
	uint16_t checksumHeader;   // Checksum of this header, with this field set to 0.

	uint32_t appBuildVersion;  // Build version of this microapp.

	uint32_t reserved;         // Reserved for future use, must be 0 for now.
};


/*
nr  | Type name                | Payload type           | Result payload  | Description
--- | ------------------------ | ---------------------- | --------------- | -----------
90  | Upload microapp          | microapp_upload_t      | -               | Upload (part of) a microapp.
91  | Validate microapp upload | microapp_ctrl_header_t | -               | Validate upload of microapp, checks if CRC matches.
92  | Remove microapp          | microapp_ctrl_header_t | -               | Remove a microapp.
93  | Enable microapp          | microapp_ctrl_header_t | -               | Enable a microapp, checks if protocol is supported.
94  | Disable microapp         | microapp_ctrl_header_t | -               | Disable a microapp.
95  | Get microapp info        | -                      | microapp_info_t | Get info about supported microapps and status of all microapps.
*/

struct __attribute__((packed)) microapp_ctrl_header_t {
	uint8_t protocol;   // Protocol of the microapp command and result packets, should match MICROAPP_PROTOCOL.
	uint8_t index;
};

struct __attribute__((packed)) microapp_upload_t {
	microapp_ctrl_header_t header;
	uint16_t offset;    // Offset in bytes of this chunk of data. Must be a multiple of 4.
	uint16_t totalSize; // Size of the complete microapp binary.
//	uint8[] data;       // A chunk of the microapp binary.
};

/**
 * SDK version: determines the API / protocol between microapp and firmware.
 */
struct __attribute__((packed)) microapp_sdk_version_t {
	uint8_t major;
	uint8_t minor;
};

/**
 * State of tests of a microapp, also stored in flash.
 */
struct __attribute__((packed)) microapp_state_t {
	bool enabled: 1;              // Whether the microapp is enabled.
	uint8_t checksum: 2;          // values: untested, passed, failed
	uint8_t memoryUsage: 1;       // values: ok, excessive
	uint8_t boot: 2;              // Values: untested, trying, passed, failed. Checks if the microapp starts, registers callback function in IPC, and returns to firmware.
	uint16_t reserved: 10;        // Reserved, must be 0 for now.
	uint8_t tryingFunction;       // Index of registered function that didn't pass yet, and that we are calling now.
	uint8_t failedFunction;       // Index of registered function that was tried, but didn't pass.
	uint32_t passedFunctions;     // Bitmask of registered functions that were called and returned to firmware successfully.
};

/**
 * Status of a microapp.
 */
struct __attribute__((packed)) microapp_status_t {
	uint32_t buildVersion;             // Build version of this microapp.
	microapp_sdk_version_t sdkVersion; // SDK version this microapp was built for.
	microapp_state_t state;
};

/**
 * Packet with all info required to upload a microapp, and to see the status of already uploaded microapps.
 */
struct __attribute__((packed)) microapp_info_t {
	uint8_t protocol = MICROAPP_PROTOCOL;   // Protocol of this packet, and the microapp command packets.
	uint8_t maxApps = MAX_MICROAPPS;        // Maximum number of packets.
	uint16_t maxAppSize;                    // Maximum binary size of a microapp.
	uint16_t maxChunkSize;                  // Maximum chunk size for uploading a microapp.
	uint16_t maxRamUsage;                   // Maximum RAM usage of a microapp.
	microapp_sdk_version_t sdkVersion;      // SDK version the firmware supports.
	microapp_status_t appsStatus[MAX_MICROAPPS];
};
