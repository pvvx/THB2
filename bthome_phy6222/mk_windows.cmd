@set PATH=D:\MCU\GNU_Tools_ARM_Embedded\13.2.rel1\bin;%PATH%
@set SWVER=_v08
@del /Q "build\THB2%SWVER%.hex"
@del /Q "build\THB2%SWVER%.bin"
@mkdir .\bin
@make -s clean
@make -s -j PROJECT_NAME=THB2%SWVER% POJECT_DEF="-DDEVICE=DEVICE_THB2"
@if not exist "build\THB2%SWVER%.hex" goto :error
@copy "build\THB2%SWVER%.bin" .\bin
@del /Q "build\BTH01%SWVER%.hex"
@del /Q "build\BTH01%SWVER%.bin"
@make -s clean
@make -s -j PROJECT_NAME=BTH01%SWVER% POJECT_DEF="-DDEVICE=DEVICE_BTH01"
@if not exist "build\BTH01%SWVER%.hex" goto :error
@copy "build\BTH01%SWVER%.bin" .\bin
@del /Q "build\BOOT_THB2%SWVER%.hex"
@make -s clean
@make -s -j PROJECT_NAME=BOOT_THB2%SWVER% BOOT_OTA=1 POJECT_DEF="-DDEVICE=DEVICE_THB2"
@if not exist "build\BOOT_THB2%SWVER%.hex" goto :error
@copy "build\BOOT_THB2%SWVER%.hex" .\bin
@del /Q "build\BOOT_BTH01%SWVER%.hex"
@make -s clean
@make -s -j PROJECT_NAME=BOOT_BTH01%SWVER% BOOT_OTA=1 POJECT_DEF="-DDEVICE=DEVICE_BTH01"
@if not exist "build\BOOT_BTH01%SWVER%.hex" goto :error
@copy "build\BOOT_BTH01%SWVER%.hex" .\bin
@del /Q "build\TH05%SWVER%.hex"
@del /Q "build\TH05%SWVER%.bin"
@mkdir .\bin
@make -s clean
@make -s -j PROJECT_NAME=TH05%SWVER% POJECT_DEF="-DDEVICE=DEVICE_TH05"
@if not exist "build\TH05%SWVER%.hex" goto :error
@copy "build\TH05%SWVER%.bin" .\bin
@del /Q "build\BOOT_TH05%SWVER%.hex"
@make -s clean
@make -s -j PROJECT_NAME=BOOT_TH05%SWVER% BOOT_OTA=1 POJECT_DEF="-DDEVICE=DEVICE_TH05"
@if not exist "build\BOOT_TH05%SWVER%.hex" goto :error
@copy "build\BOOT_TH05%SWVER%.hex" .\bin
@exit
:error
@echo "Error!" 