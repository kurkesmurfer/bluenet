# @argument INCLUDE_BASE_DIR: determines the base directory for the includes.

set(CMAKE_INCLUDE_SYSTEM_FLAG_C "-isystem ")
set(CMAKE_INCLUDE_SYSTEM_FLAG_CXX "-isystem ")

# Add separately include directories if we are to lazy to specify e.g. "#include <protocol/rbc_mesh.h>"
# We have removed include/third/nrf from here. This needs to be specified separately for the firmware versus the
# bootloader. Both need namely a different sdk_config.h file...

list(APPEND NRF5_SDK_INCLUDES_REL "include")
list(APPEND NRF5_SDK_INCLUDES_REL "include/ble")
list(APPEND NRF5_SDK_INCLUDES_REL "include/third")
list(APPEND NRF5_SDK_INCLUDES_REL "shared")

foreach(REL_FILE IN LISTS NRF5_SDK_INCLUDES_REL)
	include_directories(${REL_FILE})
endforeach()


message(STATUS "INCLUDE_BASE_DIR for nrf5-sdk-includes: ${INCLUDE_BASE_DIR}")

include_directories(SYSTEM "${INCLUDE_BASE_DIR}")
include_directories(SYSTEM "${INCLUDE_BASE_DIR}/components/toolchain/cmsis/include")
include_directories(SYSTEM "${INCLUDE_BASE_DIR}/config")
include_directories(SYSTEM "${INCLUDE_BASE_DIR}/components/libraries/fstorage")
include_directories(SYSTEM "${INCLUDE_BASE_DIR}/components/libraries/experimental_section_vars")
include_directories(SYSTEM "${INCLUDE_BASE_DIR}/components/libraries/queue")
include_directories(SYSTEM "${INCLUDE_BASE_DIR}/components/libraries/util")
include_directories(SYSTEM "${INCLUDE_BASE_DIR}/components/ble/ble_db_discovery")
include_directories(SYSTEM "${INCLUDE_BASE_DIR}/components/ble/nrf_ble_gq")

include_directories(SYSTEM "${INCLUDE_BASE_DIR}/components/ble/ble_services")
include_directories(SYSTEM "${INCLUDE_BASE_DIR}/components/ble/common")
include_directories(SYSTEM "${INCLUDE_BASE_DIR}/components/device/")
include_directories(SYSTEM "${INCLUDE_BASE_DIR}/components/drivers_nrf/common/")
include_directories(SYSTEM "${INCLUDE_BASE_DIR}/components/drivers_nrf/comp/")
include_directories(SYSTEM "${INCLUDE_BASE_DIR}/components/libraries/atomic/")
include_directories(SYSTEM "${INCLUDE_BASE_DIR}/components/libraries/atomic_fifo/")
include_directories(SYSTEM "${INCLUDE_BASE_DIR}/components/libraries/balloc/")
include_directories(SYSTEM "${INCLUDE_BASE_DIR}/components/libraries/crc16/")
include_directories(SYSTEM "${INCLUDE_BASE_DIR}/components/libraries/crc32/")
include_directories(SYSTEM "${INCLUDE_BASE_DIR}/components/libraries/delay/")
include_directories(SYSTEM "${INCLUDE_BASE_DIR}/components/libraries/experimental_log/")
include_directories(SYSTEM "${INCLUDE_BASE_DIR}/components/libraries/experimental_log/src/")
include_directories(SYSTEM "${INCLUDE_BASE_DIR}/components/libraries/experimental_memobj/")
include_directories(SYSTEM "${INCLUDE_BASE_DIR}/components/libraries/experimental_section_vars/")
include_directories(SYSTEM "${INCLUDE_BASE_DIR}/components/libraries/fds/")
include_directories(SYSTEM "${INCLUDE_BASE_DIR}/components/libraries/fstorage/")
include_directories(SYSTEM "${INCLUDE_BASE_DIR}/components/libraries/hardfault/")
include_directories(SYSTEM "${INCLUDE_BASE_DIR}/components/libraries/log/")
include_directories(SYSTEM "${INCLUDE_BASE_DIR}/components/libraries/log/src/")
include_directories(SYSTEM "${INCLUDE_BASE_DIR}/components/libraries/memobj/")
include_directories(SYSTEM "${INCLUDE_BASE_DIR}/components/libraries/ringbuf/")
include_directories(SYSTEM "${INCLUDE_BASE_DIR}/components/libraries/scheduler/")
include_directories(SYSTEM "${INCLUDE_BASE_DIR}/components/libraries/strerror/")
include_directories(SYSTEM "${INCLUDE_BASE_DIR}/components/libraries/timer/")
include_directories(SYSTEM "${INCLUDE_BASE_DIR}/components/libraries/trace/")
include_directories(SYSTEM "${INCLUDE_BASE_DIR}/components/libraries/util/")
include_directories(SYSTEM "${INCLUDE_BASE_DIR}/components/softdevice/common/")
include_directories(SYSTEM "${INCLUDE_BASE_DIR}/components/softdevice/common/softdevice_handler/")
include_directories(SYSTEM "${INCLUDE_BASE_DIR}/components/softdevice/s${SOFTDEVICE_SERIES}/headers")
include_directories(SYSTEM "${INCLUDE_BASE_DIR}/components/toolchain/")
include_directories(SYSTEM "${INCLUDE_BASE_DIR}/components/toolchain/cmsis/include")
include_directories(SYSTEM "${INCLUDE_BASE_DIR}/components/toolchain/gcc/")
include_directories(SYSTEM "${INCLUDE_BASE_DIR}/external/fprintf/")
include_directories(SYSTEM "${INCLUDE_BASE_DIR}/integration/nrfx/")
include_directories(SYSTEM "${INCLUDE_BASE_DIR}/modules/nrfx/")
include_directories(SYSTEM "${INCLUDE_BASE_DIR}/modules/nrfx/drivers/include/")
include_directories(SYSTEM "${INCLUDE_BASE_DIR}/modules/nrfx/hal/")
include_directories(SYSTEM "${INCLUDE_BASE_DIR}/modules/nrfx/mdk/")
include_directories(SYSTEM "${INCLUDE_BASE_DIR}/external/segger_rtt")
include_directories(SYSTEM "${INCLUDE_BASE_DIR}/integration/nrfx/legacy/")

include_directories(SYSTEM "${INCLUDE_BASE_DIR}/components/softdevice/s132/headers/nrf52")
