/*
 * Author: Anne van Rossum
 * Copyright: Crownstone B.V.
 * Date: Mar 1, 2018
 * License: LGPLv3+, MIT, Apache
 */
#pragma once

#include <cstdlib>

#include <cfg/cs_Config.h>
#include <nrf.h>  

// The index type for a buffer is just a minimal-sized unsigned char as offset into an array.
typedef uint8_t buffer_id_t;

// The index type for a channel (a buffer within one of the buffers), organized like [A B A B A B]
typedef uint8_t channel_id_t;

// The index type for a value can be negative to support padding.
typedef int8_t value_id_t;

// The value type
typedef nrf_saadc_value_t value_t;

// The number of buffers is to be defined statically
static const uint8_t INTERLEAVED_BUFFER_COUNT = CS_ADC_NUM_BUFFERS;

// The length of the complete buffer (including both channels) is also defined statically
static const value_t INTERLEAVED_BUFFER_LENGTH = CS_ADC_BUF_SIZE;

#if CS_ADC_BUF_SIZE % 2 == 1
#error "Buffer size needs to be even with current channel count"
#endif

// The number of channels
static const uint8_t INTERLEAVED_CHANNEL_COUNT = 2;

// The length of an individual channel within a buffer
static const value_t INTERLEAVED_CHANNEL_LENGTH = INTERLEAVED_BUFFER_LENGTH / INTERLEAVED_CHANNEL_COUNT;

/** Interleaved Buffer implementation
 *
 */
class InterleavedBuffer {
private:

	nrf_saadc_value_t* _buf[INTERLEAVED_BUFFER_COUNT];

public:
	/**
	 * Singleton implementation. There is no foreseen need to have multiple InterleavedBuffer objects instantiated.
	 * Note that this singleton implementation does not have dynamic memory allocation.
	 */
	static InterleavedBuffer& getInstance() {
		static InterleavedBuffer instance;
		return instance;
	}

	/**
	 * From the _buf array pick the one with the given buffer_id.
	 */
	nrf_saadc_value_t* getBuffer(buffer_id_t buffer_id) {
		assert (buffer_id < getBufferCount(), "ADC has fewer buffers allocated");
		return _buf[buffer_id];
	}

	/**
	 * Set the buffer at the particular buffer_id by just writing a pointer to it. The code calling this function is
	 * responsible for setting the right pointer. No checks w.r.t. this pointer are performed.
	 */
	void setBuffer(buffer_id_t buffer_id, nrf_saadc_value_t* ptr) {
		assert (buffer_id < getBufferCount(), "ADC has fewer buffers allocated");
		_buf[buffer_id] = ptr;
	}

	/**
	 * Expose buffer lengths to accessors.
	 */
	inline const value_t getBufferLength() {
		return INTERLEAVED_BUFFER_LENGTH;
	}

	/**
	 * Get number of buffers.
	 */
	inline const value_t getBufferCount() {
		return INTERLEAVED_BUFFER_COUNT;
	}
	
	/**
	 * Expose channel lengths to accessors.
	 */
	inline const value_t getChannelLength() {
		return INTERLEAVED_CHANNEL_LENGTH;
	}
	
	/**
	 * Get number of channels.
	 */
	inline const value_t getChannelCount() {
		return INTERLEAVED_CHANNEL_COUNT;
	}

	/**
	 * Given a pointer to a buffer return the buffer_id.
	 */
	cs_adc_buffer_id_t getIndex(nrf_saadc_value_t *buffer) {
		for (cs_adc_buffer_id_t i = 0; i < getBufferCount(); ++i) {
			if (buffer == _buf[i]) 
				return i;
		}
		return getBufferCount();
	}

	/**
	 * Clear the pointer to a buffer.
	 */
	void clearBuffer(buffer_id_t buffer_id) {
		setBuffer(buffer_id, NULL);
	}

	/**
	 * Uses getIndex to find out if this pointer exists. 
	 */
	bool exists(nrf_saadc_value_t *buffer) {
		return (getIndex(buffer) != getBufferCount());
	}

	/**
	 * Checks if the pointer is set.
	 */
	bool exists(buffer_id_t buffer_id) {
		return (_buf[buffer_id] != NULL);
	}

	/**
	 * Get the previous buffer index. This will just use the total static buffer count and use a modulus operation to 
	 * wrap around the first buffer.
	 *
	 * @param[in] buffer_id                      Index to the buffer (0 up to getBufferCount() - 1)
	 * @return                                   Index to the previous buffer
	 */
	inline buffer_id_t getPrevious(buffer_id_t buffer_id) {
		return (buffer_id + getBufferCount() - 1) % getBufferCount();
	}

	/**
	 * Get the next buffer index. This will just use the total static buffer count and use a modulus operation to 
	 * wrap around the first buffer.
	 *
	 * @param[in] buffer_id                      Index to the buffer (0 up to getBufferCount() - 1)
	 * @return                                   Index to the next buffer
	 */
	inline buffer_id_t getNext(buffer_id_t buffer_id) {
		return (buffer_id + 1) % getBufferCount();
	}

	/**
	 * Get a particular value from a buffer. We allow some "magic" behavior here. The value_id is a signed value. If it
	 * is negative it retrieves a value from the previous buffer. If it is larger than getBufferLength() - 1 it
	 * retrieves a value from the next buffer. 
	 *
	 * The value_id is a reference to the index of a value within a channel (half of the buffer length).
	 *
	 * @param[in] buffer_id                      Index to the buffer (0 up to getBufferCount() - 1)
	 * @param[in] channel_id                     Particular channel within this buffer (0 or 1)
	 * @param[in] value_id                       Index to the value in the buffer (0 up to getChannelLength() - 1)
	 * @return                                   Particular value
	 */
	nrf_saadc_value_t getValue(buffer_id_t buffer_id, channel_id_t channel_id, value_id_t value_id) {
		value_t value;
		nrf_saadc_value_t* buf;
		if (value_id < 0) {
			// use previous buffer, e.g. for padding
			
			buffer_id_t prev_buffer_id = getPrevious(buffer_id);
			bool buffer_exists = exists(prev_buffer_id);
			assert (buffer_exists, "Previous buffer does not exist!");

			// a value such as -1 will be multiplied by 2, hence -2 and then used to count from the back of the 
			// previous buffer, say of length 100, value_id = -1 will hence retrieve the last item from the
			// previous buffer, prev_buffer[98] or prev_buffer[99] depending on channel_id
			value_id_t prev_value_id_in_channel = getBufferLength() + (value_id * getChannelCount()) + channel_id;
			buf = getBuffer(prev_buffer_id);
			value = buf[prev_value_id_in_channel];

		} else if (value_id >= getChannelLength()) {
			// use next buffer, e.g. for padding
			
			buffer_id_t next_buffer_id = getNext(buffer_id);
			bool buffer_exists = exists(next_buffer_id);
			assert (buffer_exists, "Next buffer does not exist!");

			// a value such as 101 will be subtracted by the buffer length, say 100, hence result is 1, then
			// the next buffer will be queried at next_buffer[2] or next_buffer[3] dependin on channel_id
			value_id_t next_value_id_in_channel = (value_id - getChannelLength()) * getChannelCount() + channel_id;
			buf = getBuffer(next_buffer_id);
			value = buf[next_value_id_in_channel];

		} else {
			// return value in given buffer
			value_id_t value_id_in_channel = value_id * getChannelCount() + channel_id;
			buf = getBuffer(buffer_id);
			value = buf[value_id_in_channel];
		}

		return value;
	}

	/**
	 * For in-place filtering it is necessary to write a particular value into the buffer. 
	 *
	 * @param[in] buffer_id                      Index to the buffer (0 up to getBufferCount() - 1)
	 * @param[in] channel_id                     Particular channel within this buffer (0 or 1)
	 * @param[in] value_id                       Index to the value in the buffer
	 * @param[in] value                          Value to be written to the buffer
	 */
	void setValue(buffer_id_t buffer_id, channel_id_t channel_id, value_id_t value_id, value_t value) {
		assert(value_id >= 0, "value id should be positive");
		assert(value_id < getChannelLength(), "value id should be smaller than channel size");
		nrf_saadc_value_t* buf;
		value_id_t value_id_in_channel = value_id * getChannelCount() + channel_id;
		buf = getBuffer(buffer_id);
		buf[value_id_in_channel] = value;
	}

};
