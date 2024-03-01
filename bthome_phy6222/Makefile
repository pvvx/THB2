##############################################################################
PROJECT_NAME ?= bthome_phy6222
#POJECT_DEF ?= -DDEVICE=DEVICE_THB2
##############################################################################
COM_PORT = COM11
##############################################################################
# Source
SRC_PATH = ./source
SRC_PRJ = main.c
SRC_PRJ += battery.c
SRC_PRJ += battservice.c
SRC_PRJ += ccm.c
SRC_PRJ += bthome_beacon.c
SRC_PRJ += osal_peripheral.c
SRC_PRJ += peripheral_main.c
SRC_PRJ += sbp_profile.c
SRC_PRJ += devinfoservice.c
SRC_PRJ += trigger.c
SRC_PRJ += sensors.c
SRC_PRJ += dev_i2c.c
SRC_PRJ += config.c
SRC_PRJ += cmd_parser.c
SRC_PRJ += thb2_main.c
SRC_PRJ += thb2_peripheral.c
SRC_PRJ += thservice.c
SRC_PRJ += flash_eep.c
SRC_PRJ += lcd_th05.c
SRC_PRJ += lcd_th05d.c
SRC_PRJ += lcd_th05f.c
SRC_PRJ += lcd_thb1.c
SRC_PRJ += ble_ota.c
SRC_PRJ += logger.c

INCLUDES = -I$(SRC_PATH)

SRCS = $(addprefix $(SRC_PATH)/, $(SRC_PRJ))
##############################################################################
DEFINES += -D__GCC
DEFINES += $(POJECT_DEF)
DEFINES += -DCFG_SLEEP_MODE=PWR_MODE_SLEEP
#DEFINES += -DCFG_SLEEP_MODE=PWR_MODE_NO_SLEEP
DEFINES += -DDEBUG_INFO=0
DEFINES += -DMTU_SIZE=240
DEFINES += -DMAX_NUM_LL_CONN=1
DEFINES += -DDEF_GAPBOND_MGR_ENABLE=0

#Debug:
DEFINES += -DTEST_RTC_DELTA=1
DEFINES += -DLL_DEBUG_NONE=1 
DEFINES += -DCLK_16M_ONLY=1
DEFINES += -DSTACK_MAX_SRAM=1
#if Flash HS
#DEFINES += -DXFLASH_HIGH_SPEED=1

#HOST_CONFIG:
DEFINES += -DBROADCASTER_CFG=0x01 
DEFINES += -DOBSERVER_CFG=0x02 
DEFINES += -DPERIPHERAL_CFG=0x04 
DEFINES += -DCENTRAL_CFG=0x08 
DEFINES += -DHOST_CONFIG=0x04

DEFINES += -DARMCM0
DEFINES += -DOSAL_CBTIMER_NUM_TASKS=1 
#  osal_cbtimer.h
DEFINES += -DENABLE_LOG_ROM_=0

#osal_heap info:
DEFINES += -DOSALMEM_METRICS=0

DEFINES += -DPHY_MCU_TYPE=MCU_BUMBEE_M0

#osal_snv.c:
DEFINES += -DUSE_FS=0

#Not used:
#DEFINES += -D_RTE_
#DEFINES += -DHCI_TL_NONE=1
#DEFINES += -D_OBJ_DIR_FOR_DTM_=0
#DEFINES += -DDBG_ROM_MAIN=0
#DEFINES += -DOSALMEM_METRICS=0
#DEFINES += -DAPP_CFG=0
#DEFINES += -DCFG_CP
#CTRL_CONFIG:
#DEFINES += -DADV_NCONN_CFG=0x01
#DEFINES += -DADV_CONN_CFG=0x02
#DEFINES += -DSCAN_CFG=0x04
#DEFINES += -DINIT_CFG=0x08

##############################################################################
BIN_DIR = ./bin
OBJ_DIR = ./build
SDK_PATH = ./SDK
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

ifdef BOOT_OTA
LDSCRIPT   ?= $(SDK_PATH)/misc/boot_ota_phy62x2.ld
DEFINES += -DOTA_TYPE=OTA_TYPE_BOOT
BIN_OTA	= $(OBJ_DIR)/$(PROJECT_NAME).bin
ADD_OPT = -w 0x2F00 -f ota_upboot.add
else
LDSCRIPT   ?= $(SDK_PATH)/misc/phy6222.ld
DEFINES += -DOTA_TYPE=OTA_TYPE_NONE
BIN_OTA	= $(OBJ_DIR)/$(PROJECT_NAME).bin
ADD_OPT =
endif

LDFLAGS    := $(ARCH_FLAGS)
LDFLAGS    += --static -nostartfiles -nostdlib
LDFLAGS    += -Wl,--gc-sections
LDFLAGS    += -Wl,--script=$(LDSCRIPT)
# LDFLAGS    += -Wl,--no-warn-rwx-segments
LDFLAGS    += -Wl,--just-symbols=$(SDK_PATH)/misc/bb_rom_sym_m0.gcc
LDFLAGS    += -Wl,-Map=$(OBJ_DIR)/$(PROJECT_NAME).map 
LIBS       += -Wl,--start-group -lgcc -lnosys -Wl,--end-group

##############################################################################
INCLUDES += -I$(SDK_PATH)/misc
INCLUDES += -I$(SDK_PATH)/misc/CMSIS/include
INCLUDES += -I$(SDK_PATH)/components/arch/cm0
INCLUDES += -I$(SDK_PATH)/components/ble/controller
INCLUDES += -I$(SDK_PATH)/components/ble/include
INCLUDES += -I$(SDK_PATH)/components/ble/hci
INCLUDES += -I$(SDK_PATH)/components/ble/host
INCLUDES += -I$(SDK_PATH)/components/inc
INCLUDES += -I$(SDK_PATH)/components/osal/include
INCLUDES += -I$(SDK_PATH)/components/common
INCLUDES += -I$(SDK_PATH)/components/profiles/ota_app
INCLUDES += -I$(SDK_PATH)/components/profiles/DevInfo
INCLUDES += -I$(SDK_PATH)/components/profiles/SimpleProfile
INCLUDES += -I$(SDK_PATH)/components/profiles/Roles
INCLUDES += -I$(SDK_PATH)/components/driver/adc
INCLUDES += -I$(SDK_PATH)/components/driver/bsp_button
INCLUDES += -I$(SDK_PATH)/components/driver/clock
INCLUDES += -I$(SDK_PATH)/components/driver/dma
INCLUDES += -I$(SDK_PATH)/components/driver/flash
INCLUDES += -I$(SDK_PATH)/components/driver/gpio
INCLUDES += -I$(SDK_PATH)/components/driver/i2c
INCLUDES += -I$(SDK_PATH)/components/driver/key
INCLUDES += -I$(SDK_PATH)/components/driver/kscan
INCLUDES += -I$(SDK_PATH)/components/driver/led_light
INCLUDES += -I$(SDK_PATH)/components/driver/log
INCLUDES += -I$(SDK_PATH)/components/driver/pwm
INCLUDES += -I$(SDK_PATH)/components/driver/pwrmgr
INCLUDES += -I$(SDK_PATH)/components/driver/qdec
INCLUDES += -I$(SDK_PATH)/components/driver/spi
INCLUDES += -I$(SDK_PATH)/components/driver/spiflash
INCLUDES += -I$(SDK_PATH)/components/driver/timer
INCLUDES += -I$(SDK_PATH)/components/driver/uart
INCLUDES += -I$(SDK_PATH)/components/driver/voice
INCLUDES += -I$(SDK_PATH)/components/driver/watchdog
INCLUDES += -I$(SDK_PATH)/components/libraries/crc16
INCLUDES += -I$(SDK_PATH)/components/libraries/cliface
#INCLUDES += -I$(SDK_PATH)/components/libraries/fs
INCLUDES += -I$(SDK_PATH)/components/driver/watchdog

###########################################
# LIBs
SRCS += $(SDK_PATH)/lib/rf/patch.c
SRCS += $(SDK_PATH)/lib/sec/phy_sec_ext.c
SRCS += $(SDK_PATH)/lib/sec/aes.c
SRCS += $(SDK_PATH)/lib/ble_host/att_client.c
SRCS += $(SDK_PATH)/lib/ble_host/att_server.c
SRCS += $(SDK_PATH)/lib/ble_host/att_util.c
SRCS += $(SDK_PATH)/lib/ble_host/gap_centdevmgr.c
SRCS += $(SDK_PATH)/lib/ble_host/gap_centlinkmgr.c
SRCS += $(SDK_PATH)/lib/ble_host/gap_configmgr.c
SRCS += $(SDK_PATH)/lib/ble_host/gap_devmgr.c
SRCS += $(SDK_PATH)/lib/ble_host/gap_linkmgr.c
SRCS += $(SDK_PATH)/lib/ble_host/gap_peridevmgr.c
SRCS += $(SDK_PATH)/lib/ble_host/gap_perilinkmgr.c
#SRCS += $(SDK_PATH)/lib/ble_host/gap_simpletask.c
SRCS += $(SDK_PATH)/lib/ble_host/gap_task.c
SRCS += $(SDK_PATH)/lib/ble_host/gatt_client.c
SRCS += $(SDK_PATH)/lib/ble_host/gatt_server.c
SRCS += $(SDK_PATH)/lib/ble_host/gatt_task.c
SRCS += $(SDK_PATH)/lib/ble_host/gatt_uuid.c
SRCS += $(SDK_PATH)/lib/ble_host/l2cap_if.c
SRCS += $(SDK_PATH)/lib/ble_host/l2cap_task.c
SRCS += $(SDK_PATH)/lib/ble_host/l2cap_util.c
SRCS += $(SDK_PATH)/lib/ble_host/linkdb.c
SRCS += $(SDK_PATH)/lib/ble_host/sm_intpairing.c
SRCS += $(SDK_PATH)/lib/ble_host/sm_mgr.c
SRCS += $(SDK_PATH)/lib/ble_host/sm_pairing.c
SRCS += $(SDK_PATH)/lib/ble_host/smp.c
SRCS += $(SDK_PATH)/lib/ble_host/sm_rsppairing.c
SRCS += $(SDK_PATH)/lib/ble_host/sm_task.c
ifdef LIB_BLE_ADD
SRCS += $(SDK_PATH)/lib/ble_controller/ll_common.c
SRCS += $(SDK_PATH)/lib/ble_controller/ll_enc.c
SRCS += $(SDK_PATH)/lib/ble_controller/ll_hw_drv.c
SRCS += $(SDK_PATH)/lib/ble_controller/ll_hwItf.c
SRCS += $(SDK_PATH)/lib/ble_controller/ll.c
SRCS += $(SDK_PATH)/lib/ble_controller/ll_masterEndCauses.c
SRCS += $(SDK_PATH)/lib/ble_controller/ll_slaveEndCauses.c
SRCS += $(SDK_PATH)/lib/ble_controller/ll_sleep.c
endif
SRCS += $(SDK_PATH)/lib/ble_controller/rf_phy_driver.c

###########################################
# SDK
SRCS += $(SDK_PATH)/components/driver/clock/clock.c
SRCS += $(SDK_PATH)/components/driver/flash/flash.c
SRCS += $(SDK_PATH)/components/driver/gpio/gpio.c
#SRCS += $(SDK_PATH)/components/driver/key/key.c
#SRCS += $(SDK_PATH)/components/driver/led_light/led_light.c
#SRCS += $(SDK_PATH)/components/driver/pwm/pwm.c
SRCS += $(SDK_PATH)/components/driver/pwrmgr/pwrmgr.c
SRCS += $(SDK_PATH)/components/driver/timer/timer.c
SRCS += $(SDK_PATH)/components/driver/uart/uart.c
SRCS += $(SDK_PATH)/components/driver/watchdog/watchdog.c
SRCS += $(SDK_PATH)/components/driver/log/my_printf.c

#SRCS += $(SDK_PATH)/components/profiles/Roles/central.c
#SRCS += $(SDK_PATH)/components/profiles/ota_app/ota_app_service.c
#SRCS += $(SDK_PATH)/components/profiles/Roles/peripheral.c
SRCS += $(SDK_PATH)/components/profiles/Roles/gap.c
SRCS += $(SDK_PATH)/components/profiles/Roles/gapbondmgr.c
SRCS += $(SDK_PATH)/components/profiles/Roles/gapgattserver.c
SRCS += $(SDK_PATH)/components/profiles/GATT/gattservapp.c
#SRCS += $(SDK_PATH)/components/profiles/DevInfo/devinfoservice.c

#SRCS += $(SDK_PATH)/components/osal/snv/osal_snv.c
#SRCS += $(SDK_PATH)/components/libraries/fs/fs.c

SRCS += $(SDK_PATH)/misc/jump_table.c

##############################################################################

INCLUDES += -I$(SDK_PATH)/misc/CMSIS/device/phyplus

STARTUP_ASM = $(SDK_PATH)/misc/CMSIS/device/phyplus/phy6222_start.s

SRCS += $(SDK_PATH)/misc/CMSIS/device/phyplus/phy6222_cstart.c
SRCS += $(SDK_PATH)/misc/CMSIS/device/phyplus/phy6222_vectors.c

##############################################################################

CFLAGS  += $(DEFINES) $(INCLUDES) 

#SRC_O = $(patsubst %.c,%.o,$(patsubst sdk/%, $(SDK_PATH)%, $(SRCS)))

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

%.bin: %.hex
	@echo Make: $@
	@$(PYTHON) ./phy62x2_ota.py $(ADD_OPT) $(OBJ_DIR)/$(PROJECT_NAME).hex

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

VPATH:=$(OBJ_DIR) $(SDK_PATH)
