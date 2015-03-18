# USB boot code

This is the USB MSD boot code which should work on the Raspberry Pi model A and the Compute Module.

The BCM2835 device has a number of different modes for booting, the most obvious one is the SD card
which looks for a file called bootcode.bin on the first FAT partition of the SD card to boot from.

The second method it can use is USB, the BCM2835 becomes a USB device which a standard PC (a Windows
or Linux PC or even a Raspberry Pi) will enumerate.  The usbbootcode.bin is then squirted down the USB
to the BCM2835.

This is then executing the second stage boot process that normally looks for a file called config.txt
and start.elf on the SD card and loads start.elf (or whatever version of start you require depending
on the settings in config.txt).

With usbbootcode.bin it re-enumerates as a device (but this time with a serial number of 1 to distinguish
the second stage from the primary boot) and waits for a message packet from the host.  After
The message packet it receives the data (the length is in the message packet), and it then checks the
length of the downloaded information (should be an .elf file either msd.elf or buildroot.elf), if the
length is greater than the size of the elf file, it assumes the rest of the data is a FAT16 image of a
filesystem as created using the Raspberry Pi buildroot target raspberrypi_defconfig

## Building

Clone this on your Pi or an Ubuntu linux machine

```
$ git clone --depth=1 https://github.com/raspberrypi/tools
$ cd tools/usbboot
$ sudo apt-get install libusb-1.0-0-dev
$ make
$ sudo make install
$ sudo rpiboot
```

Checkout the upstream buildroot from git

```
git clone git://git.buildroot.net/buildroot
```

Patch with Pi patches (requires raspberrypi/tools from above)

```
cd buildroot
patch -p1 < ../tools/usbboot/buildroot.patch
make raspberrypi_defconfig
make
```

Note: you'll need superuser password to finish the build and create the image

Running

```
sudo rpiboot -b buildroot/output/images/fatimage
```
