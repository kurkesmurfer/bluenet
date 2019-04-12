# Collect flags that are used in the code, as macros
SET(CONFIG_FILE ${CMAKE_SOURCE_DIR}/CMakeConfig.cmake)
IF(EXISTS "${CONFIG_FILE}")
	MESSAGE(STATUS "crownstone.defs.cmake: Load config file ${CONFIG_FILE}")
	INCLUDE(${CONFIG_FILE})
ELSE()
	MESSAGE(FATAL_ERROR "crownstone.defs.cmake: Cannot find config file ${CONFIG_FILE}")
ENDIF()

ADD_DEFINITIONS("-MMD -DNRF52 -DNRF52_SERIES -DEPD_ENABLE_EXTRA_RAM -DNRF51_USE_SOFTDEVICE=${NRF51_USE_SOFTDEVICE} -DUSE_RENDER_CONTEXT -DSYSCALLS -DUSING_FUNC -DDEBUG_NRF")

LIST(APPEND CUSTOM_DEFINITIONS, TEMPERATURE)

ADD_DEFINITIONS("-DBLE_STACK_SUPPORT_REQD")

IF(NRF51_USE_SOFTDEVICE)
	ADD_DEFINITIONS("-DSOFTDEVICE_PRESENT")
ENDIF()

MESSAGE(STATUS "crownstone.defs.cmake: Build type: ${CMAKE_BUILD_TYPE}")
IF(CMAKE_BUILD_TYPE MATCHES "Debug")
	ADD_DEFINITIONS("-DGIT_HASH=${GIT_HASH}")
	ADD_DEFINITIONS("-DGIT_BRANCH=${GIT_BRANCH}")
ENDIF()

# The bluetooth name is not optional
IF(DEFINED BLUETOOTH_NAME)
	ADD_DEFINITIONS("-DBLUETOOTH_NAME=${BLUETOOTH_NAME}")
ELSE()
	MESSAGE(FATAL_ERROR "We require a BLUETOOTH_NAME in CMakeBuild.config (5 characters or less), i.e. \"Crown\" (with quotes)")
ENDIF()

# For mesh SDK:
#IF (DEFINED BUILD_MESHING AND "${BUILD_MESHING}" STRGREATER "0")
IF (BUILD_MESHING)
	ADD_DEFINITIONS("-DS${SOFTDEVICE_SERIES}")
	ADD_DEFINITIONS("-DCONFIG_APP_IN_CORE")
	ADD_DEFINITIONS("-DNRF52832")
#	ADD_DEFINITIONS("-DNRF52832_XXAA")
	ADD_DEFINITIONS("-DNRF_SD_BLE_API_VERSION=${SOFTDEVICE_MAJOR}")
#	ADD_DEFINITIONS("-DPERSISTENT_STORAGE=1")
	ADD_DEFINITIONS("-DSOFTDEVICE_PRESENT")
	SET(EXPERIMENTAL_INSTABURST_ENABLED OFF)
	IF (EXPERIMENTAL_INSTABURST_ENABLED)
		ADD_DEFINITIONS("-DEXPERIMENTAL_INSTABURST_ENABLED")
	ENDIF()
	SET(MESH_MEM_BACKEND "stdlib")
	# HF timer peripheral index to allocate for bearer handler. E.g. if set to 2, NRF_TIMER2 will be used. Must be a literal number.
	ADD_DEFINITIONS("-DBEARER_ACTION_TIMER_INDEX=3")
ENDIF()

# Pass variables in defined in the configuration file to the compiler
ADD_DEFINITIONS("-DNRF5_DIR=${NRF5_DIR}")
ADD_DEFINITIONS("-DNORDIC_SDK_VERSION=${NORDIC_SDK_VERSION}")
ADD_DEFINITIONS("-DSOFTDEVICE_SERIES=${SOFTDEVICE_SERIES}")
ADD_DEFINITIONS("-DSOFTDEVICE_MAJOR=${SOFTDEVICE_MAJOR}")
ADD_DEFINITIONS("-DSOFTDEVICE_MINOR=${SOFTDEVICE_MINOR}")
ADD_DEFINITIONS("-DSOFTDEVICE=${SOFTDEVICE}")
ADD_DEFINITIONS("-DSOFTDEVICE_NO_SEPARATE_UICR_SECTION=${SOFTDEVICE_NO_SEPARATE_UICR_SECTION}")
ADD_DEFINITIONS("-DAPPLICATION_START_ADDRESS=${APPLICATION_START_ADDRESS}")
ADD_DEFINITIONS("-DAPPLICATION_LENGTH=${APPLICATION_LENGTH}")
ADD_DEFINITIONS("-DCOMPILATION_TIME=${COMPILATION_TIME}")
ADD_DEFINITIONS("-DCOMPILATION_DAY=${COMPILATION_DAY}")
ADD_DEFINITIONS("-DDEFAULT_HARDWARE_BOARD=${DEFAULT_HARDWARE_BOARD}")
ADD_DEFINITIONS("-DSERIAL_VERBOSITY=${SERIAL_VERBOSITY}")
ADD_DEFINITIONS("-DMASTER_BUFFER_SIZE=${MASTER_BUFFER_SIZE}")
ADD_DEFINITIONS("-DDEFAULT_ON=${DEFAULT_ON}")
ADD_DEFINITIONS("-DRSSI_ENABLE=${RSSI_ENABLE}")
ADD_DEFINITIONS("-DTX_POWER=${TX_POWER}")
ADD_DEFINITIONS("-DADVERTISEMENT_INTERVAL=${ADVERTISEMENT_INTERVAL}")
ADD_DEFINITIONS("-DLOW_POWER_MODE=${LOW_POWER_MODE}")
ADD_DEFINITIONS("-DPWM_ENABLE=${PWM_ENABLE}")
ADD_DEFINITIONS("-DFIRMWARE_VERSION=${FIRMWARE_VERSION}")
ADD_DEFINITIONS("-DBOOT_DELAY=${BOOT_DELAY}")
ADD_DEFINITIONS("-DSCAN_DURATION=${SCAN_DURATION}")
ADD_DEFINITIONS("-DSCAN_BREAK_DURATION=${SCAN_BREAK_DURATION}")
ADD_DEFINITIONS("-DMAX_CHIP_TEMP=${MAX_CHIP_TEMP}")
ADD_DEFINITIONS("-DINTERVAL_SCANNER_ENABLED=${INTERVAL_SCANNER_ENABLED}")
#ADD_DEFINITIONS("-DCONTINUOUS_POWER_SAMPLER=${CONTINUOUS_POWER_SAMPLER}")
ADD_DEFINITIONS("-DDEFAULT_OPERATION_MODE=${DEFAULT_OPERATION_MODE}")
ADD_DEFINITIONS("-DPERSISTENT_FLAGS_DISABLED=${PERSISTENT_FLAGS_DISABLED}")
ADD_DEFINITIONS("-DMESH_INTERVAL_MIN_MS=${MESH_INTERVAL_MIN_MS}")
ADD_DEFINITIONS("-DMESH_BOOT_TIME=${MESH_BOOT_TIME}")
ADD_DEFINITIONS("-DNRF_SERIES=${NRF_SERIES}")
ADD_DEFINITIONS("-DRAM_R1_BASE=${RAM_R1_BASE}")
ADD_DEFINITIONS("-DMAX_NUM_VS_SERVICES=${MAX_NUM_VS_SERVICES}")
ADD_DEFINITIONS("-DADVERTISEMENT_IMPROVEMENT=${ADVERTISEMENT_IMPROVEMENT}")
ADD_DEFINITIONS("-DCONNECTION_ALIVE_TIMEOUT=${CONNECTION_ALIVE_TIMEOUT}")
ADD_DEFINITIONS("-DCS_UART_BINARY_PROTOCOL_ENABLED=${CS_UART_BINARY_PROTOCOL_ENABLED}")
ADD_DEFINITIONS("-DCS_SERIAL_ENABLED=${CS_SERIAL_ENABLED}")
IF(DEFINED RELAY_DEFAULT_ON)
ADD_DEFINITIONS("-DRELAY_DEFAULT_ON=${RELAY_DEFAULT_ON}")
ENDIF()


# Define PANs
ADD_DEFINITIONS("-DNRF52_PAN_12")
ADD_DEFINITIONS("-DNRF52_PAN_15")
ADD_DEFINITIONS("-DNRF52_PAN_20")
#ADD_DEFINITIONS("-DNRF52_PAN_28") # Although rev1 shouldn't have this PAN, defining it did fix our code
ADD_DEFINITIONS("-DNRF52_PAN_31")
ADD_DEFINITIONS("-DNRF52_PAN_36")
ADD_DEFINITIONS("-DNRF52_PAN_51")
ADD_DEFINITIONS("-DNRF52_PAN_54")
ADD_DEFINITIONS("-DNRF52_PAN_55")
ADD_DEFINITIONS("-DNRF52_PAN_58")
ADD_DEFINITIONS("-DNRF52_PAN_64")
ADD_DEFINITIONS("-DNRF52_PAN_66")
ADD_DEFINITIONS("-DNRF52_PAN_67")
ADD_DEFINITIONS("-DNRF52_PAN_68")
ADD_DEFINITIONS("-DNRF52_PAN_72")
ADD_DEFINITIONS("-DNRF52_PAN_74")
ADD_DEFINITIONS("-DNRF52_PAN_75")
ADD_DEFINITIONS("-DNRF52_PAN_76")
ADD_DEFINITIONS("-DNRF52_PAN_77")
ADD_DEFINITIONS("-DNRF52_PAN_78")
ADD_DEFINITIONS("-DNRF52_PAN_79")
ADD_DEFINITIONS("-DNRF52_PAN_81")
ADD_DEFINITIONS("-DNRF52_PAN_83")
ADD_DEFINITIONS("-DNRF52_PAN_84")
ADD_DEFINITIONS("-DNRF52_PAN_86")
ADD_DEFINITIONS("-DNRF52_PAN_87")
ADD_DEFINITIONS("-DNRF52_PAN_88")
ADD_DEFINITIONS("-DNRF52_PAN_89")
ADD_DEFINITIONS("-DNRF52_PAN_91")
ADD_DEFINITIONS("-DNRF52_PAN_97")
ADD_DEFINITIONS("-DNRF52_PAN_101")
ADD_DEFINITIONS("-DNRF52_PAN_108")
ADD_DEFINITIONS("-DNRF52_PAN_109")
ADD_DEFINITIONS("-DNRF52_PAN_113")
ADD_DEFINITIONS("-DNRF52_PAN_132")
ADD_DEFINITIONS("-DNRF52_PAN_136")
ADD_DEFINITIONS("-DNRF52_PAN_138")
ADD_DEFINITIONS("-DNRF52_PAN_141")

# UICR settings
ADD_DEFINITIONS("-DUICR_DFU_INDEX=${UICR_DFU_INDEX}")
ADD_DEFINITIONS("-DUICR_BOARD_INDEX=${UICR_BOARD_INDEX}")
ADD_DEFINITIONS("-DHARDWARE_BOARD_ADDRESS=${HARDWARE_BOARD_ADDRESS}")

# Set Attribute table size
ADD_DEFINITIONS("-DATTR_TABLE_SIZE=${ATTR_TABLE_SIZE}")

# Add encryption
ADD_DEFINITIONS("-DENCRYPTION=${ENCRYPTION}")

ADD_DEFINITIONS("-DMESHING=${MESHING}")
ADD_DEFINITIONS("-DBUILD_MESHING=${BUILD_MESHING}")
ADD_DEFINITIONS("-DDIMMING=${DIMMING}")

# Mesh Settings
ADD_DEFINITIONS("-DMESH_USE_APP_SCHEDULER")
ADD_DEFINITIONS("-DRBC_MESH_DEBUG=${RBC_MESH_DEBUG}")
ADD_DEFINITIONS("-DRBC_MESH_HANDLE_CACHE_ENTRIES=${RBC_MESH_HANDLE_CACHE_ENTRIES}")
ADD_DEFINITIONS("-DRBC_MESH_DATA_CACHE_ENTRIES=${RBC_MESH_DATA_CACHE_ENTRIES}")
ADD_DEFINITIONS("-DRBC_MESH_APP_EVENT_QUEUE_LENGTH=${RBC_MESH_APP_EVENT_QUEUE_LENGTH}")
ADD_DEFINITIONS("-DRADIO_PCNF1_MAXLEN=${RADIO_PCNF1_MAXLEN}")
ADD_DEFINITIONS("-DRADIO_PCNF0_S1LEN=${RADIO_PCNF0_S1LEN}")
ADD_DEFINITIONS("-DRADIO_PCNF0_LFLEN=${RADIO_PCNF0_LFLEN}")

IF(DEFINED RBC_MESH_VALUE_MAX_LEN)
	ADD_DEFINITIONS("-DRBC_MESH_VALUE_MAX_LEN=${RBC_MESH_VALUE_MAX_LEN}")
ENDIF()

# Add iBeacon default values
ADD_DEFINITIONS("-DIBEACON=${IBEACON}")
#IF(IBEACON)
ADD_DEFINITIONS("-DBEACON_UUID=${BEACON_UUID}")
ADD_DEFINITIONS("-DBEACON_MAJOR=${BEACON_MAJOR}")
ADD_DEFINITIONS("-DBEACON_MINOR=${BEACON_MINOR}")
ADD_DEFINITIONS("-DBEACON_RSSI=${BEACON_RSSI}")
#ENDIF()

ADD_DEFINITIONS("-DEDDYSTONE=${EDDYSTONE}")
ADD_DEFINITIONS("-DCHANGE_NAME_ON_RESET=${CHANGE_NAME_ON_RESET}")

# Add services
ADD_DEFINITIONS("-DCROWNSTONE_SERVICE=${CROWNSTONE_SERVICE}")
ADD_DEFINITIONS("-DINDOOR_SERVICE=${INDOOR_SERVICE}")
ADD_DEFINITIONS("-DGENERAL_SERVICE=${GENERAL_SERVICE}")
ADD_DEFINITIONS("-DPOWER_SERVICE=${POWER_SERVICE}")
ADD_DEFINITIONS("-DSCHEDULE_SERVICE=${SCHEDULE_SERVICE}")

# Add characteristics
ADD_DEFINITIONS("-DCHAR_CONTROL=${CHAR_CONTROL}")
ADD_DEFINITIONS("-DCHAR_MESHING=${CHAR_MESHING}")
ADD_DEFINITIONS("-DCHAR_TEMPERATURE=${CHAR_TEMPERATURE}")
ADD_DEFINITIONS("-DCHAR_RESET=${CHAR_RESET}")
ADD_DEFINITIONS("-DCHAR_CONFIGURATION=${CHAR_CONFIGURATION}")
ADD_DEFINITIONS("-DCHAR_STATE=${CHAR_STATE}")
ADD_DEFINITIONS("-DCHAR_PWM=${CHAR_PWM}")
ADD_DEFINITIONS("-DCHAR_SAMPLE_CURRENT=${CHAR_SAMPLE_CURRENT}")
ADD_DEFINITIONS("-DCHAR_CURRENT_LIMIT=${CHAR_CURRENT_LIMIT}")
ADD_DEFINITIONS("-DCHAR_RSSI=${CHAR_RSSI}")
ADD_DEFINITIONS("-DCHAR_SCAN_DEVICES=${CHAR_SCAN_DEVICES}")
ADD_DEFINITIONS("-DCHAR_TRACK_DEVICES=${CHAR_TRACK_DEVICES}")
ADD_DEFINITIONS("-DCHAR_RELAY=${CHAR_RELAY}")
ADD_DEFINITIONS("-DCHAR_SCHEDULE=${CHAR_SCHEDULE}")

# only required if Nordic files are used
ADD_DEFINITIONS("-DBOARD_NRF6310")



# Publish all options as CMake options as well

# Obtain variables to be used for the compiler
SET(NRF5_DIR                                    "${NRF5_DIR}"                   CACHE STRING "Nordic SDK Directory" FORCE)
SET(NORDIC_SDK_VERSION                          "${NORDIC_SDK_VERSION}"             CACHE STRING "Nordic SDK Version" FORCE)
SET(SOFTDEVICE_SERIES                           "${SOFTDEVICE_SERIES}"              CACHE STRING "SOFTDEVICE_SERIES" FORCE)
SET(SOFTDEVICE_MAJOR                            "${SOFTDEVICE_MAJOR}"               CACHE STRING "SOFTDEVICE_MAJOR" FORCE)
SET(SOFTDEVICE_MINOR                            "${SOFTDEVICE_MINOR}"               CACHE STRING "SOFTDEVICE_MINOR" FORCE)
SET(SOFTDEVICE                                  "${SOFTDEVICE}"                     CACHE STRING "SOFTDEVICE" FORCE)
SET(SOFTDEVICE_NO_SEPARATE_UICR_SECTION         "${SOFTDEVICE_NO_SEPARATE_UICR_SECTION}"   CACHE STRING "SOFTDEVICE_NO_SEPARATE_UICR_SECTION" FORCE)
SET(APPLICATION_START_ADDRESS                   "${APPLICATION_START_ADDRESS}"      CACHE STRING "APPLICATION_START_ADDRESS" FORCE)
SET(APPLICATION_LENGTH                          "${APPLICATION_LENGTH}"             CACHE STRING "APPLICATION_LENGTH" FORCE)
SET(COMPILATION_TIME                            "${COMPILATION_TIME}"               CACHE STRING "COMPILATION_TIME" FORCE)
SET(COMPILATION_DAY                             "${COMPILATION_DAY}"                CACHE STRING "COMPILATION_DAY" FORCE)
IF(DEFINED GIT_HASH)
SET(GIT_HASH                                    "${GIT_HASH}"                       CACHE STRING "GIT_HASH" FORCE)
ENDIF()
#SET(HARDWARE_BOARD                              "${HARDWARE_BOARD}"                 CACHE STRING "HARDWARE_BOARD" FORCE)
SET(SERIAL_VERBOSITY                            "${SERIAL_VERBOSITY}"               CACHE STRING "SERIAL_VERBOSITY" FORCE)
SET(CS_SERIAL_ENABLED                           "${CS_SERIAL_ENABLED}"              CACHE STRING "CS_SERIAL_ENABLED" FORCE)
SET(MASTER_BUFFER_SIZE                          "${MASTER_BUFFER_SIZE}"             CACHE STRING "MASTER_BUFFER_SIZE" FORCE)
SET(DEFAULT_ON                                  "${DEFAULT_ON}"                     CACHE STRING "DEFAULT_ON" FORCE)
SET(RSSI_ENABLE                                 "${RSSI_ENABLE}"                    CACHE STRING "RSSI_ENABLE" FORCE)
SET(TX_POWER                                    "${TX_POWER}"                       CACHE STRING "TX_POWER" FORCE)
SET(ADVERTISEMENT_INTERVAL                      "${ADVERTISEMENT_INTERVAL}"         CACHE STRING "ADVERTISEMENT_INTERVAL" FORCE)
SET(LOW_POWER_MODE                              "${LOW_POWER_MODE}"                 CACHE STRING "LOW_POWER_MODE" FORCE)
SET(PWM_ENABLE                                  "${PWM_ENABLE}"                     CACHE STRING "PWM_ENABLE" FORCE)
SET(FIRMWARE_VERSION                            "${FIRMWARE_VERSION}"               CACHE STRING "FIRMWARE_VERSION" FORCE)
SET(BOOT_DELAY                                  "${BOOT_DELAY}"                     CACHE STRING "BOOT_DELAY" FORCE)
SET(SCAN_DURATION                               "${SCAN_DURATION}"                  CACHE STRING "SCAN_DURATION" FORCE)
SET(SCAN_BREAK_DURATION                         "${SCAN_BREAK_DURATION}"            CACHE STRING "SCAN_BREAK_DURATION" FORCE)
SET(MAX_CHIP_TEMP                               "${MAX_CHIP_TEMP}"                  CACHE STRING "MAX_CHIP_TEMP" FORCE)
SET(INTERVAL_SCANNER_ENABLED                    "${INTERVAL_SCANNER_ENABLED}"       CACHE STRING "INTERVAL_SCANNER_ENABLED" FORCE)
SET(DEFAULT_OPERATION_MODE                      "${DEFAULT_OPERATION_MODE}"         CACHE STRING "DEFAULT_OPERATION_MODE" FORCE)
SET(PERSISTENT_FLAGS_DISABLED                   "${PERSISTENT_FLAGS_DISABLED}"      CACHE STRING "PERSISTENT_FLAGS_DISABLED" FORCE)
SET(MESH_ACCESS_ADDR                            "${MESH_ACCESS_ADDR}"               CACHE STRING "MESH_ACCESS_ADDR" FORCE)
SET(MESH_INTERVAL_MIN_MS                        "${MESH_INTERVAL_MIN_MS}"           CACHE STRING "MESH_INTERVAL_MIN_MS" FORCE)
SET(MESH_BOOT_TIME                              "${MESH_BOOT_TIME}"                 CACHE STRING "MESH_BOOT_TIME" FORCE)
SET(NRF_SERIES                                  "${NRF_SERIES}"                     CACHE STRING "NRF_SERIES" FORCE)
SET(RAM_R1_BASE                                 "${RAM_R1_BASE}"                    CACHE STRING "RAM_R1_BASE" FORCE)
SET(MAX_NUM_VS_SERVICES                         "${MAX_NUM_VS_SERVICES}"            CACHE STRING "MAX_NUM_VS_SERVICES" FORCE)
SET(ADVERTISEMENT_IMPROVEMENT                   "${ADVERTISEMENT_IMPROVEMENT}"      CACHE STRING "ADVERTISEMENT_IMPROVEMENT" FORCE)
IF(DEFINED RELAY_DEFAULT_ON)
SET(RELAY_DEFAULT_ON                            "${RELAY_DEFAULT_ON}"               CACHE STRING "RELAY_DEFAULT_ON" FORCE)
ENDIF()
SET(CONNECTION_ALIVE_TIMEOUT                    "${CONNECTION_ALIVE_TIMEOUT}"       CACHE STRING "CONNECTION_ALIVE_TIMEOUT" FORCE)

# Set Attribute table size
SET(ATTR_TABLE_SIZE                             "${ATTR_TABLE_SIZE}"                CACHE STRING "ATTR_TABLE_SIZE" FORCE)

# Add encryption
SET(ENCRYPTION                                  "${ENCRYPTION}"                     CACHE STRING "ENCRYPTION" FORCE)

SET(MESHING                                     "${MESHING}"                        CACHE STRING "MESHING" FORCE)
SET(BUILD_MESHING                               "${BUILD_MESHING}"                  CACHE STRING "BUILD_MESHING" FORCE)

# Add iBeacon default values
SET(IBEACON                                     "${IBEACON}"                        CACHE STRING "IBEACON" FORCE)
SET(BEACON_UUID                                 "${BEACON_UUID}"                    CACHE STRING "BEACON_UUID" FORCE)
SET(BEACON_MAJOR                                "${BEACON_MAJOR}"                   CACHE STRING "BEACON_MAJOR" FORCE)
SET(BEACON_MINOR                                "${BEACON_MINOR}"                   CACHE STRING "BEACON_MINOR" FORCE)
SET(BEACON_RSSI                                 "${BEACON_RSSI}"                    CACHE STRING "BEACON_RSSI" FORCE)
SET(EDDYSTONE                                   "${EDDYSTONE}"                      CACHE STRING "EDDYSTONE" FORCE)

# Add services
SET(CROWNSTONE_SERVICE                          "${CROWNSTONE_SERVICE}"             CACHE STRING "CROWNSTONE_SERVICE" FORCE)
SET(INDOOR_SERVICE                              "${INDOOR_SERVICE}"                 CACHE STRING "INDOOR_SERVICE" FORCE)
SET(GENERAL_SERVICE                             "${GENERAL_SERVICE}"                CACHE STRING "GENERAL_SERVICE" FORCE)
SET(POWER_SERVICE                               "${POWER_SERVICE}"                  CACHE STRING "POWER_SERVICE" FORCE)
SET(SCHEDULE_SERVICE                            "${SCHEDULE_SERVICE}"               CACHE STRING "SCHEDULE_SERVICE" FORCE)

# Add characteristics
SET(CHAR_CONTROL                                "${CHAR_CONTROL}"                   CACHE STRING "CHAR_CONTROL" FORCE)
SET(CHAR_MESHING                                "${CHAR_MESHING}"                   CACHE STRING "CHAR_MESHING" FORCE)
SET(CHAR_TEMPERATURE                            "${CHAR_TEMPERATURE}"               CACHE STRING "CHAR_TEMPERATURE" FORCE)
SET(CHAR_RESET                                  "${CHAR_RESET}"                     CACHE STRING "CHAR_RESET" FORCE)
SET(CHAR_CONFIGURATION                          "${CHAR_CONFIGURATION}"             CACHE STRING "CHAR_CONFIGURATION" FORCE)
SET(CHAR_STATE                                  "${CHAR_STATE}"                     CACHE STRING "CHAR_STATE" FORCE)
SET(CHAR_PWM                                    "${CHAR_PWM}"                       CACHE STRING "CHAR_PWM" FORCE)
SET(CHAR_SAMPLE_CURRENT                         "${CHAR_SAMPLE_CURRENT}"            CACHE STRING "CHAR_SAMPLE_CURRENT" FORCE)
SET(CHAR_CURRENT_LIMIT                          "${CHAR_CURRENT_LIMIT}"             CACHE STRING "CHAR_CURRENT_LIMIT" FORCE)
SET(CHAR_RSSI                                   "${CHAR_RSSI}"                      CACHE STRING "CHAR_RSSI" FORCE)
SET(CHAR_SCAN_DEVICES                           "${CHAR_SCAN_DEVICES}"              CACHE STRING "CHAR_SCAN_DEVICES" FORCE)
SET(CHAR_TRACK_DEVICES                          "${CHAR_TRACK_DEVICES}"             CACHE STRING "CHAR_TRACK_DEVICES" FORCE)
SET(CHAR_RELAY                                  "${CHAR_RELAY}"                     CACHE STRING "CHAR_RELAY" FORCE)
SET(CHAR_SCHEDULE                               "${CHAR_SCHEDULE}"                  CACHE STRING "CHAR_SCHEDULE" FORCE)

