# Raspberry Pi Test

This code is an example Compute Module test system designed to run on a Raspberry Pi.  The idea is that the
test code talks to the unit under test through the serial port on the Raspberry Pi (/dev/ttyAMA0), this requires
that the test master is not already running a getty (i.e. a login prompt) on the serial port.

## Removing the getty from the serial port

First edit /etc/inittab and remove the line near the end which spawns and getty on /dev/ttyAMA0

## Connect UUT to Raspberry Pi

Connect the Unit Under Test to the Raspberry Pi through the USB, then use the tools/usbboot to run a buildroot
fat image on the compute module.  See [usbboot](../usbboot/Readme.md)

```
$ sudo rpiboot -b fatimage -x ./test_code
```

What this will do is use the USB booting mechanism to boot the UUT and run the buildroot image.  When it has
finished the downloading it then runs the test_code.

## Building the test code

To build the test code just use make

```
$ make
```

## What does it do?

There are a number of important functions defined in the test code in main.c

```
char * wait(int fd, char *string);
```

This function waits for the string (string) to come in from the file descriptor fd.  On return it returns a pointer
to a static buffer containing all of the text received up to and including the search string.

So you can do something like:

```
str = wait(fd, "login:");
writes(fd, "root\n");
wait(fd, "Password:");
writes(fd, "raspberry");
```

This will then log into the Linux system...

For further testing then you can add functionality to the test_set structure, the first entry in the structure
is the instruction to send to the UUT, the second entry is a string to match the output against and the
third entry is a function to call to test the output of the test.  You only need to provide the string or the test
function not both (the lsusb example provides a function to search for and count the number of "DEVICE" strings
to count the number of devices.  This will only actually work of course if you''ve unplugged the UUT and plugged
in a device (or forced the switch on the compute module to switch back to host mode)

