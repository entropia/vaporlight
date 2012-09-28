PRG            = led-board
OBJ            = command.o config.o console.o console_prompt.o console_wp.o debug.o error.o fail.o heat.o led.o main.o startup.o usart.o

# OPTIMIZE       = -O0 -g
OPTIMIZE       = -flto -O2


LIBS           =

# Enable/disable debugging code. Available switches are:
#
# Additional trace output:
# 	TRACE_STARTUP, TRACE_COMMANDS, TRACE_FLASH, TRACE_ERRORS, TRACE_USART
# Simplifications for debugging
# 	SHORT_LOOPS, OMIT_HEAT_CHECK
# Cycle counting:
# 	COUNT_USART_ISR

DBG = -DOMIT_HEAT_CHECK -DTRACE_ERRORS

# You should not have to change anything below here.
# Oh, yes?

CC             = arm-elf-gcc

# Override is only needed by avr-lib build system.

override CFLAGS        = -Wall $(OPTIMIZE) -std=c99 -msoft-float -mcpu=cortex-m3 -mthumb $(DBG) $(VERSION)
override LDFLAGS       = -Wl,-Map,$(PRG).map -nostartfiles -Tlinker.ld

OBJCOPY        = arm-elf-objcopy
OBJDUMP        = arm-elf-objdump

all: gamma_table.inc git_version.h $(PRG).elf lst text eeprom

gamma_table.inc: make_gamma_table.py
	./make_gamma_table.py 13 1.8 1.8 1.8 1.8 > gamma_table.inc

git_version.h:
	echo "#define GIT_VERSION_ID \"" $(shell git rev-parse HEAD) > git_version.h "\""

$(PRG).elf: $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -rf *.o $(PRG).elf *.eps *.png *.pdf *.bak 
	rm -rf *.lst *.map *.bin *.hex *.srec $(EXTRA_CLEAN_FILES)
	rm -f gamma_table.inc
	rm -f git_version.h

lst:  $(PRG).lst

%.lst: %.elf
	$(OBJDUMP) -x -s -d $< > $@

# Rules for building the .text rom images

text: hex bin srec

hex:  $(PRG).hex
bin:  $(PRG).bin
srec: $(PRG).srec

%.hex: %.elf
	$(OBJCOPY) -j .text -j .data -j .config -O ihex $< $@

%.srec: %.elf
	$(OBJCOPY) -j .text -j .data -j .config -O srec $< $@

%.bin: %.elf
	$(OBJCOPY) -j .text -j .data -j .config -O binary $< $@

# Rules for building the .eeprom rom images

eeprom: ehex ebin esrec

ehex:  $(PRG)_eeprom.hex
ebin:  $(PRG)_eeprom.bin
esrec: $(PRG)_eeprom.srec

%_eeprom.hex: %.elf
	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O ihex $< $@ \
	|| { echo empty $@ not generated; exit 0; }

%_eeprom.srec: %.elf
	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O srec $< $@ \
	|| { echo empty $@ not generated; exit 0; }

%_eeprom.bin: %.elf
	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O binary $< $@ \
	|| { echo empty $@ not generated; exit 0; }
