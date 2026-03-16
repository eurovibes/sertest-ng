/*
 * Copyright (c) 2026 Benedikt Spranger
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/drivers/rtc.h>

#ifndef SERTEST_NG_H
#define SERTEST_NG_H

int get_type_plate(void);

/* RTC */
int rtc_init(void);
int get_date_time(struct rtc_time *tm);
int set_date_time(struct rtc_time *tm);

/* USB */
int usb_init(void);
int usb_reset(void);

#endif /* SERTEST_NG_H */
