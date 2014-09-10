#include "libusb-1.0/libusb.h"
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>

#include <string.h>

/* Defines the LibUSB maximum transfer size as 507904 bytes
   Transfers above this size seem to give errors */
#define LIBUSB_MAX_TRANSFER 507904

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
	int total_len = 0;
	int transfer_len;

	len &= 0x7fffffff; // Ignore MSB of length, because it is used to signal whether the file being sent is an ELF file
	do {
		transfer_len = len < LIBUSB_MAX_TRANSFER ? len : LIBUSB_MAX_TRANSFER;
		libusb_bulk_transfer(usb_device, 0x01, buf, transfer_len, &a_len, 1000);
		total_len += a_len;
		buf += a_len;
		len -= a_len;
	} while (len);

	return total_len;
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
	char def1[] = "usbbootcode.bin";
	char def2[] = "msd.bin";
	char *str1, *str2, *str;
	struct MESSAGE_S {
		int length;
		unsigned char signature[20];
	} message;

	if (argc < 3)
	{
		str1 = def1;
		str2 = def2;
	} else
	{
		str1 = argv[1];
		str2 = argv[2];
	}

	fp1 = fopen(str1, "rb");
	if (fp1 == NULL)
	{
		printf("Cannot open file %s\n", str1);
		exit(-1);
	}

	fp2 = fopen(str2, "rb");
	if (fp2 == NULL)
	{
		printf("Cannot open file %s\n", str2);
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
		FILE *fp;

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

		{
			struct libusb_device_descriptor desc;
			ret =
			    libusb_get_device_descriptor(libusb_get_device
							 (usb_device), &desc);
			printf("Found serial = %d: writing file %s\n",
			       desc.iSerialNumber,
			       desc.iSerialNumber == 0 ? str1 : str2);
			fp = desc.iSerialNumber == 0 ? fp1 : fp2;
			str = desc.iSerialNumber == 0 ? str1 : str2;
		}

		fseek(fp, 0, SEEK_END);
		message.length = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		txbuf = (unsigned char *)malloc(message.length);
		if (txbuf == NULL)
		{
			printf("Failed to allocate memory\n");
			exit(-1);
		}

		fread(txbuf, 1, message.length, fp);

		size =
		    ep_write((unsigned char *)&message, sizeof(message),
			     usb_device);
		if (size != sizeof(message))
		{
			printf("Failed to write correct length, returned %d\n",
			       size);
			exit(-1);
		}

		// Set MSB of length as 1 if the file being sent is an ELF file
		if (strcmp(&str[strlen(str)-4], ".elf") == 0)
		{
			size = ep_write(txbuf, message.length | (1 << 31), usb_device);
		}
		else
		{
			size = ep_write(txbuf, message.length, usb_device);
		}
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
				if (strncmp(str, "msd", 3) == 0)
				{
					printf("Raspberry Pi is now a mass storage device, use lsblk to find it\n");
				}
				else
				{
					printf("Raspberry Pi has successfully received the final binary/ELF file\n");
				}
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
