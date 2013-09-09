# Name of the output program.
PRG            = led-board

# Optimization settings
# OPTIMIZE       = -O2 -g
OPTIMIZE       = -Os -flto

# Unused for now
LIBS           =

# Enable/disable debugging code. Available switches are:
#
# Additional trace output:
# 	TRACE_STARTUP, TRACE_COMMANDS, TRACE_FLASH, TRACE_ERRORS, TRACE_USART
# Suppress all trace output:
#	NDEBUG
# Simplifications for debugging
# 	SHORT_LOOPS, OMIT_HEAT_CHECK
# Cycle counting:
# 	COUNT_USART_ISR
# Additionaly sanity checks in usart1.c
#       USART1_CHECKS
DBG = -DOMIT_HEAT_CHECK -DTRACE_ERRORS -DUSART1_CHECKS

# 500000: normal, 115200: raspi
BUS_BAUDRATE = 500000

# End of configuration section.

OBJ            = color.o command.o config.o console.o console_prompt.o debug.o error.o fail.o flash.o heat.o main.o pwm.o startup.o usart1.o usart2.o

CC             = arm-none-eabi-gcc
OBJCOPY        = arm-none-eabi-objcopy
OBJDUMP        = arm-none-eabi-objdump

# Override is only needed by avr-lib build system.

override CFLAGS        = -Wall $(OPTIMIZE) -std=c99 -msoft-float -mfloat-abi=soft -mcpu=cortex-m3 -mthumb $(DBG) -DBUS_BAUDRATE=$(BUS_BAUDRATE)
override LDFLAGS       = -Wl,-Map,$(PRG).map -nostartfiles -Tlinker.ld


all: git_version.h $(PRG).elf lst text eeprom

git_version.h:
	echo "#define GIT_VERSION_ID \""$(shell git rev-parse HEAD)"\"" > git_version.h

$(PRG).elf: $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -rf *.o $(PRG).elf *.eps *.png *.pdf *.bak
	rm -rf *.lst *.map *.bin *.hex *.srec $(EXTRA_CLEAN_FILES)
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
