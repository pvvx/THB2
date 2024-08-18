@set PATH=D:\MCU\GNU_Tools_ARM_Embedded\13.2.rel1\bin;%PATH%
@set SWVER=_v18
@del /Q "build\THB2%SWVER%.hex"
@del /Q "build\THB2%SWVER%.bin"
@mkdir .\bin
@mkdir .\boot
@make -s clean
@make -s -j PROJECT_NAME=THB2%SWVER% POJECT_DEF="-DDEVICE=DEVICE_THB2"
@if not exist "build\THB2%SWVER%.hex" goto :error
@copy "build\THB2%SWVER%.bin" .\bin
@
@del /Q "build\BTH01%SWVER%.hex"
@del /Q "build\BTH01%SWVER%.bin"
@make -s clean
@make -s -j PROJECT_NAME=BTH01%SWVER% POJECT_DEF="-DDEVICE=DEVICE_BTH01"
@if not exist "build\BTH01%SWVER%.hex" goto :error
@copy "build\BTH01%SWVER%.bin" .\bin
@
@del /Q "build\TH05%SWVER%.hex"
@del /Q "build\TH05%SWVER%.bin"
@make -s clean
@make -s -j PROJECT_NAME=TH05%SWVER% POJECT_DEF="-DDEVICE=DEVICE_TH05"
@if not exist "build\TH05%SWVER%.hex" goto :error
@copy "build\TH05%SWVER%.bin" .\bin
@
@del /Q "build\TH05D%SWVER%.hex"
@del /Q "build\TH05D%SWVER%.bin"
@make -s clean
@make -s -j PROJECT_NAME=TH05D%SWVER% POJECT_DEF="-DDEVICE=DEVICE_TH05D"
@if not exist "build\TH05D%SWVER%.hex" goto :error
@copy "build\TH05D%SWVER%.bin" .\bin
@
@del /Q "build\TH05F%SWVER%.hex"
@del /Q "build\TH05F%SWVER%.bin"
@make -s clean
@make -s -j PROJECT_NAME=TH05F%SWVER% POJECT_DEF="-DDEVICE=DEVICE_TH05F"
@if not exist "build\TH05F%SWVER%.hex" goto :error
@copy "build\TH05F%SWVER%.bin" .\bin
@
@del /Q "build\THB1%SWVER%.hex"
@del /Q "build\THB1%SWVER%.bin"
@make -s clean
@make -s -j PROJECT_NAME=THB1%SWVER% POJECT_DEF="-DDEVICE=DEVICE_THB1"
@if not exist "build\THB1%SWVER%.hex" goto :error
@copy "build\THB1%SWVER%.bin" .\bin
@
@del /Q "build\THB3%SWVER%.hex"
@del /Q "build\THB3%SWVER%.bin"
@make -s clean
@make -s -j PROJECT_NAME=THB3%SWVER% POJECT_DEF="-DDEVICE=DEVICE_THB3"
@if not exist "build\THB3%SWVER%.hex" goto :error
@copy "build\THB3%SWVER%.bin" .\bin
@
@del /Q "build\BOOT_THB2%SWVER%.hex"
@make -s clean
@make -s -j PROJECT_NAME=BOOT_THB2%SWVER% BOOT_OTA=1 POJECT_DEF="-DDEVICE=DEVICE_THB2"
@if not exist "build\BOOT_THB2%SWVER%.hex" goto :error
@copy "build\BOOT_THB2%SWVER%.hex" .\bin
@copy "build\BOOT_THB2%SWVER%.bin" .\boot
@
@del /Q "build\BOOT_BTH01%SWVER%.hex"
@make -s clean
@make -s -j PROJECT_NAME=BOOT_BTH01%SWVER% BOOT_OTA=1 POJECT_DEF="-DDEVICE=DEVICE_BTH01"
@if not exist "build\BOOT_BTH01%SWVER%.hex" goto :error
@copy "build\BOOT_BTH01%SWVER%.hex" .\bin
@copy "build\BOOT_BTH01%SWVER%.bin" .\boot
@
@del /Q "build\BOOT_TH05%SWVER%.hex"
@make -s clean
@make -s -j PROJECT_NAME=BOOT_TH05%SWVER% BOOT_OTA=1 POJECT_DEF="-DDEVICE=DEVICE_TH05"
@if not exist "build\BOOT_TH05%SWVER%.hex" goto :error
@copy "build\BOOT_TH05%SWVER%.hex" .\bin
@copy "build\BOOT_TH05%SWVER%.bin" .\boot
@
@del /Q "build\BOOT_TH05D%SWVER%.hex"
@make -s clean
@make -s -j PROJECT_NAME=BOOT_TH05D%SWVER% BOOT_OTA=1 POJECT_DEF="-DDEVICE=DEVICE_TH05D"
@if not exist "build\BOOT_TH05D%SWVER%.hex" goto :error
@copy "build\BOOT_TH05D%SWVER%.hex" .\bin
@copy "build\BOOT_TH05D%SWVER%.bin" .\boot
@
@del /Q "build\BOOT_TH05F%SWVER%.hex"
@make -s clean
@make -s -j PROJECT_NAME=BOOT_TH05F%SWVER% BOOT_OTA=1 POJECT_DEF="-DDEVICE=DEVICE_TH05F"
@if not exist "build\BOOT_TH05F%SWVER%.hex" goto :error
@copy "build\BOOT_TH05F%SWVER%.hex" .\bin
@copy "build\BOOT_TH05F%SWVER%.bin" .\boot
@
@del /Q "build\BOOT_THB1%SWVER%.hex"
@make -s clean
@make -s -j PROJECT_NAME=BOOT_THB1%SWVER% BOOT_OTA=1 POJECT_DEF="-DDEVICE=DEVICE_THB1"
@if not exist "build\BOOT_THB1%SWVER%.hex" goto :error
@copy "build\BOOT_THB1%SWVER%.hex" .\bin
@copy "build\BOOT_THB1%SWVER%.bin" .\boot
@
@del /Q "build\BOOT_THB3%SWVER%.hex"
@make -s clean
@make -s -j PROJECT_NAME=BOOT_THB3%SWVER% BOOT_OTA=1 POJECT_DEF="-DDEVICE=DEVICE_THB3"
@if not exist "build\BOOT_THB3%SWVER%.hex" goto :error
@copy "build\BOOT_THB3%SWVER%.hex" .\bin
@copy "build\BOOT_THB3%SWVER%.bin" .\boot
@exit
:error
@echo "Error!" 