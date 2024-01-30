##############################################################################
PROJECT_NAME ?= bthome_phy6222
DEVICE     ?= DEVICE_THB2
OTA_TYPE   ?= 0
DEBUG_INFO ?= 0
# AUX_DEFS   ?= -DEXT_ADV_ENABLE 
PROJ_DEFS  ?= -DDEVICE=$(DEVICE) -DDEBUG_INFO=$(DEBUG_INFO)
##############################################################################
COM_PORT = COM11
##############################################################################
# Source
SRC_PATH:= ./source
SRC_PRJ := main.c
SRC_PRJ += battery.c
SRC_PRJ += battservice.c
SRC_PRJ += bthome_beacon.c
SRC_PRJ += osal_peripheral.c
SRC_PRJ += peripheral_main.c
SRC_PRJ += sbp_profile.c
SRC_PRJ += devinfoservice.c
SRC_PRJ += sensors.c
SRC_PRJ += dev_i2c.c
SRC_PRJ += config.c
SRC_PRJ += cmd_parser.c
SRC_PRJ += thb2_main.c
SRC_PRJ += thb2_peripheral.c
SRC_PRJ += thservice.c
SRC_PRJ += flash_eep.c
SRC_PRJ += lcd_th05.c
SRC_PRJ += ble_ota.c
SRC_PRJ += logger.c

INCLUDES:= -I$(SRC_PATH)

SRCS = $(addprefix $(SRC_PATH)/, $(SRC_PRJ))
##############################################################################
DEFINES += $(PROJ_DEFS)
DEFINES += -DMTU_SIZE=247
DEFINES += -DCFG_SLEEP_MODE=PWR_MODE_SLEEP
DEFINES += -DADV_NCONN_CFG=0x01
DEFINES += -DADV_CONN_CFG=0x02
DEFINES += -DSCAN_CFG=0x04
DEFINES += -DINIT_CFG=0x08
DEFINES += -DBROADCASTER_CFG=0x01
DEFINES += -DOBSERVER_CFG=0x02
DEFINES += -DPERIPHERAL_CFG=0x04
DEFINES += -DCENTRAL_CFG=0x08
DEFINES += -DHOST_CONFIG=0x4
DEFINES += -DARMCM0
DEFINES += -D_RTE_
DEFINES += -DCFG_CP
DEFINES += -DOSAL_CBTIMER_NUM_TASKS=1
DEFINES += -DHCI_TL_NONE=1
DEFINES += -DENABLE_LOG_ROM_=0
DEFINES += -D_OBJ_DIR_FOR_DTM_=0
DEFINES += -DDBG_ROM_MAIN=0
DEFINES += -DAPP_CFG=0
DEFINES += -DOSALMEM_METRICS=0
DEFINES += -DPHY_MCU_TYPE=MCU_BUMBEE_M0
DEFINES += -DDEF_GAPBOND_MGR_ENABLE=0
DEFINES += -DUSE_FS=0
DEFINES += -DMAX_NUM_LL_CONN=1
#DEFINES += -DXFLASH_HIGH_SPEED=1
##############################################################################
BIN_DIR := ./bin
OBJ_DIR := ./build
SDK     := ./SDK
PYTHON  := python3
GCC     ?= 
CC      := $(GCC)arm-none-eabi-gcc
OBJCOPY := $(GCC)arm-none-eabi-objcopy
OBJDUMP := $(GCC)arm-none-eabi-objdump
SIZE    := $(GCC)arm-none-eabi-size
READELF := $(GCC)arm-none-eabi-readelf
##############################################################################
ARCH_FLAGS := -mcpu=cortex-m0 -mthumb
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
CFLAGS     += -fms-extensions
CFLAGS     += -specs=nosys.specs
CFLAGS     += -Wl,--gc-sections

$(info DEVICE is [${DEVICE}])
$(info OTA TYPE is [${OTA_TYPE}])

ifeq ($(OTA_TYPE), 0)
DEFINES += -DOTA_TYPE=OTA_TYPE_NONE
LDSCRIPT?= $(SDK)/misc/phy62x2.ld
endif
ifeq ($(OTA_TYPE), 1)
DEFINES += -DOTA_TYPE=OTA_TYPE_BOOT
LDSCRIPT?= $(SDK)/misc/phy62x2_ota_boot.ld
BIN_OTA	:= $(OBJ_DIR)/$(PROJECT_NAME).bin
endif
ifeq ($(OTA_TYPE), 2)
DEFINES += -DOTA_TYPE=OTA_TYPE_APP
LDSCRIPT?= $(SDK)/misc/phy62x2_ota_app.ld
BIN_OTA	:= $(OBJ_DIR)/$(PROJECT_NAME).bin
endif
$(info $$LDSCRIPT is [${LDSCRIPT}])

LDFLAGS    := $(ARCH_FLAGS)
LDFLAGS    += --static -nostartfiles -nostdlib
LDFLAGS    += -Wl,--gc-sections
LDFLAGS    += -Wl,--script=$(LDSCRIPT)
# LDFLAGS    += -Wl,--no-warn-rwx-segments
LDFLAGS    += -Wl,--just-symbols=$(SDK)/misc/bb_rom_sym_m0.gcc
LDFLAGS    += -Wl,-Map=$(OBJ_DIR)/$(PROJECT_NAME).map 
LIBS       += -Wl,--start-group -lgcc -lnosys -Wl,--end-group

##############################################################################
INCLUDES += -I$(SDK)/misc
INCLUDES += -I$(SDK)/misc/CMSIS/include
INCLUDES += -I$(SDK)/components/arch/cm0
INCLUDES += -I$(SDK)/components/ble/controller
INCLUDES += -I$(SDK)/components/ble/include
INCLUDES += -I$(SDK)/components/ble/hci
INCLUDES += -I$(SDK)/components/ble/host
INCLUDES += -I$(SDK)/components/inc
INCLUDES += -I$(SDK)/components/osal/include
INCLUDES += -I$(SDK)/components/common
INCLUDES += -I$(SDK)/components/profiles/ota_app
INCLUDES += -I$(SDK)/components/profiles/DevInfo
INCLUDES += -I$(SDK)/components/profiles/SimpleProfile
INCLUDES += -I$(SDK)/components/profiles/Roles
INCLUDES += -I$(SDK)/components/driver/adc
INCLUDES += -I$(SDK)/components/driver/bsp_button
INCLUDES += -I$(SDK)/components/driver/clock
INCLUDES += -I$(SDK)/components/driver/dma
INCLUDES += -I$(SDK)/components/driver/flash
INCLUDES += -I$(SDK)/components/driver/gpio
INCLUDES += -I$(SDK)/components/driver/i2c
INCLUDES += -I$(SDK)/components/driver/key
INCLUDES += -I$(SDK)/components/driver/kscan
INCLUDES += -I$(SDK)/components/driver/led_light
INCLUDES += -I$(SDK)/components/driver/log
INCLUDES += -I$(SDK)/components/driver/pwm
INCLUDES += -I$(SDK)/components/driver/pwrmgr
INCLUDES += -I$(SDK)/components/driver/qdec
INCLUDES += -I$(SDK)/components/driver/spi
INCLUDES += -I$(SDK)/components/driver/spiflash
INCLUDES += -I$(SDK)/components/driver/timer
INCLUDES += -I$(SDK)/components/driver/uart
INCLUDES += -I$(SDK)/components/driver/voice
INCLUDES += -I$(SDK)/components/driver/watchdog
INCLUDES += -I$(SDK)/components/libraries/crc16
INCLUDES += -I$(SDK)/components/libraries/cliface
#INCLUDES += -I$(SDK)/components/libraries/fs
INCLUDES += -I$(SDK)/components/driver/watchdog

###########################################
# LIBs
SRCS += $(SDK)/lib/rf/patch.c
SRCS += $(SDK)/lib/sec/phy_sec_ext.c
SRCS += $(SDK)/lib/sec/aes.c
SRCS += $(SDK)/lib/ble_host/att_client.c
SRCS += $(SDK)/lib/ble_host/att_server.c
SRCS += $(SDK)/lib/ble_host/att_util.c
SRCS += $(SDK)/lib/ble_host/gap_centdevmgr.c
SRCS += $(SDK)/lib/ble_host/gap_centlinkmgr.c
SRCS += $(SDK)/lib/ble_host/gap_configmgr.c
SRCS += $(SDK)/lib/ble_host/gap_devmgr.c
SRCS += $(SDK)/lib/ble_host/gap_linkmgr.c
SRCS += $(SDK)/lib/ble_host/gap_peridevmgr.c
SRCS += $(SDK)/lib/ble_host/gap_perilinkmgr.c
SRCS += $(SDK)/lib/ble_host/gap_simpletask.c
SRCS += $(SDK)/lib/ble_host/gap_task.c
SRCS += $(SDK)/lib/ble_host/gatt_client.c
SRCS += $(SDK)/lib/ble_host/gatt_server.c
SRCS += $(SDK)/lib/ble_host/gatt_task.c
SRCS += $(SDK)/lib/ble_host/gatt_uuid.c
SRCS += $(SDK)/lib/ble_host/l2cap_if.c
SRCS += $(SDK)/lib/ble_host/l2cap_task.c
SRCS += $(SDK)/lib/ble_host/l2cap_util.c
SRCS += $(SDK)/lib/ble_host/linkdb.c
SRCS += $(SDK)/lib/ble_host/sm_intpairing.c
SRCS += $(SDK)/lib/ble_host/sm_mgr.c
SRCS += $(SDK)/lib/ble_host/sm_pairing.c
SRCS += $(SDK)/lib/ble_host/smp.c
SRCS += $(SDK)/lib/ble_host/sm_rsppairing.c
SRCS += $(SDK)/lib/ble_host/sm_task.c
ifdef LIB_BLE_ADD
SRCS += $(SDK)/lib/ble_controller/ll_common.c
SRCS += $(SDK)/lib/ble_controller/ll_enc.c
SRCS += $(SDK)/lib/ble_controller/ll_hw_drv.c
SRCS += $(SDK)/lib/ble_controller/ll_hwItf.c
SRCS += $(SDK)/lib/ble_controller/ll.c
SRCS += $(SDK)/lib/ble_controller/ll_masterEndCauses.c
SRCS += $(SDK)/lib/ble_controller/ll_slaveEndCauses.c
SRCS += $(SDK)/lib/ble_controller/ll_sleep.c
endif
SRCS += $(SDK)/lib/ble_controller/rf_phy_driver.c

###########################################
# SDK
SRCS += $(SDK)/components/driver/clock/clock.c
SRCS += $(SDK)/components/driver/flash/flash.c
SRCS += $(SDK)/components/driver/gpio/gpio.c
#SRCS += $(SDK)/components/driver/key/key.c
#SRCS += $(SDK)/components/driver/led_light/led_light.c
#SRCS += $(SDK)/components/driver/pwm/pwm.c
SRCS += $(SDK)/components/driver/pwrmgr/pwrmgr.c
SRCS += $(SDK)/components/driver/timer/timer.c
SRCS += $(SDK)/components/driver/uart/uart.c
SRCS += $(SDK)/components/driver/watchdog/watchdog.c
SRCS += $(SDK)/components/driver/log/my_printf.c

#SRCS += $(SDK)/components/profiles/Roles/central.c
#SRCS += $(SDK)/components/profiles/ota_app/ota_app_service.c
#SRCS += $(SDK)/components/profiles/Roles/peripheral.c
SRCS += $(SDK)/components/profiles/Roles/gap.c
SRCS += $(SDK)/components/profiles/Roles/gapbondmgr.c
SRCS += $(SDK)/components/profiles/Roles/gapgattserver.c
SRCS += $(SDK)/components/profiles/GATT/gattservapp.c
#SRCS += $(SDK)/components/profiles/DevInfo/devinfoservice.c

SRCS += $(SDK)/components/osal/snv/osal_snv.c
SRCS += $(SDK)/components/libraries/fs/fs.c

SRCS += $(SDK)/misc/jump_table.c

##############################################################################

INCLUDES += -I$(SDK)/misc/CMSIS/device/phyplus

STARTUP_ASM = $(SDK)/misc/CMSIS/device/phyplus/phy6222_start.s

SRCS += $(SDK)/misc/CMSIS/device/phyplus/phy6222_cstart.c
SRCS += $(SDK)/misc/CMSIS/device/phyplus/phy6222_vectors.c

##############################################################################

CFLAGS  += $(DEFINES) $(INCLUDES) 

#SRC_O = $(patsubst %.c,%.o,$(patsubst sdk/%, $(SDK)%, $(SRCS)))

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
	@$(PYTHON) ./phy62x2_ota.py $(OBJ_DIR)/$(PROJECT_NAME).hex

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

VPATH:=$(OBJ_DIR) $(SDK)
