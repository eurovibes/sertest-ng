/*
 * Copyright (c) 2026 Benedikt Spranger
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart/uart_bridge.h>
#include <zephyr/logging/log.h>
#include <zephyr/usb/bos.h>
#include <zephyr/usb/msos_desc.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/usb/usbd.h>

#include <cannectivity/usb/class/gs_usb.h>

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

struct cannectivity_msosv2_descriptor {
	struct msosv2_descriptor_set_header header;
	struct msosv2_compatible_id gs_usb_compatible_id;
	struct msosv2_guids_property gs_usb_guids_property;
	struct msosv2_vendor_revision gs_usb_vendor_revision;
} __packed;

#define COMPATIBLE_ID_WINUSB 'W', 'I', 'N', 'U', 'S', 'B', 0x00, 0x00

#define CANNECTIVITY_GS_USB_DEVICE_INTERFACE_GUID \
	'{', 0x00, 'B', 0x00, '2', 0x00, '4', 0x00, 'D', 0x00, '8', 0x00, \
	'3', 0x00, '7', 0x00, '9', 0x00, '-', 0x00, '2', 0x00, '3', 0x00, \
	'5', 0x00, 'F', 0x00, '-', 0x00, '4', 0x00, '8', 0x00, '5', 0x00, \
	'3', 0x00, '-', 0x00, '9', 0x00, '5', 0x00, 'E', 0x00, '7', 0x00, \
	'-', 0x00, '7', 0x00, '7', 0x00, '7', 0x00, '2', 0x00, '5', 0x00, \
	'1', 0x00, '6', 0x00, 'F', 0x00, 'A', 0x00, '2', 0x00, 'D', 0x00, \
	'5', 0x00, '}', 0x00, 0x00, 0x00

static const struct cannectivity_msosv2_descriptor msosv2_descriptor = {
	.header = {
		.wLength = sizeof(struct msosv2_descriptor_set_header),
		.wDescriptorType = MS_OS_20_SET_HEADER_DESCRIPTOR,
		/* Windows version (8.1) (0x06030000) */
		.dwWindowsVersion = 0x06030000,
		.wTotalLength = sizeof(struct cannectivity_msosv2_descriptor),
	},
	.gs_usb_compatible_id = {
		.wLength = sizeof(struct msosv2_compatible_id),
		.wDescriptorType = MS_OS_20_FEATURE_COMPATIBLE_ID,
		.CompatibleID = {COMPATIBLE_ID_WINUSB},
	},
	.gs_usb_guids_property = {
		.wLength = sizeof(struct msosv2_guids_property),
		.wDescriptorType = MS_OS_20_FEATURE_REG_PROPERTY,
		.wPropertyDataType = MS_OS_20_PROPERTY_DATA_REG_MULTI_SZ,
		.wPropertyNameLength = 42,
		.PropertyName = {DEVICE_INTERFACE_GUIDS_PROPERTY_NAME},
		.wPropertyDataLength = 80,
		.bPropertyData = {CANNECTIVITY_GS_USB_DEVICE_INTERFACE_GUID},
	},
	.gs_usb_vendor_revision = {
		.wLength = sizeof(struct msosv2_vendor_revision),
		.wDescriptorType = MS_OS_20_FEATURE_VENDOR_REVISION,
		.VendorRevision = 1U,
	},
};

struct usb_bos_capability_msosv2 {
	struct usb_bos_platform_descriptor platform;
	struct usb_bos_capability_msos cap;
} __packed;


static struct usb_bos_capability_msosv2 bos_cap_msosv2 = {
	.platform = {
		.bLength = sizeof(struct usb_bos_capability_msosv2),
		.bDescriptorType = USB_DESC_DEVICE_CAPABILITY,
		.bDevCapabilityType = USB_BOS_CAPABILITY_PLATFORM,
		.bReserved = 0,
		.PlatformCapabilityUUID = {
			/* MS OS 2.0 Platform Capability ID: D8DD60DF-4589-4CC7-9CD2-659D9E648A9F */
			0xDF, 0x60, 0xDD, 0xD8,
			0x89, 0x45,
			0xC7, 0x4C,
			0x9C, 0xD2,
			0x65, 0x9D, 0x9E, 0x64, 0x8A, 0x9F,
		},
	},
	.cap = {
		/* Windows version (8.1) (0x06030000) */
		.dwWindowsVersion = sys_cpu_to_le32(0x06030000),
		.wMSOSDescriptorSetTotalLength =
			sys_cpu_to_le16(sizeof(struct cannectivity_msosv2_descriptor)),
		.bMS_VendorCode = GS_USB_MS_VENDORCODE,
		.bAltEnumCode = 0x00
	},
};

static int vendorcode_handler(const struct usbd_context *const ctx,
			      const struct usb_setup_packet *const setup,
			      struct net_buf *const buf)
{
	size_t len = bos_cap_msosv2.cap.wMSOSDescriptorSetTotalLength;

	if (setup->bRequest == GS_USB_MS_VENDORCODE &&
	    setup->wIndex == MS_OS_20_DESCRIPTOR_INDEX) {
		net_buf_add_mem(buf, &msosv2_descriptor,
				MIN(net_buf_tailroom(buf), len));

		return 0;
	}

	return -ENOTSUP;
}

USBD_DESC_BOS_VREQ_DEFINE(msosv2, sizeof(bos_cap_msosv2), &bos_cap_msosv2,
			  GS_USB_MS_VENDORCODE, vendorcode_handler, NULL);

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

	err = usbd_device_set_bcd_usb(&usbd_ctx, USBD_SPEED_FS, USB_SRN_2_0_1);
	if (err) {
		LOG_ERR("failed to set full-speed bcdUSB (err %d)", err);
		return NULL;
	}

	err = usbd_add_descriptor(&usbd_ctx, &usbext);
	if (err) {
		LOG_ERR("Failed to add USB 2.0 Extension Descriptor");
		return NULL;
	}

	err = usbd_add_descriptor(&usbd_ctx, &msosv2);
	if (err) {
		LOG_ERR("failed to add Microsoft OS 2.0 descriptor (err %d)", err);
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
