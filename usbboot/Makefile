rpiboot: main.c
	$(CC) -g -o $@ $< -lusb-1.0

install: rpiboot
	cp rpiboot /usr/bin
	mkdir -p /usr/share/rpiboot
	cp usbbootcode.bin /usr/share/rpiboot
	cp msd.elf /usr/share/rpiboot
	cp buildroot.elf /usr/share/rpiboot

uninstall:
	rm -f /usr/bin/rpiboot
	rm -f /usr/share/rpiboot/usbbootcode.bin
	rm -f /usr/share/rpiboot/msd.elf
	rm -f /usr/share/rpiboot/buildroot.elf
	rmdir --ignore-fail-on-non-empty /usr/share/rpiboot/

clean: 
	rm rpiboot

.PHONY: install uninstall clean
