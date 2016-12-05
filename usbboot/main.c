#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <unistd.h>

#include "libusb-1.0/libusb.h"

int verbose = 0;
int out_ep = 1;
int in_ep = 2;

void usage(int error)
{
	FILE * dest = error ? stderr : stdout;

	fprintf(dest, "Usage: rpiboot\n");
	fprintf(dest, "   or: rpiboot -b fatimage\n");
	fprintf(dest, "Boot a Raspberry Pi Model A or Compute Module through USB\n");
	fprintf(dest, "\n");
	fprintf(dest, "rpiboot			: Boot the device into Mass Storage Device mode\n");
	fprintf(dest, "rpiboot -b fatimage	: Boot the device into a buildroot linux image\n");
	fprintf(dest, "\n");
	fprintf(dest, "Further options:\n");
	fprintf(dest, "      -x executable      : Autoexecute function\n");
	fprintf(dest, "           This option is used to trigger the execution of a\n");
	fprintf(dest, "           script after finishing the USB boot process\n");
	fprintf(dest, "      -l                 : Sit in a loop permanently\n");
	fprintf(dest, "      -v                 : Verbose output");
	fprintf(dest, "      -h                 : This help\n");
	exit(-1);
}

libusb_device_handle * LIBUSB_CALL open_device_with_vid(
	libusb_context *ctx, uint16_t vendor_id)
{
	struct libusb_device **devs;
	struct libusb_device *found = NULL;
	struct libusb_device *dev;
	struct libusb_device_handle *handle = NULL;
	size_t i = 0;
	int r;

	if (libusb_get_device_list(ctx, &devs) < 0)
		return NULL;

	while ((dev = devs[i++]) != NULL) {
		struct libusb_device_descriptor desc;
		r = libusb_get_device_descriptor(dev, &desc);
		if (r < 0)
			goto out;
		if(verbose)
			printf("Found device %d idVendor=0x%04x idProduct=0x%04x\n", i, desc.idVendor, desc.idProduct);
		if (desc.idVendor == vendor_id) {
			if(desc.idProduct == 0x2763 ||
			   desc.idProduct == 0x2764)
			{
				if(verbose) printf("Device located successfully\n");
				found = dev;
				break;
			}
		}
	}

	if (found) {
		r = libusb_open(found, &handle);
		if (r < 0)
		{
			if(verbose) printf("Failed to open the requested device\n");
			handle = NULL;
		}
	}

out:
	libusb_free_device_list(devs, 1);
	return handle;

}

int Initialize_Device(libusb_context ** ctx, libusb_device_handle ** usb_device)
{
	int ret = 0;
	int interface;
	struct libusb_config_descriptor *config;

	*usb_device = open_device_with_vid(*ctx, 0x0a5c);
	if (*usb_device == NULL)
	{
		return -1;
	}

	libusb_get_active_config_descriptor(libusb_get_device(*usb_device), &config);

	if(config->bNumInterfaces == 1)
	{
		interface = 0;
		out_ep = 1;
		in_ep = 2;
	}
	else
	{
		interface = 1;
		out_ep = 3;
		in_ep = 4;
	}

	ret = libusb_claim_interface(*usb_device, interface);
	if (ret)
	{
		printf("Failed to claim interface\n");
		return ret;
	}

	printf("Initialised device correctly\n");

	return ret;
}

int ep_write(unsigned char *buf, int len, libusb_device_handle * usb_device)
{
	int a_len;
	int ret =
	    libusb_control_transfer(usb_device, LIBUSB_REQUEST_TYPE_VENDOR, 0,
				    len & 0xffff, len >> 16, NULL, 0, 1000);

	if(ret != 0)
	{
		printf("Failed control transfer\n");
		return ret;
	}

	ret = libusb_bulk_transfer(usb_device, out_ep, buf, len, &a_len, 100000);
	if(verbose)
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

char *getfilepath(char *filename)
{
	char *progpath, *filepath, *progdir;
	ssize_t len;

	/* If file is available locally, use it */
	if (access(filename, F_OK) != -1)
		return filename;

	/* Otherwise, use the installed version */
	progpath = malloc(PATH_MAX);
	len = readlink("/proc/self/exe", progpath, PATH_MAX - 1);
	if (len == -1)
	{
		free(progpath);
		return NULL;
	}

	progpath[len] = '\0';
	progdir = dirname(progpath);
	if (asprintf(&filepath, "%s/../share/rpiboot/%s", progdir, filename) < 0)
	{
		free(progpath);
		return NULL;
	}

	free(progpath);

	return filepath;
}

int main(int argc, char *argv[])
{
	int result;
	libusb_context *ctx;
	libusb_device_handle *usb_device;
	unsigned char *txbuf;
	int size;
	int retcode;
	int last_serial = -1;
	FILE *fp1, *fp2, *fp;

	char def1_name[] = "usbbootcode.bin";
	char def2_name[] = "msd.elf";
	char def3_name[] = "buildroot.elf";

	char *def1, *def2, *def3;

	char *stage1   = NULL, *stage2     = NULL;
	char *fatimage = NULL, *executable = NULL;
	int loop       = 0;

	def1 = getfilepath(def1_name);
	def2 = getfilepath(def2_name);
	def3 = getfilepath(def3_name);

	if (!def1 || !def2 || !def3)
	{
		fprintf(stderr, "One of %s, %s or %s cannot be found\n",
			def1_name, def2_name, def3_name);
		exit(1);
	}

	stage1   = def1;
	stage2   = def2;

	struct MESSAGE_S {
		int length;
		unsigned char signature[20];
	} message;
	
#if defined (__CYGWIN__)
  //printf("Running under Cygwin\n");
#else
	//exit if not run as sudo
	if(getuid() != 0)
	{
		printf("Must be run with sudo...\n");
		exit(-1);
	}
#endif

	// Skip the command name
	argv++; argc--;
	while(*argv)
	{
		if(strcmp(*argv, "-b") == 0)
		{
			argv++; argc--;
			if(argc < 1)
				usage(1);
			stage1 = def1;
			stage2 = def3;
			fatimage = *argv;
		}
		else if(strcmp(*argv, "-h") == 0 || strcmp(*argv, "--help") == 0)
		{
			usage(0);
		}
		else if(strcmp(*argv, "-x") == 0)
		{
			argv++; argc--;
			executable = *argv;
		}
		else if(strcmp(*argv, "-l") == 0)
		{
			loop = 1;
		}
		else if(strcmp(*argv, "-v") == 0)
		{
			verbose = 1;
		}
		else
		{
			usage(1);
		}
		
		argv++; argc--;
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

	libusb_set_debug(ctx, verbose ? LIBUSB_LOG_LEVEL_WARNING : 0);

	do
	{
		FILE *fp_img = NULL;
		struct libusb_device_descriptor desc;
		struct libusb_config_descriptor *config;

		printf("Waiting for BCM2835 ...\n");

		// Wait for a device to get plugged in
		do
		{
			result = Initialize_Device(&ctx, &usb_device);
			if(result == 0)
			{
				libusb_get_device_descriptor(libusb_get_device
								 (usb_device), &desc);
				printf("Found serial number %d\n", desc.iSerialNumber);
				// Make sure we've re-enumerated since the last time
				if(desc.iSerialNumber == last_serial)
				{
					result = -1;
					libusb_close(usb_device);
				}			

				libusb_get_active_config_descriptor(libusb_get_device(usb_device), &config);
			}

			if (result)
			{
				usleep(100);
			}
		}
		while (result);

		last_serial = desc.iSerialNumber;
		printf("Found serial = %d: writing file %s\n",
		       desc.iSerialNumber,
		       desc.iSerialNumber == 0 ? stage1 : stage2);
		fp = desc.iSerialNumber == 0 ? fp1 : fp2;

		fseek(fp, 0, SEEK_END);
		message.length = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		if(desc.iSerialNumber == 1 && fatimage != NULL)
		{
			// Been given a filesystem image
			fp_img = fopen(fatimage, "rb");
			if(fp_img == NULL)
			{
				printf("Failed to open image %s\n", fatimage);
				exit(-1);
			}
			fseek(fp_img, 0, SEEK_END);
			message.length += ftell(fp_img);
			if(verbose) printf("Adding %ld bytes of binary to end of elf\n", ftell(fp_img));
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
		if(verbose) printf("Writing %d bytes\n", message.length);
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
			printf("Successful read %d bytes \n", size);

			if(fp == fp2 && executable)
			{
				system(executable);
			}
		}
		else
			printf("Failed : 0x%x", retcode);

		libusb_close(usb_device);
		sleep(1);
	}
	while(fp == fp1 || loop);

	libusb_exit(ctx);

	return 0;
}
