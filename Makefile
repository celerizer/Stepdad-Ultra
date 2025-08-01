.PHONY: all rename_spaces clean

all: rename_spaces Stepdad.z64

rename_spaces:
	@find ./roms -maxdepth 1 -type f -name "* *" | while read file; do \
		newfile=$$(echo "$$file" | tr ' ' '_'); \
		mv "$$file" "$$newfile"; \
		echo "Renamed: $$file -> $$newfile"; \
	done

CFLAGS += \
	-DH8_BIG_ENDIAN=1 \
	-DH8_HAVE_NETWORK_IMPL=0 \
	-DH8_HAVE_NETWORK_STUB=1 \
	-DH8_TESTS=1 \
	-DH8_REVERSE_BITFIELDS=1 \
	-O2 -funroll-loops \
	-std=c89 -Wall -Wextra

BUILD_DIR = build
SRC_DIR = src
include $(N64_INST)/include/n64.mk
include src/libh8300h/libh8300h.mk

assets_fnt = $(wildcard assets/*.fnt)
assets_ttf = $(wildcard assets/*.ttf)
assets_png = $(wildcard assets/*.png)

assets_bin = $(wildcard roms/*.bin)
assets_rom = $(wildcard roms/*.rom)

assets_conv = \
	$(addprefix filesystem/,$(notdir $(assets_ttf:%.ttf=%.font64))) \
	$(addprefix filesystem/,$(notdir $(assets_fnt:%.fnt=%.font64))) \
    $(addprefix filesystem/,$(notdir $(assets_png:%.png=%.sprite))) \
	$(addprefix filesystem/roms/,$(notdir $(assets_bin:%.bin=%.bin))) \
	$(addprefix filesystem/roms/,$(notdir $(assets_rom:%.rom=%.rom)))

MKSPRITE_FLAGS ?=
MKFONT_FLAGS ?= --range all

src = \
	$(SRC_DIR)/main.c

src += $(H8_SOURCES)

filesystem/%.font64: assets/%.ttf
	@mkdir -p $(dir $@)
	@echo "    [FONT] $@"
	$(N64_MKFONT) $(MKFONT_FLAGS) -o filesystem "$<"

filesystem/%.font64: assets/%.fnt
	@mkdir -p $(dir $@)
	@echo "    [FONT] $@"
	$(N64_MKFONT) $(MKFONT_FLAGS) -o filesystem "$<"

filesystem/%.sprite: assets/%.png
	@mkdir -p $(dir $@)
	@echo "    [SPRITE] $@"
	$(N64_MKSPRITE) $(MKSPRITE_FLAGS) -o filesystem "$<"

filesystem/roms/%: roms/%
	@mkdir -p "$(dir $@)"
	@echo "    [ROM] $@"
	@cp "$<" "$@"

filesystem/Tuffy_Bold.font64: MKFONT_FLAGS += --size 18 --outline 1

$(BUILD_DIR)/Stepdad.dfs: $(assets_conv) 
$(BUILD_DIR)/Stepdad.elf: $(src:%.c=$(BUILD_DIR)/%.o)

# Get the current git version
GIT_VERSION := $(shell git rev-parse --short=8 HEAD)

# Define the N64 ROM title with the git version
N64_ROM_TITLE_WITH_VERSION := "Stepdad $(GIT_VERSION)"

Stepdad.z64: N64_ROM_TITLE = $(N64_ROM_TITLE_WITH_VERSION)
Stepdad.z64: $(BUILD_DIR)/Stepdad.dfs

clean:
	rm -rf $(BUILD_DIR) filesystem *.z64

-include $(wildcard $(BUILD_DIR)/*.d)

.PHONY: clean
