#include "libusb-1.0/libusb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

int Initialize_Device(libusb_context ** ctx, libusb_device_handle ** usb_device)
{
	int ret = 0;

	*usb_device = libusb_open_device_with_vid_pid(*ctx, 0x0a5c, 0x2763);
	if (*usb_device == NULL)
	{
		return -1;
	}

	ret = libusb_claim_interface(*usb_device, 0);
	if (ret)
	{
		printf("Failed to claim interface\n");
		return ret;
	}

	return ret;
}

int ep_write(unsigned char *buf, int len, libusb_device_handle * usb_device)
{
	int ret =
	    libusb_control_transfer(usb_device, LIBUSB_REQUEST_TYPE_VENDOR, 0,
				    len & 0xffff, len >> 16, NULL, 0, 1000);
	int a_len;

	ret = libusb_bulk_transfer(usb_device, 0x01, buf, len, &a_len, 100000);
	printf("libusb_bulk_transfer returned %d\n", ret);

	return a_len;
}

int ep_read(unsigned char *buf, int len, libusb_device_handle * usb_device)
{
	int ret =
	    libusb_control_transfer(usb_device,
				    LIBUSB_REQUEST_TYPE_VENDOR |
				    LIBUSB_ENDPOINT_IN, 0, len & 0xffff,
				    len >> 16, buf, len, 1000);

	return len;
}

int main(int argc, char *argv[])
{
	int result;
	libusb_context *ctx;
	libusb_device_handle *usb_device;
	unsigned char *txbuf;
	int size;
	int retcode;
	FILE *fp1, *fp2;
	char def1[] = "/usr/share/rpiboot/usbbootcode.bin";
	char def2[] = "/usr/share/rpiboot/msd.elf";

	char *stage1, *stage2;
	char *dir;

	struct MESSAGE_S {
		int length;
		unsigned char signature[20];
	} message;

	if(argc > 2)
	{
		stage1 = argv[1];
		stage2 = argv[2];
	}
	else
	{
		stage1 = def1;
		stage2 = def2;
	}

	fp1 = fopen(stage1, "rb");
	if (fp1 == NULL)
	{
		printf("Cannot open file %s\n", stage1);
		exit(-1);
	}

	fp2 = fopen(stage2, "rb");
	if (fp2 == NULL)
	{
		printf("Cannot open file %s\n", stage2);
		exit(-1);
	}
	if(strcmp(stage2 + strlen(stage2) - 4, ".elf"))
	{
		printf("Third stage needs to be .elf format\n");
		exit(-1);
	}

	int ret = libusb_init(&ctx);
	if (ret)
	{
		printf("Failed to initialise libUSB\n");
		exit(-1);
	}

	libusb_set_debug(ctx, 0);

	while (1)
	{
		FILE *fp, *fp_img = NULL;
		struct libusb_device_descriptor desc;

		printf("Waiting for BCM2835 ...\n");

		// Wait for a device to get plugged in
		do
		{
			result = Initialize_Device(&ctx, &usb_device);
			if (result)
			{
				sleep(1);
			}

		}
		while (result);

		ret =
		    libusb_get_device_descriptor(libusb_get_device
							 (usb_device), &desc);
		printf("Found serial = %d: writing file %s\n",
		       desc.iSerialNumber,
		       desc.iSerialNumber == 0 ? stage1 : stage2);
		fp = desc.iSerialNumber == 0 ? fp1 : fp2;

		fseek(fp, 0, SEEK_END);
		message.length = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		printf("Writing %d bytes of program data\n", message.length);

		if(desc.iSerialNumber == 1 && argc == 4)
		{
			// Been given a filesystem image
			fp_img = fopen(argv[3], "rb");
			if(fp_img == NULL)
			{
				printf("Failed to open image %s\n", argv[3]);
				exit(-1);
			}
			fseek(fp_img, 0, SEEK_END);
			message.length += ftell(fp_img);
			printf("Adding %ld bytes of binary to end of elf\n", ftell(fp_img));
			fseek(fp_img, 0, SEEK_SET);
		}

		txbuf = (unsigned char *)malloc(message.length);
		if (txbuf == NULL)
		{
			printf("Failed to allocate memory\n");
			exit(-1);
		}

		size = fread(txbuf, 1, message.length, fp);
		if(fp_img)
		{
			size += fread(txbuf + size, 1, message.length - size, fp_img);
		}

		size =
		    ep_write((unsigned char *)&message, sizeof(message),
			     usb_device);
		if (size != sizeof(message))
		{
			printf("Failed to write correct length, returned %d\n",
			       size);
			exit(-1);
		}

		size = ep_write(txbuf, message.length, usb_device);
		if (size != message.length)
		{
			printf("Failed to read correct length, returned %d\n",
			       size);
			exit(-1);
		}

		sleep(1);

		size =
		    ep_read((unsigned char *)&retcode, sizeof(retcode),
			    usb_device);

		if (retcode == 0)
		{
			printf("Successful\n");
			if(fp==fp2)
			{
				printf("Raspberry Pi is now a mass storage device, use lsblk to find it\n");
				exit(0);
			}
		}
		else
			printf("Failed : 0x%x", retcode);

		sleep(1);
		libusb_close(usb_device);
	}

	libusb_exit(ctx);

	return 0;
}
