# Ce makefile utilise une implémentation perso de color-gcc
# pour d'autres pc, il suffit de remplacer avr-gcc-color par avr-gcc

MCU = at90can128
TARGET = main
FREQ=16000000

# Dossiers contenant les .c à compiler
SRC_DIRS =.\
drivers\
libs\
periphs

# Optimisation [0, 1, 2, 3, s].
OPT = 3

# Ajout éventuel de divers -D
CDEFS = -DF_CPU=$(FREQ)UL
CDEFS +=

AVRDUDE_HFUSE = 0xDF
AVRDUDE_LFUSE = 0xFF
AVRDUDE_EFUSE = 0xFF
AVRDUDE_PORT = /dev/ttyUSB0
AVRDUDE_OPS = -U flash:w:gcc/$(TARGET).hex
#AVRDUDE_OPS += -U eeprom:w:gcc/$(TARGET).eep

# niveau de détails affichés {-q -q; -q; ; -v; -v -v}
AVRDUDE_VERBOSITY = -q -q








SRC = $(shell bash -c "for d in $(SRC_DIRS); do find \"\$$d\" -maxdepth 1 -name \"*.c\" -printf \"%p \"; done")
OBJ = $(addprefix gcc/,$(SRC:.c=.o))

all: begin gccversion build size end

love: all program clean

clean: begin clean_list end

build: dir elf hex eep lss sym

elf: gcc/$(TARGET).elf
hex: gcc/$(TARGET).hex
eep: gcc/$(TARGET).eep
lss: gcc/$(TARGET).lss
sym: gcc/$(TARGET).sym

begin:
	@echo
	@echo "-------- begin --------"

end:
	@echo "--------  end  --------"
	@echo

dir:
	@if test ! -d gcc; then mkdir gcc; mkdir gcc/dep; fi
	@bash -c "for d in $(SRC_DIRS); do if test ! -d \"gcc/\$$d\"; then mkdir \"gcc/\$$d\"; fi; done"

size:
	@if test -f gcc/$(TARGET).elf; then echo; avr-mem.sh gcc/$(TARGET).elf $(MCU) 2>/dev/null; fi

gccversion :
	@avr-gcc --version

program: gcc/$(TARGET).hex gcc/$(TARGET).eep
	avrdude $(AVRDUDE_VERBOSITY) -p $(MCU) -P $(AVRDUDE_PORT) -c stk500v2 $(AVRDUDE_OPS)

fuses:
	avrdude -q -p $(MCU) -P $(AVRDUDE_PORT) -c stk500v2 -U hfuse:w:$(AVRDUDE_HFUSE):m -U lfuse:w:$(AVRDUDE_LFUSE):m -U efuse:w:$(AVRDUDE_EFUSE):m

clean_list :
	@echo
	@echo "Cleaning project:"
	rm -rf ./gcc/

gcc/$(TARGET).hex: gcc/$(TARGET).elf
	@echo
	@echo "Creating load file for Flash:" $@
	avr-objcopy -O ihex -R .eeprom $< $@

gcc/$(TARGET).eep: gcc/$(TARGET).elf
	@echo
	@echo "Creating load file for EEPROM:" $@
	-avr-objcopy -j .eeprom --set-section-flags=.eeprom="alloc,load" --change-section-lma .eeprom=0 --no-change-warnings -O ihex $< $@

gcc/$(TARGET).lss: gcc/$(TARGET).elf
	@echo
	@echo "Creating Extended Listing:" $@
	avr-objdump -w -h -S $< > $@

gcc/$(TARGET).sym: gcc/$(TARGET).elf
	@echo
	@echo "Creating Symbol Table:" $@
	avr-nm -n $< > $@

gcc/$(TARGET).elf: $(OBJ)
	@echo
	@echo "Linking:" $@
	avr-gcc-color -mmcu=$(MCU) -Wl,-Map=gcc/$(TARGET).map $^ --output $@

gcc/%.o : %.c
	@echo
	@echo "Compiling:" $<
	avr-gcc-color -c -mmcu=$(MCU) -I. -gdwarf-2 -std=gnu99 $(CDEFS) -O$(OPT) -Wall -Wstrict-prototypes -Wunused -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums -finline-functions -Wextra -Wshadow -Winline -Wundef -Wfloat-equal -Wuninitialized -Wswitch -Wswitch-default -Wswitch-enum -Wmissing-braces -Wmissing-prototypes $(patsubst %,-I%,$(EXTRAINCDIRS)) -MD -MP -MF gcc/dep/$(@F).d $< -o $@
