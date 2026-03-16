/*
 * Copyright (c) 2026 Benedikt Spranger
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/eeprom.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(type_plate, CONFIG_APP_LOG_LEVEL);

static const struct device *eeprom = DEVICE_DT_GET(DT_NODELABEL(eeprom));

int get_type_plate(void)
{
	size_t size;
	int ret;

	if (!device_is_ready(eeprom)) {
		LOG_ERR("eeprom device not ready\n");
		return -ENODEV;
	}

	size = eeprom_get_size(eeprom);
	LOG_INF("EEPROM size: %zu", size);

	return 0;
}
