rpiboot: main.c
	$(CC) -g $(CFLAGS) -o $@ $< -lusb-1.0 $(LDFLAGS)

install: rpiboot
	cp rpiboot $(DESTDIR)/usr/bin
	mkdir -p $(DESTDIR)//usr/share/rpiboot
	cp usbbootcode.bin $(DESTDIR)/usr/share/rpiboot
	cp msd.elf $(DESTDIR)/usr/share/rpiboot
	cp buildroot.elf $(DESTDIR)/usr/share/rpiboot

uninstall:
	rm -f $(DESTDIR)/usr/bin/rpiboot
	rm -f $(DESTDIR)/usr/share/rpiboot/usbbootcode.bin
	rm -f $(DESTDIR)/usr/share/rpiboot/msd.elf
	rm -f $(DESTDIR)/usr/share/rpiboot/buildroot.elf
	rmdir --ignore-fail-on-non-empty $(DESTDIR)/usr/share/rpiboot/

clean: 
	rm rpiboot

.PHONY: install uninstall clean
