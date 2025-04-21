#!/usr/bin/env python3

import sys
from pathlib import Path
import subprocess
import shutil
from typing import NoReturn

# Note: Modify your PATH to include the desired compiler before launching this Script!

SWVER = '_v21'

BINDIR = "bin"
BOOTDIR = "boot"

DEVICES = [
    'THB2',
    'BTH01',
    'TH05',
    'TH05D',
    'TH05F',
    'THB1',
    'THB3',
    'KEY2',
    'TH04',
    'HDP16'
]

def error_exit(message: str = "Error!") -> NoReturn:
    sys.exit(f"\033[0;31m{message}\033[0m")

def ensure_existing(file: Path) -> None:
    if not file.exists():
        error_exit(f'Expected file "{file}" does not exist.')

def make(*args, silent: bool = True) -> None:
    try:
        silent_arg = ('-s',) if silent else ()
        subprocess.check_call(['make', *silent_arg, *args], shell=False)
    except subprocess.CalledProcessError:
        error_exit()

if __name__ == "__main__":
    # Some santiy checks to prevent misuse
    if len(sys.argv) > 1:
        error_exit("Run this script without arguments to build the app/boot firmware for all supported devices.")

    for cmd in ('arm-none-eabi-gcc', 'make'):
        if shutil.which(cmd) is None:
            error_exit(f'Command "{cmd}" not found. Please install it and/or modify your PATH accordingly.')


    script_dir = Path(__file__).resolve().parent
    bin_dir = script_dir / BINDIR
    boot_dir = script_dir / BOOTDIR
    build_dir = script_dir / "build"

    # Prepare output directories:
    bin_dir.mkdir(parents=True, exist_ok=True)
    boot_dir.mkdir(parents=True, exist_ok=True)

    for device in DEVICES:
        print(f" --------- Compiling Application firmware for {device}")
        app_hex = build_dir / f'{device}{SWVER}.hex'
        app_bin = build_dir / f'{device}{SWVER}.bin'
        app_hex.unlink(missing_ok=True)
        app_bin.unlink(missing_ok=True)
        make('clean')
        make('-j', f'PROJECT_NAME={device}{SWVER}', f'PROJECT_DEF="-DDEVICE=DEVICE_{device}"')
        ensure_existing(app_bin)
        shutil.copy(app_bin, bin_dir)

        print(f" --------- Compiling OTA boot firmware for {device}")
        boot_hex = build_dir / f'BOOT_{device}{SWVER}.hex'
        boot_bin = build_dir / f'BOOT_{device}{SWVER}.bin'
        boot_hex.unlink(missing_ok=True)
        boot_bin.unlink(missing_ok=True)
        make('clean')
        make('-j', f'PROJECT_NAME=BOOT_{device}{SWVER}', 'BOOT_OTA=1', f'PROJECT_DEF="-DDEVICE=DEVICE_{device}"')
        ensure_existing(boot_hex)
        ensure_existing(boot_bin)
        shutil.copy(boot_hex, bin_dir)
        shutil.copy(boot_bin, boot_dir)

    print("✅ All Done ")