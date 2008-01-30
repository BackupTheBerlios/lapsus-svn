/*
 *  Asus OLED USB driver
 *
 *  Copyright (C) 2007 Jakub Schmidtke (sjakub@gmail.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 *
 *
 *  This module is based on usbled and asus-laptop modules.
 *
 *
 *  Asus OLED support is based on asusoled program taken from
 *  https://launchpad.net/asusoled/.
 *
 *
 */

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/usb.h>
#include <linux/platform_device.h>
#include <linux/ctype.h>

#define ASUS_OLED_VERSION		"0.02"
#define ASUS_OLED_NAME			"asus-oled"

#define ASUS_OLED_ERROR			"Asus OLED Display Error: "

#define ASUS_OLED_STATIC		's'
#define ASUS_OLED_ROLL			'r'
#define ASUS_OLED_FLASH			'f'

#define ASUS_OLED_MAX_WIDTH		1792
#define ASUS_OLED_DISP_WIDTH		128
#define ASUS_OLED_DISP_HEIGHT		32
#define ASUS_OLED_PACKET_BUF_SIZE	256

MODULE_AUTHOR("Jakub Schmidtke, sjakub@gmail.com");
MODULE_DESCRIPTION("Asus OLED Driver v" ASUS_OLED_VERSION);
MODULE_LICENSE("GPL");

static uint start_off = 0;
module_param(start_off, uint, 0644);
MODULE_PARM_DESC(start_off, "Set to 1 to switch off OLED display after it is attached");

/* table of devices that work with this driver */
static struct usb_device_id id_table [] = {
	{ USB_DEVICE(0x0b05, 0x1726) },
	{ },
};

MODULE_DEVICE_TABLE (usb, id_table);

#define SETUP_PACKET_HEADER(packet, val1, val2, val3, val4, val5, val6, val7) \
	do {					\
		memset(packet, 0, sizeof(struct asus_oled_header));		\
		packet->header.magic1 = 0x55;		\
		packet->header.magic2 = 0xaa;		\
		packet->header.flags = val1;		\
		packet->header.value3 = val2;		\
		packet->header.buffer1 = val3;		\
		packet->header.buffer2 = val4;		\
		packet->header.value6 = val5;		\
		packet->header.value7 = val6;		\
		packet->header.value8 = val7;		\
	} while(0);

struct asus_oled_header {
	uint8_t		magic1;
	uint8_t		magic2;
	uint8_t		flags;
	uint8_t		value3;
	uint8_t		buffer1;
	uint8_t		buffer2;
	uint8_t		value6;
	uint8_t		value7;
	uint8_t		value8;
	uint8_t		padding2[7];
} __attribute((packed));

struct asus_oled_packet {
	struct asus_oled_header		header;
	uint8_t				bitmap[ASUS_OLED_PACKET_BUF_SIZE];
} __attribute((packed));

struct asus_oled_dev {
	struct usb_device *	udev;
	uint8_t			pic_mode;
	size_t			height;
	size_t			width;
	size_t			x_shift;
	size_t			y_shift;
	size_t			buf_offs;
	uint8_t			last_val;
	size_t			buf_size;
	char			*buf;
};

static void enable_oled(struct asus_oled_dev *odev, uint8_t enabl)
{
	int a;
	int retval;
	int act_len;
	struct asus_oled_packet * packet;

	packet = kzalloc(sizeof(struct asus_oled_packet), GFP_KERNEL);
	
	if (!packet) {
		dev_err(&odev->udev->dev, "out of memory\n");
		return;
	}
	
	SETUP_PACKET_HEADER(packet, 0x20, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00);
	
	if (enabl) packet->bitmap[0] = 0xaf;
	else packet->bitmap[0] = 0xae;
	
	for (a=0; a<1; a++) {
		retval = usb_bulk_msg(odev->udev,
			usb_sndbulkpipe(odev->udev, 2),
			packet,
			sizeof(struct asus_oled_header) + 1,
			&act_len,
			-1);
	
		if (retval)
			dev_dbg(&odev->udev->dev, "retval = %d\n", retval);
	}
	
	kfree(packet);
}

static ssize_t set_enabled(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct usb_interface *intf = to_usb_interface(dev);
	struct asus_oled_dev *odev = usb_get_intfdata(intf);
	int temp = simple_strtoul(buf, NULL, 10);

	enable_oled(odev, temp);
	
	return count;
}

static void send_packets(struct usb_device *udev, struct asus_oled_packet *packet,
	char *buf, uint8_t p_type, size_t p_num)
{
	size_t i;
	int act_len;
	
	for (i = 0; i < p_num; i++) {
		int retval;
		
		switch (p_type) {
			case ASUS_OLED_ROLL:
				SETUP_PACKET_HEADER(packet, 0x40, 0x80, p_num, i + 1, 0x00, 0x01, 0xff);
			break;
			case ASUS_OLED_STATIC:
				SETUP_PACKET_HEADER(packet, 0x10 + i, 0x80, 0x01, 0x01, 0x00, 0x01, 0x00);
			break;
			case ASUS_OLED_FLASH:
				SETUP_PACKET_HEADER(packet, 0x10 + i, 0x80, 0x01, 0x01, 0x00, 0x00, 0xff);
			break;
		}
		
		memcpy(packet->bitmap, buf + (ASUS_OLED_PACKET_BUF_SIZE*i), ASUS_OLED_PACKET_BUF_SIZE);
		
		retval = usb_bulk_msg(udev,
			usb_sndctrlpipe(udev, 2),
			packet,
			sizeof(struct asus_oled_packet),
			&act_len,
			-1);
		
		if (retval)
			dev_dbg(&udev->dev, "retval = %d\n", retval);
	}
}

static void send_data(struct asus_oled_dev *odev)
{
	size_t packet_num = odev->buf_size / ASUS_OLED_PACKET_BUF_SIZE;
	struct asus_oled_packet * packet;
	
	packet = kzalloc(sizeof(struct asus_oled_packet), GFP_KERNEL);
	
	if (!packet) {
		dev_err(&odev->udev->dev, "out of memory\n");
		return;
	}
	
	// When sending roll-mode data the display updated only first packet.
	// I have no idea why, but when static picture is send just before
	// rolling picture - everything works fine.
	if (odev->pic_mode == ASUS_OLED_ROLL)
		send_packets(odev->udev, packet, odev->buf, ASUS_OLED_STATIC, 2);
	
	// Only ROLL mode can use more than 2 packets.
	if (odev->pic_mode != ASUS_OLED_ROLL && packet_num > 2)
		packet_num = 2;
	
	send_packets(odev->udev, packet, odev->buf, odev->pic_mode, packet_num);
	
	kfree(packet);
}

static int append_values(struct asus_oled_dev *odev, uint8_t val, size_t count)
{
	while (count-- > 0) {
		if (val) {
			size_t x = odev->buf_offs % odev->width;
			size_t y = odev->buf_offs / odev->width;
			size_t i;
			
			x += odev->x_shift;
			y += odev->y_shift;
			
			i = (x/128)*640 + 127 - x + (y/8)*128;
			
			if (i >= odev->buf_size) {
				printk(ASUS_OLED_ERROR "Buffer overflow! Report a bug in the driver: offs: %d >= %d i: %d (x: %d y: %d)\n",
					(int) odev->buf_offs, (int) odev->buf_size, (int) i, (int) x, (int) y);
				return -EIO;
			}
			else odev->buf[i] &= ~(1<<(y%8));
		}
		
		odev->last_val = val;
		odev->buf_offs++;
	}
	
	return 0;
}

static ssize_t set_picture(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	size_t offs = 0, max_offs;
	struct usb_interface *intf = to_usb_interface(dev);
	struct asus_oled_dev *odev = usb_get_intfdata(intf);
	
	if (count < 1) return 0;
	
	if (buf[0] == '<') {
		size_t i;
		size_t w = 0, h = 0;
		size_t w_mem, h_mem;
		
		if (count < 10 || buf[2] != ':') {
			goto error_header;
		}
		
		switch(tolower(buf[1])) {
			case ASUS_OLED_STATIC:
			case ASUS_OLED_ROLL:
			case ASUS_OLED_FLASH:
				odev->pic_mode = buf[1];
			break;
			default:
				printk(ASUS_OLED_ERROR "Wrong picture mode: '%c'.\n", buf[1]);
				return -EIO;
			break;
		}
		
		for (i = 3; i < count; ++i) {
			if (buf[i] >= '0' && buf[i] <= '9') {
				w = 10*w + (buf[i] - '0');
				
				if (w > ASUS_OLED_MAX_WIDTH) goto error_width;
			}
			else if (tolower(buf[i]) == 'x') break;
			else goto error_width;
		}
		
		for (++i; i < count; ++i) {
			if (buf[i] >= '0' && buf[i] <= '9') {
				h = 10*h + (buf[i] - '0');
				
				if (h > ASUS_OLED_DISP_HEIGHT) goto error_height;
			}
			else if (tolower(buf[i]) == '>') break;
			else goto error_height;
		}
		
		if (w < 1 || w > ASUS_OLED_MAX_WIDTH) goto error_width;
		
		if (h < 1 || h > ASUS_OLED_DISP_HEIGHT) goto error_height;
		
		if (i >= count || buf[i] != '>') goto error_header;
		
		offs = i+1;
		
		if (w % ASUS_OLED_DISP_WIDTH != 0)
			w_mem = (w/ASUS_OLED_DISP_WIDTH + 1)*ASUS_OLED_DISP_WIDTH;
		else
			w_mem = w;
		
		if (h < ASUS_OLED_DISP_HEIGHT)
			h_mem = ASUS_OLED_DISP_HEIGHT;
		else
			h_mem = h;
		
		odev->buf_size = w_mem * h_mem / 8;
		
		if (odev->buf) kfree(odev->buf);
		odev->buf = kmalloc(odev->buf_size, GFP_KERNEL);
		
		if (odev->buf == NULL) {
			odev->buf_size = 0;
			printk(ASUS_OLED_ERROR "Out of memory!\n");
			return -ENOMEM;
		}
		
		memset(odev->buf, 0xff, odev->buf_size);
		
		odev->buf_offs = 0;
		odev->width = w;
		odev->height = h;
		odev->x_shift = 0;
		odev->y_shift = 0;
		odev->last_val = 0;
		
		if (odev->pic_mode == ASUS_OLED_FLASH) {
			if (h < ASUS_OLED_DISP_HEIGHT/2)
					odev->y_shift = (ASUS_OLED_DISP_HEIGHT/2 - h)/2;
		}
		else {
			if (h < ASUS_OLED_DISP_HEIGHT)
				odev->y_shift = (ASUS_OLED_DISP_HEIGHT - h)/2;
		}
		
		if (w < ASUS_OLED_DISP_WIDTH)
			odev->x_shift = (ASUS_OLED_DISP_WIDTH - w)/2;
	}
	
	max_offs = odev->width * odev->height;
	
	while (offs < count && odev->buf_offs < max_offs) {
		int ret;
		
		if (buf[offs] == '1' || buf[offs] == '#') {
			if ( (ret = append_values(odev, 1, 1)) < 0) return ret;
		}
		else if (buf[offs] == '0' || buf[offs] == ' ') {
			if ( (ret = append_values(odev, 0, 1)) < 0) return ret;
		}
		else if (buf[offs] == '\n') {
			// New line detected. Lets assume, that all characters till the end of the
			// line were equal to the last character in this line.
			if (odev->buf_offs % odev->width != 0)
				if ( (ret = append_values(odev, odev->last_val,
						odev->width - (odev->buf_offs % odev->width))) < 0) return ret;
		}
		
		offs++;
	}
	
	if (odev->buf_offs >= max_offs) send_data(odev);
	
	return count;

error_width:
	printk(ASUS_OLED_ERROR "Wrong picture width specified.\n");
	return -EIO;
	
error_height:
	printk(ASUS_OLED_ERROR "Wrong picture height specified.\n");
	return -EIO;

error_header:
	printk(ASUS_OLED_ERROR "Wrong picture header.\n");
	return -EIO;
}

#define ASUS_OLED_CREATE_DEVICE_ATTR(_file)	\
	static DEVICE_ATTR(asus_oled_##_file, S_IWUGO, NULL, set_##_file);

#define ASUS_OLED_DEVICE_ATTR(_file)		\
	dev_attr_asus_oled_##_file

ASUS_OLED_CREATE_DEVICE_ATTR(enabled)
ASUS_OLED_CREATE_DEVICE_ATTR(picture)

static int asus_oled_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
	struct usb_device *udev = interface_to_usbdev(interface);
	struct asus_oled_dev *odev = NULL;
	int retval = -ENOMEM;

	odev = kzalloc(sizeof(struct asus_oled_dev), GFP_KERNEL);
	
	if (odev == NULL) {
		dev_err(&interface->dev, "Out of memory\n");
		goto error_mem;
	}

	odev->udev = usb_get_dev(udev);
	odev->pic_mode = ASUS_OLED_STATIC;
	odev->height = 0;
	odev->width = 0;
	odev->x_shift = 0;
	odev->y_shift = 0;
	odev->buf_offs = 0;
	odev->buf_size = 0;
	odev->last_val = 0;
	odev->buf = NULL;
	
	usb_set_intfdata (interface, odev);

	if ((retval = device_create_file(&interface->dev, &ASUS_OLED_DEVICE_ATTR(enabled)))) {
		device_remove_file(&interface->dev, &ASUS_OLED_DEVICE_ATTR(enabled));
		goto error;
	}

	if ((retval = device_create_file(&interface->dev, &ASUS_OLED_DEVICE_ATTR(picture)))) {
		device_remove_file(&interface->dev, &ASUS_OLED_DEVICE_ATTR(picture));
		goto error;
	}

	dev_info(&interface->dev, "Attached Asus OLED device\n");
	
	if (start_off)
		enable_oled(odev, 0);
	
	return 0;

error:
	usb_set_intfdata (interface, NULL);
	usb_put_dev(odev->udev);
	kfree(odev);
	
error_mem:
	return retval;
}

static void asus_oled_disconnect(struct usb_interface *interface)
{
	struct asus_oled_dev *odev;

	odev = usb_get_intfdata (interface);
	usb_set_intfdata (interface, NULL);

	device_remove_file(&interface->dev, & ASUS_OLED_DEVICE_ATTR(picture));
	device_remove_file(&interface->dev, & ASUS_OLED_DEVICE_ATTR(enabled));

	usb_put_dev(odev->udev);

	if (odev->buf) kfree(odev->buf);
	
	kfree(odev);

	dev_info(&interface->dev, "Disconnected Asus OLED device\n");
}

static struct usb_driver oled_driver = {
	.name =		ASUS_OLED_NAME,
	.probe =	asus_oled_probe,
	.disconnect =	asus_oled_disconnect,
	.id_table =	id_table,
};

static int __init asus_oled_init(void)
{
	int retval = 0;

	retval = usb_register(&oled_driver);
	
	if (retval) err("usb_register failed. Error number %d", retval);
	
	return retval;
}

static void __exit asus_oled_exit(void)
{
	usb_deregister(&oled_driver);
}

module_init (asus_oled_init);
module_exit (asus_oled_exit);

