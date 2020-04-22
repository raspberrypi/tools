BINS=armstub.bin armstub7.bin armstub8-32.bin armstub8-32-gic.bin armstub8.bin armstub8-gic.bin armstub8-gic-highperi.bin

CC8?=aarch64-linux-gnu-gcc
LD8?=aarch64-linux-gnu-ld
OBJCOPY8?=aarch64-linux-gnu-objcopy
OBJDUMP8?=aarch64-linux-gnu-objdump -maarch64

CC7?=arm-linux-gnueabihf-gcc -march=armv7-a
LD7?=arm-linux-gnueabihf-ld
OBJCOPY7?=arm-linux-gnueabihf-objcopy
OBJDUMP7?=arm-linux-gnueabihf-objdump -marm

BIN2C=./bin2c

all : $(BINS)

clean :
	rm -f *.o *.out *.tmp *.bin *.elf *.ds *.C armstubs.h bin2c *~

%8.o: %8.S
	$(CC8) -c $< -o $@

%8-gic.o: %8.S
	$(CC8) -DGIC=1 -DBCM2711=1 -c $< -o $@

%8-gic-highperi.o: %8.S
	$(CC8) -DGIC=1 -DHIGH_PERI=1 -DBCM2711=1 -c $< -o $@

%8-32.o: %7.S
	$(CC7) -DBCM2710=1 -c $< -o $@

%8-32-gic.o: %7.S
	$(CC7) -DGIC=1 -DBCM2710=1 -DBCM2711=1 -c $< -o $@

%.o: %.S
	$(CC7) -DBCM2710=0 -c $< -o $@

%8-gic.elf: %8-gic.o
	$(LD8) --section-start=.text=0 $< -o $@

%8-gic-highperi.elf: %8-gic-highperi.o
	$(LD8) --section-start=.text=0 $< -o $@

%8.elf: %8.o
	$(LD8) --section-start=.text=0 $< -o $@

%.elf: %.o
	$(LD7) --section-start=.init=0 $< -o $@

%8-gic.tmp: %8-gic.elf
	$(OBJCOPY8) $< -O binary $@

%8-gic-highperi.tmp: %8-gic-highperi.elf
	$(OBJCOPY8) $< -O binary $@

%8.tmp: %8.elf
	$(OBJCOPY8) $< -O binary $@

%.tmp: %.elf
	$(OBJCOPY7) $< -O binary $@


%.bin: %.tmp
	dd if=$< ibs=256 of=$@ conv=sync

%8.ds: %8.bin
	$(OBJDUMP8) -D --target binary $< > $@

%.ds: %.bin
	$(OBJDUMP7) -D --target binary $< > $@

%.C: %.bin bin2c
	$(BIN2C) $< > $@

$(BIN2C): bin2c.c
	gcc $< -o $@

armstubs.h: armstub.C armstub7.C armstub8-32.C armstub8-32-gic.C armstub8.C armstub8-gic.C armstub8-gic-highperi.C
	echo 'static const unsigned armstub[] = {' > $@
	cat armstub.C >> $@
	echo '};' >> $@
	echo 'static const unsigned armstub7[] = {' >> $@
	cat armstub7.C >> $@
	echo '};' >> $@
	echo 'static const unsigned armstub8_32[] = {' >> $@
	cat armstub8-32.C >> $@
	echo '};' >> $@
	echo 'static const unsigned armstub8_32_gic[] = {' >> $@
	cat armstub8-32-gic.C >> $@
	echo '};' >> $@
	echo 'static const unsigned armstub8[] = {' >> $@
	cat armstub8.C >> $@
	echo '};' >> $@
	echo 'static const unsigned armstub8_gic[] = {' >> $@
	cat armstub8-gic.C >> $@
	echo '};' >> $@
	echo 'static const unsigned armstub8_gic_highperi[] = {' >> $@
	cat armstub8-gic-highperi.C >> $@
	echo '};' >> $@

