BINS=armstub.bin armstub7.bin armstub8-32.bin armstub8-32-gic.bin armstub8.bin armstub8-gic.bin

CC8=aarch64-linux-gnu-gcc
LD8=aarch64-linux-gnu-ld
OBJCOPY8=aarch64-linux-gnu-objcopy
OBJDUMP8=aarch64-linux-gnu-objdump -maarch64

CC=arm-linux-gnueabihf-gcc -march=armv7-a
LD=arm-linux-gnueabihf-ld
OBJCOPY=arm-linux-gnueabihf-objcopy
OBJDUMP=arm-linux-gnueabihf-objdump -marm

BIN2C=./bin2c

all : $(BINS)

clean :
	rm -f *.o *.out *.tmp *.bin *.ds *.C armstubs.h bin2c *~

%8.o: %8.S
	$(CC8) -c $< -o $@

%8-gic.o: %8.S
	$(CC8) -DGIC=1 -c $< -o $@

%8-32.o: %7.S
	$(CC) -DBCM2710=1 -c $< -o $@

%8-32-gic.o: %7.S
	$(CC) -DGIC=1 -DBCM2710=1 -DBCM2711=1 -c $< -o $@

%.o: %.S
	$(CC) -DBCM2710=0 -c $< -o $@

%8-gic.elf: %8-gic.o
	$(LD8) --section-start=.text=0 $< -o $@

%8.elf: %8.o
	$(LD8) --section-start=.text=0 $< -o $@

%.elf: %.o
	$(LD) --section-start=.init=0 $< -o $@

%8-gic.tmp: %8-gic.elf
	$(OBJCOPY8) $< -O binary $@

%8.tmp: %8.elf
	$(OBJCOPY8) $< -O binary $@

%.tmp: %.elf
	$(OBJCOPY) $< -O binary $@


%.bin: %.tmp
	dd if=$< ibs=512 count=1 of=$@ conv=sync

%8.ds: %8.bin
	$(OBJDUMP8) -D --target binary $< > $@

%.ds: %.bin
	$(OBJDUMP) -D --target binary $< > $@

%.C: %.bin bin2c
	$(BIN2C) $< > $@

$(BIN2C): bin2c.c
	gcc $< -o $@

armstubs.h: armstub.C armstub7.C armstub8-32.C armstub8-32-gic.C armstub8.C armstub8-gic.C
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

