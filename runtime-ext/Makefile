# This Makefie produces multiple output executable DOL files.
# The file generation is controlled by the presence of `linker-X.ld' linker scripts,
# where X is any supported region. This Makefile uses all linker scripts to generate
# equivalent runtime-ext-X.dol files. These files may then be placed on the SD card
# for dynamic loading based on the game region.

CC := powerpc-eabi-gcc
LD := $(CC)

MACHDEP := -DGEKKO -mrvl -mcpu=750 -meabi -mhard-float

CFILES := $(wildcard source/*.c) $(wildcard vendor/**/*.c)
OFILES := $(addprefix build/,$(CFILES:.c=.o))

# Prevent deletion of .o files, helps with incr rebuilds
.SECONDARY: $(OFILES)

INCLUDE := -I$(DEVKITPRO)/bslug/include -I../shared -Ivendor
CFLAGS	:= -nostdlib -mno-eabi -O2 -Wall $(MACHDEP) $(INCLUDE)

LINKERSCRIPTS := $(wildcard linker-*.ld)
# get region characters
REGIONS := $(patsubst linker-%.ld, %, $(LINKERSCRIPTS))
LINKERSCRIPTPARAMS := $(addprefix -T, $(LINKERSCRIPTS))

# create dol and elf filenames
FILENAMES := $(addprefix runtime-ext-,$(REGIONS))
DOLS := $(addsuffix .dol, $(FILENAMES))
ELFS := $(addsuffix .elf, $(FILENAMES))

# Linker script is added per region in the elf rule
LDFLAGS := $(OFILES) $(MACHDEP) -Wl,--section-start=.init=81744260,--section-start=.text=81744660,--section-start=.rodata=81782fe0

all: $(DOLS)

runtime-ext-%.dol: $(ELFS)
	@echo runtime-ext-$*.elf
	@elf2dol runtime-ext-$*.elf $@

runtime-ext-%.elf: $(OFILES)
	@echo Linking for region: $*...
# Append -Tlinker-%.ld onto LDFLAGS so each region uses the correct linker script
	@$(LD) $(LDFLAGS),-Tlinker-$*.ld -o $@

build/%.o: %.c
	@echo "$<"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) $< -c -o $@

clean:
	@rm -fr build runtime-ext-*.elf runtime-ext-*.dol
