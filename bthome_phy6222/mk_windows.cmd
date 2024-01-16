@set PATH=D:\MCU\GNU_Tools_ARM_Embedded\13.2.rel1\bin;%PATH%
@set SWVER=_v06
@del /Q "THB2%SWVER%.hex"
@mkdir .\bin
@make -s clean
@make -s -j PROJECT_NAME=THB2%SWVER% POJECT_DEF="-DDEVICE=DEVICE_THB2"
@if not exist "build\THB2%SWVER%.hex" goto :error
@copy "build\THB2%SWVER%.hex" .\bin
@del /Q "build\BTH01%SWVER%.hex"
@make -s clean
@make -s -j PROJECT_NAME=BTH01%SWVER% POJECT_DEF="-DDEVICE=DEVICE_BTH01"
@if not exist "build\BTH01%SWVER%.hex" goto :error
@copy "build\BTH01%SWVER%.hex" .\bin
@exit
:error
@echo "Error!" 