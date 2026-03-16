/*
 * Copyright (c) 2026 Benedikt Spranger
 * SPDX-License-Identifier: Apache-2.0
 */
#include <cryptoauthlib.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart/uart_bridge.h>
#include <zephyr/logging/log.h>
#include <zephyr/logging/log_ctrl.h>
#include <zephyr/sys/util.h>

#include <app_version.h>

#include "sertest-ng.h"

LOG_MODULE_REGISTER(main, CONFIG_APP_LOG_LEVEL);

int main(void)
{
	struct rtc_time tm = {
		.tm_year = 2026 - 1900,
		.tm_mon = 3 - 1,
		.tm_mday = 17,
		.tm_hour = 2,
		.tm_min = 40,
		.tm_sec = 0,
	};

	uint8_t random_number[32];
	ATCA_STATUS status;
	int ret;

	printk("Eurovibes Sertest-NG Application %s\n", APP_VERSION_STRING);
	LOG_INIT();
	log_thread_set(k_current_get());

	if (get_type_plate()) {
		LOG_ERR("EEPROM device not ready");
		return -ENODEV;
	}

	cfg_ateccx08a_i2c_default.cfg_data = DEVICE_DT_NAME(DT_NODELABEL(i2c2));
	status = atcab_init(&cfg_ateccx08a_i2c_default);
	if (status != ATCA_SUCCESS)
	{
		LOG_ERR("atcab_init() failed: %02x", status);
		return -ENODEV;
	}

	atcab_random(&random_number);
	LOG_INF("Random: 0x%08x", *((uint32_t *) random_number));

	if (rtc_init()) {
		LOG_ERR("RTC device is not ready");
		return -ENODEV;
	}

	LOG_INF("RTC device support enabled");

	if (usb_init()) {
		LOG_ERR("Failed to initialize USB device");
		return -ENODEV;
	}

	LOG_INF("USB device support enabled");

	set_date_time(&tm);

	while (1) {
		ret = get_date_time(&tm);
		if (ret)
			LOG_ERR("get_date_time() failed: %d", ret);
		k_sleep(K_MSEC(1000));
	}

	return 0;
}
