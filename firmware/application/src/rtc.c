/*
 * Copyright (c) 2026 Benedikt Spranger
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/rtc.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(rtc, CONFIG_APP_LOG_LEVEL);

static const struct device *const rtc = DEVICE_DT_GET(DT_ALIAS(rtc));

int set_date_time(struct rtc_time *tm)
{
	int ret;

	ret = rtc_set_time(rtc, tm);
	if (ret < 0) {
		LOG_ERR("Cannot write date time: %d\n", ret);
		return ret;
	}
	return ret;
}

int get_date_time(struct rtc_time *tm)
{
	int ret = 0;

	if (!tm)
		return -EINVAL;

	ret = rtc_get_time(rtc, tm);
	if (ret < 0) {
		LOG_ERR("Cannot read date time: %d\n", ret);
		return ret;
	}

	LOG_DBG("RTC date and time: %04d-%02d-%02d %02d:%02d:%02d\n",
		tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
		tm->tm_hour, tm->tm_min, tm->tm_sec);

	return 0;
}

int rtc_init(void)
{
	if (!device_is_ready(rtc))
		return -ENODEV;

	return 0;
}
