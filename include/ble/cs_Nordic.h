/**
 * Author: Crownstone Team
 * Copyright: Crownstone (https://crownstone.rocks)
 * Date: Apr. 21, 2015
 * License: LGPLv3+, Apache License 2.0, and/or MIT (triple-licensed)
 */
#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <app_util_platform.h>
typedef uint32_t ret_code_t;
#include <nrfx_comp.h>
//#include <nrf_drv_comp.h>


//! The header files in the Nordic SDK. Keep all the includes here, not dispersed over all the header files.
#include <app_scheduler.h>
#include <app_util.h>
#include <ble.h>
#include <ble_advdata.h>
#include <ble_gatt.h>
#include <ble_gatts.h>
#include <ble_hci.h>
#include <ble_srv_common.h>
#include <fds.h>
#include <nrf.h>
#include <nrf_delay.h>
#include <nrf_error.h>
#include <nrf_fstorage.h>
#include <nrf_gpio.h>
#include <nrf_gpiote.h>
#include <nrf_log_ctrl.h>
#include <nrf_log_default_backends.h>
#include <nrf_nvic.h>
#include <nrf_sdh.h>
#include <nrf_sdh_ble.h>
#include <nrf_sdh_soc.h>
#include <nrf_sdm.h>
// legacy #include <nrf_drv_uart.h>
#include <nrfx_glue.h>

#ifndef BOOTLOADER_COMPILATION
#include <crc16.h>
#include <nrf_gpiote.h>
#include <nrf_ppi.h>
#include <nrf_saadc.h>
#include <nrf_timer.h>
#include <nrf_uart.h>
#endif

// undefine the following macros, we will use our macros as defined in <util/cs_BleError.h>
#ifdef APP_ERROR_CHECK
#undef APP_ERROR_CHECK
#endif

#ifdef APP_ERROR_HANDLER
#undef APP_ERROR_HANDLER
#endif

#ifdef __cplusplus
}
#endif
