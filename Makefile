SDCC ?= sdcc
STCCODESIZE ?= 8000
SDCCOPTS ?= --iram-size 256 --xram-size 256 --code-size $(STCCODESIZE) --data-loc 0x30 --disable-warning 158
SRC = src/adc.c src/alarm.c src/clock.c src/display.c src/ds1302.c src/font.c src/key_handler.c src/timer.c src/utility.c
# src/utility.c
OBJ=$(patsubst src%.c,build%.rel, $(SRC))

STCGAL=/usr/local/bin/stcgal
STCGALPORT=/dev/ttyUSB0
STCGALPROT=auto
SYSCLK=22118.4
STCGALOPTS=
FLASHFILE=main.hex

.PHONY : doall

all: main

build/%.rel: src/%.c src/%.h doall
	mkdir -p $(dir $@)
	$(SDCC) $(SDCCOPTS) -o $@ -c $<

main: $(OBJ)
	$(SDCC) -o build/ src/$@.c $(SDCCOPTS) $^
	cp build/$@.ihx $@.hex
	gawk -f lastadr.awk main.hex

clean:
	rm -f *.ihx *.hex *.bin
	rm -rf build/*

flash:
	#$$(STCGAL) -p $(STCGALPORT) -P $(STCGALPROT) -t $(SYSCLK) $(STCGALOPTS) $(FLASHFILE)
	$(STCGAL) -p $(STCGALPORT) -t $(SYSCLK) $(FLASHFILE)
