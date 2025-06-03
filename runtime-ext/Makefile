CC := powerpc-eabi-gcc
LD := $(CC)

MACHDEP := -DGEKKO -mrvl -mcpu=750 -meabi -mhard-float

CFILES := $(wildcard source/*.c) $(wildcard vendor/**/*.c)
OFILES := $(addprefix build/,$(CFILES:.c=.o))

INCLUDE := -I$(DEVKITPRO)/bslug/include -I../shared -Ivendor
CFLAGS	:= -nostdlib -mno-eabi -O2 -Wall $(MACHDEP) $(INCLUDE)

LDFLAGS := $(OFILES) $(MACHDEP) -Wl,-Tlinker.ld,--section-start=.init=81744260,--section-start=.text=81744660,--section-start=.rodata=81782fe0

# keep .dol as the first/default target so that 'make' alone builds this
runtime-ext.dol: runtime-ext.elf
	@echo "$<"
	@elf2dol runtime-ext.elf runtime-ext.dol

runtime-ext.elf: $(OFILES)
	@echo Linking...
	@$(LD) $(LDFLAGS) -o $@

build/%.o: %.c
	@echo "$<"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) $< -c -o $@

clean:
	@rm -fr build runtime-ext.elf runtime-ext.dol
