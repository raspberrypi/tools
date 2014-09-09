# USB boot code

This is the USB MSD boot code which should work on the Raspberry Pi model A and the Compute Module.

## Building

Check out this on your Pi or an Ubuntu linux machine

```
$ git clone --depth=1 https://github.com/raspberrypi/tools
$ cd tools/usbboot
$ sudo apt-get install libusb-1.0-0-dev
$ make
$ sudo ./rpiboot


