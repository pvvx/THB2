##############################################################################
PROJECT_NAME ?= ota_upboot
#POJECT_DEF ?= -DDEVICE=DEVICE_THB2
##############################################################################
COM_PORT = COM11
##############################################################################
# Source
SRC_PATH = ./source
SRC_PRJ = main.c

INCLUDES = -I$(SRC_PATH)
INCLUDES += -I$(SRC_PATH)/include
INCLUDES += -I$(SRC_PATH)/misc
INCLUDES += -I$(SRC_PATH)/misc/CMSIS/include
INCLUDES += -I$(SRC_PATH)/misc/CMSIS/device/phyplus

STARTUP_ASM = $(SRC_PATH)/misc/CMSIS/device/phyplus/phy6222_start.s

SRCS = $(addprefix $(SRC_PATH)/, $(SRC_PRJ))
SRCS += $(SRC_PATH)/misc/CMSIS/device/phyplus/phy6222_vectors.c

##############################################################################
DEFINES += -D__GCC
DEFINES += $(POJECT_DEF)
DEFINES += -DPHY_MCU_TYPE=MCU_BUMBEE_M0

##############################################################################
BIN_DIR = ./bin
OBJ_DIR = ./build
PYTHON = python3
GCC_PATH = 
CC = $(GCC_PATH)arm-none-eabi-gcc
OBJCOPY = $(GCC_PATH)arm-none-eabi-objcopy
OBJDUMP = $(GCC_PATH)arm-none-eabi-objdump
SIZE = $(GCC_PATH)arm-none-eabi-size
READELF = $(GCC_PATH)arm-none-eabi-readelf
##############################################################################
ARCH_FLAGS := -mcpu=cortex-m0 -mthumb -mthumb-interwork
OPT_CFLAGS ?= -Os
DEB_CFLAGS ?= -g3 -ggdb
##############################################################################
ASFLAGS	   := $(ARCH_FLAGS) $(OPT_CFLAGS) $(DEB_CFLAGS)
CFLAGS     := $(ARCH_FLAGS) $(OPT_CFLAGS) $(DEB_CFLAGS)
CFLAGS     += -W -Wall --std=gnu99
CFLAGS     += -fno-diagnostics-show-caret
CFLAGS     += -fdata-sections -ffunction-sections
CFLAGS     += -funsigned-char -funsigned-bitfields
#CFLAGS     += -fpack-struct
#CFLAGS     += -mno-unaligned-access
#CFLAGS     += -munaligned-access
CFLAGS     += -fms-extensions
CFLAGS     += -specs=nosys.specs
CFLAGS     += -Wl,--gc-sections

LDSCRIPT   ?= $(SRC_PATH)/misc/ota_upboot.ld
#BIN_OTA	   = $(OBJ_DIR)/$(PROJECT_NAME).bin
BIN_OTA	   = ./$(PROJECT_NAME).add

LDFLAGS    := $(ARCH_FLAGS)
LDFLAGS    += --static -nostartfiles -nostdlib
LDFLAGS    += -Wl,--gc-sections
LDFLAGS    += -Wl,--script=$(LDSCRIPT)
LDFLAGS    += -Wl,--just-symbols=$(SRC_PATH)/misc/bb_rom_sym_m0.gcc
LDFLAGS    += -Wl,-Map=$(OBJ_DIR)/$(PROJECT_NAME).map 
LIBS       += -Wl,--start-group -lgcc -lnosys -Wl,--end-group

##############################################################################

CFLAGS  += $(DEFINES) $(INCLUDES) 

#SRC_O = $(patsubst %.c,%.o,$(patsubst sdk/%, $(SRC_PATH)%, $(SRCS)))

SRC_O = $(SRCS:%.c=%.o) $(STARTUP_ASM:%.s=%.o)
OBJS = $(patsubst %, $(OBJ_DIR)/%, $(patsubst ./%, %, $(SRC_O)))

DEPENDENCY_LIST = $(OBJS:%o=%d)

##############################################################################
.PHONY: all directory clean size flash erase_and_flash

all: directory $(SRC_O) $(OBJ_DIR)/$(PROJECT_NAME).elf $(OBJ_DIR)/$(PROJECT_NAME).hex $(BIN_OTA) $(OBJ_DIR)/$(PROJECT_NAME).asm size

%.elf %.map: $(SRC_O) $(LDSCRIPT) Makefile
	@echo LD: $@
	@$(CC) $(LDFLAGS) $(OBJS) $(LIBS) -o $@

%.hex: %.elf
	@echo OBJCOPY: $@
	@$(OBJCOPY) -O ihex $^ $@

%.add: %.hex
	@echo Make: $@
	$(PYTHON) ./phy62x2_ota.py -o $(BIN_OTA) $(OBJ_DIR)/$(PROJECT_NAME).hex

%.asm: %.elf
	@echo OBJDUMP: $@
	@$(OBJDUMP) -s -S $^ >$@ 

%.o : %.c
	@echo CC: $<
	@mkdir -p $(OBJ_DIR)/$(dir $@)
	@$(CC) $(CFLAGS) $(INCFLAGS) -c $< -o $(OBJ_DIR)/$@
	@$(CC) -MM $(CFLAGS) $(INCFLAGS) $< -MT $@ -MF $(OBJ_DIR)/$(patsubst %.o,%.d,$@)

%.o : %.s
	@echo CC: $<
	@mkdir -p $(OBJ_DIR)/$(dir $@)
	@$(CC) $(CFLAGS) $(INCFLAGS) -c $< -o $(OBJ_DIR)/$@
	@$(CC) -MM $(CFLAGS) $(INCFLAGS) $< -MT $@ -MF $(OBJ_DIR)/$(patsubst %.o,%.d,$@)

flash:
	@$(PYTHON) ./rdwr_phy62x2.py -p$(COM_PORT) -b 1000000 -r wh $(OBJ_DIR)/$(PROJECT_NAME).hex

flash_ota:
	@$(PYTHON) ./rdwr_phy62x2.py -p$(COM_PORT) -b 1000000 -r we 0x10000 $(BIN_OTA)

erase_and_flash:
	@$(PYTHON) ./rdwr_phy62x2.py -p$(COM_PORT) -b 1000000 -e -r wh $(OBJ_DIR)/$(PROJECT_NAME).hex

reset:
	@$(PYTHON) ./rdwr_phy62x2.py -p$(COM_PORT) -r i

directory:
	@mkdir -p $(OBJ_DIR)

size: $(OBJ_DIR)/$(PROJECT_NAME).elf
	@echo size:
	@$(SIZE) -t $^
	@$(READELF) -l $^
	@echo

clean:
	@echo clean
	@-rm -rf $(OBJ_DIR)

-include $(DEPENDENCY_LIST)

VPATH:=$(OBJ_DIR)
