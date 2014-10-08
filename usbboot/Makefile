rpiboot: main.c
	$(CC) -g -o $@ $< -lusb-1.0

install: rpiboot
	cp rpiboot /usr/bin
	mkdir -p /usr/share/rpiboot
	cp usbbootcode.bin /usr/share/rpiboot
	cp msd.bin /usr/share/rpiboot
