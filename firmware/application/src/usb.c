/*
 * Copyright (c) 2026 Benedikt Spranger
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart/uart_bridge.h>
#include <zephyr/logging/log.h>
#include <zephyr/usb/bos.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/usb/usbd.h>

#include "sertest-ng.h"

#define USBD_VID	0xFFFF
#define USBD_PID	0xFFFF

LOG_MODULE_REGISTER(usb, CONFIG_APP_LOG_LEVEL);

#define DEVICE_DT_GET_COMMA(node_id) DEVICE_DT_GET(node_id),

static const char *const blocklist[] = {
	"dfu_dfu",
	NULL,
};

const struct device *uart_bridges[] = {
	DT_FOREACH_STATUS_OKAY(zephyr_uart_bridge, DEVICE_DT_GET_COMMA)
};

USBD_DEVICE_DEFINE(usbd_ctx, DEVICE_DT_GET(DT_NODELABEL(zephyr_udc0)),
		   USBD_VID, USBD_PID);

USBD_DESC_LANG_DEFINE(lang);
USBD_DESC_MANUFACTURER_DEFINE(mfr, "Eurovibes");
USBD_DESC_PRODUCT_DEFINE(product, "Sertest-NG");
USBD_DESC_SERIAL_NUMBER_DEFINE(sn);
USBD_DESC_CONFIG_DEFINE(fs_cfg_desc, "FS Configuration");

static const uint8_t attributes = 0; // USB_SCD_REMOTE_WAKEUP;
static struct usbd_context *usbd;

USBD_CONFIGURATION_DEFINE(fs_config, attributes, 50, &fs_cfg_desc);

static const struct usb_bos_capability_lpm bos_cap_lpm = {
	.bLength = sizeof(struct usb_bos_capability_lpm),
	.bDescriptorType = USB_DESC_DEVICE_CAPABILITY,
	.bDevCapabilityType = USB_BOS_CAPABILITY_EXTENSION,
	.bmAttributes = 0UL,
};
USBD_DESC_BOS_DEFINE(usbext, sizeof(bos_cap_lpm), &bos_cap_lpm);

struct usbd_context *usbd_setup_device(usbd_msg_cb_t msg_cb)
{
	int err;

	err = usbd_add_descriptor(&usbd_ctx, &lang);
	if (err) {
		LOG_ERR("Failed to initialize language descriptor (%d)", err);
		return NULL;
	}

	err = usbd_add_descriptor(&usbd_ctx, &mfr);
	if (err) {
		LOG_ERR("Failed to initialize manufacturer descriptor (%d)", err);
		return NULL;
		}

	err = usbd_add_descriptor(&usbd_ctx, &product);
	if (err) {
		LOG_ERR("Failed to initialize product descriptor (%d)", err);
		return NULL;
	}

	err = usbd_add_descriptor(&usbd_ctx, &sn);
	if (err) {
		LOG_ERR("Failed to initialize SN descriptor (%d)", err);
		return NULL;
	}

	err = usbd_add_configuration(&usbd_ctx, USBD_SPEED_FS,
				     &fs_config);
	if (err) {
		LOG_ERR("Failed to add Full-Speed configuration");
		return NULL;
	}

	err = usbd_register_all_classes(&usbd_ctx, USBD_SPEED_FS, 1, blocklist);
	if (err) {
		LOG_ERR("FS: Failed to add register classes");
		return NULL;
	}

	usbd_device_set_code_triple(&usbd_ctx, USBD_SPEED_FS,
				    USB_BCC_MISCELLANEOUS, 0x02, 0x01);
	usbd_self_powered(&usbd_ctx, attributes & USB_SCD_SELF_POWERED);

	if (msg_cb) {
		err = usbd_msg_register_cb(&usbd_ctx, msg_cb);
		if (err) {
			LOG_ERR("Failed to register message callback");
			return NULL;
		}
	}

	(void)usbd_device_set_bcd_usb(&usbd_ctx, USBD_SPEED_FS, 0x0201);

	err = usbd_add_descriptor(&usbd_ctx, &usbext);
	if (err) {
		LOG_ERR("Failed to add USB 2.0 Extension Descriptor");
		return NULL;
	}

	return &usbd_ctx;
}

struct usbd_context *usbd_init_device(usbd_msg_cb_t msg_cb)
{
	struct usbd_context *ctx;
	int err;

	ctx = usbd_setup_device(msg_cb);
	if (!ctx)
		return NULL;

	err = usbd_init(ctx);
	if (err) {
		LOG_ERR("Failed to initialize device support");
		return NULL;
	}

	return ctx;
}

static void bridge_msg_cb(struct usbd_context *const ctx,
			  const struct usbd_msg	*msg)
{
	LOG_INF("USBD message: %s", usbd_msg_type_string(msg->type));

	if (usbd_can_detect_vbus(ctx)) {
		if (msg->type == USBD_MSG_VBUS_READY) {
			if (usbd_enable(ctx)) {
				LOG_ERR("Failed to enable device support");
			}
		}

		if (msg->type == USBD_MSG_VBUS_REMOVED) {
			if (usbd_disable(ctx)) {
				LOG_ERR("Failed to disable device support");
			}
		}
	}

	if (msg->type == USBD_MSG_CDC_ACM_LINE_CODING ||
	    msg->type == USBD_MSG_CDC_ACM_CONTROL_LINE_STATE) {
		for (uint8_t i = 0; i < ARRAY_SIZE(uart_bridges); i++) {
			/* update all bridges, non valid combinations are
                         * skipped automatically.
                         */
			uart_bridge_settings_update(msg->dev, uart_bridges[i]);
		}
	}
}

int usb_init(void)
{
	int err;

	usbd = usbd_init_device(bridge_msg_cb);
	if (!usbd) {
		LOG_ERR("Failed to initialize USB device");
		return -ENODEV;
	}

	if (!usbd_can_detect_vbus(usbd)) {
		err = usbd_enable(usbd);
		if (err) {
			LOG_ERR("Failed to enable device support");
			return err;
		}
	}

	return err;
}

int usb_reset(void)
{
	return usbd_shutdown(&usbd_ctx);
}
