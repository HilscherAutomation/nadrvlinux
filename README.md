# Overview

This repository provides the following components:

| component                                                 | license                                       | description
|:---------------------------------------------------------:|:---------------------------------------------:|:-------------:|
| [libnetana](libnetana/)                                   | [MIT](libnetana/COPYING)                      | User space driver for netANALYZER devices.
| [kernel driver](netanalyzer_kernel_mod/)                  | [GPLv-2-only](netanalyzer_kernel_mod/COPYING) | Kernel driver for netANALYZER devices.
|  - [netANALYZER Toolkit](netanalyzer_kernel_mod/toolkit/) | [MIT](netanalyzer_kernel_mod/toolkit/COPYING) | OS independant device handling abstraction of netX devices, used by kernel driver).
| [firmware](firmware/)                                     | [HSLA](LICENSE-HSLA)                          | netX Analyzer firmware binaries
| [bsl](bsl/)                                               | [HSLA](LICENSE-HSLA)                          | netX bootloader binaries
| [example](example/)                                       | [MIT](LICENSE-MIT)                            | Simple demo application.

# Compilation / Installation

## Quick Start

  1. Open a console window and enter the directory containing this README file
  2. run the installation script './build_install_driver' (root privileges
     required).
     The script will compile and install all required files (driver, library
     and firmware). To run only single installation steps see the following
     guidelines.

  3. To build the example application see step 4).

##  Driver installation step by step

  ---
  **NOTE**

  Make sure a netANALYZER/cifX Device is inserted into your device

  ---

  1. Build the netAnalyzer kernel module netana.ko
      - Enter directory ./netana_kernel_mod and run "make"
  2. Install the firmware and the kernel module (netana.ko)
      - Enter ./tools/ and run ./install_netanako install (root required)
      - Run ./install_firmware install (root required)
      - Run ./install_netanako update (root required)
      - Run ./install_netanako load (root required)
  3. Build the User Space library libnetana
      - Enter ./libnetana/ and run "./configure"
      - Run "make"
      - Run "make install" (root required)
  4. Build and run demo application
      - Enter ./example/ and run "make"
      - Start application ./netana_demo (root required)
