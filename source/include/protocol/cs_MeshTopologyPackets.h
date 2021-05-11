/*
 * Author: Crownstone Team
 * Copyright: Crownstone (https://crownstone.rocks)
 * Date: Apr 30, 2021
 * License: LGPLv3+, Apache License 2.0, and/or MIT (triple-licensed)
 */

#pragma once

#include <cstdint>
#include <protocol/cs_Typedefs.h>
#include <util/cs_Rssi.h>

static constexpr uint8_t MESH_TOPOLOGY_CHANNEL_COUNT = 3;

struct __attribute__((packed)) neighbour_node_t {
	stone_id_t id;
	compressed_rssi_data_t compressedRssi;
	uint8_t lastSeenSeconds; // Last seen N seconds ago.

	neighbour_node_t() {};
	neighbour_node_t(stone_id_t id, compressed_rssi_data_t rssiData):
		id(id),
		compressedRssi(rssiData),
		lastSeenSeconds(0)
	{}
};

/**
 * Message format to be sent over uart.
 * This is the inflated counterpart of rssi_data_message_t.
 *
 * (Necessary since we have to fold in our own id anyway.)
 */
struct __attribute__((packed)) mesh_topology_neighbour_rssi_t {
	stone_id_t receiverId;
	stone_id_t senderId;
	uint8_t count[MESH_TOPOLOGY_CHANNEL_COUNT];
	uint8_t rssi[MESH_TOPOLOGY_CHANNEL_COUNT];
	uint8_t standardDeviation[MESH_TOPOLOGY_CHANNEL_COUNT];
};

