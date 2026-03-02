# Wii (devkitPPC + libogc) target bootstrap.
# Usage: make -C handheld/project/wii

DEVKITPRO ?= /opt/devkitpro
DEVKITPPC ?= $(DEVKITPRO)/devkitPPC
WII_RULES := $(DEVKITPPC)/wii_rules

ifeq (,$(wildcard $(WII_RULES)))
all:
	@echo "devkitPPC toolchain not found."
	@echo "Set DEVKITPRO/DEVKITPPC before building, for example:"
	@echo "  export DEVKITPRO=/opt/devkitpro"
	@echo "  export DEVKITPPC=\$$DEVKITPRO/devkitPPC"
	@echo "Then rerun: make -C handheld/project/wii"

clean:
	@echo "Nothing to clean (toolchain not configured)."

.PHONY: all clean

else

include $(WII_RULES)

TARGET      := minecraftwii
BUILD       := build
SOURCES_DIR := ../../src
INCLUDES    := -I$(SOURCES_DIR) -I../../lib/include -I$(LIBOGC_INC)

# Keep game logic unchanged; only platform/entry bindings are swapped per target.
EXCLUDED_SRCS := \
	$(SOURCES_DIR)/main_android.cpp \
	$(SOURCES_DIR)/main_android_java.cpp \
	$(SOURCES_DIR)/main_rpi.cpp \
	$(SOURCES_DIR)/main_dedicated.cpp

SOURCES := $(filter-out $(EXCLUDED_SRCS),$(shell find $(SOURCES_DIR) -name '*.cpp'))

CXXFLAGS += -DWII -DPOSIX -DGFX_SMALLER_CHUNKS -fno-exceptions -fno-rtti $(INCLUDES)
LDFLAGS  += -L$(LIBOGC_LIB)
LIBS     := -logc -lbte -lm -lz -lfat

OBJECTS := $(patsubst $(SOURCES_DIR)/%.cpp,$(BUILD)/%.o,$(SOURCES))

all: $(TARGET).elf

$(TARGET).elf: $(OBJECTS)
	@echo linking $@
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS) $(LIBS)

$(BUILD)/%.o: $(SOURCES_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD) $(TARGET).elf

.PHONY: all clean

endif
