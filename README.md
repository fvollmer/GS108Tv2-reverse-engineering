Netgear GS108Tv2 reverse engineering
=======================================

## Device Details
 * CPU: BCM53312 (BCM47xx/53xx MIPS Big Endian)
 * Flash: JS28F128P33T85 (128Mbit = 16MB)
 * RAM: NT5DS32M16BS (512Mbit = 64 MB)

## General Observations:
 * Stock firmware provides telnet access on port 60000. Login with username admin and your password
 * Heat sink is glued. Tremendous force is requiered to remove it. I was succesful by using a putty knife to pry it out. A sheet of paper was used to protect the pcb, but I still think this is a rather risky mehtod.
 * There is a 14 pin smd pin header. Most likely it is a JTAG port. I didn't try it out.
 * There is a 4 pin smd pin header for the serial port, but it is hidden under the heatsink. GND, RX, TX, VCC. Logic levels appear to be 3.3V. Use 9600 baud 8 bit, no parity and 1 stop bit for communication.
* Bootloader is CFE. You cancel boot by pressing ctrl+c and will get a promt.
* System is based on ecos

## Regarding booting OpenWRT:
 * You can do a network boot from the cfe bootloader (setup an tftp server):
   ```
   CFE> ifconfig eth0 -addr=192.168.0.10
   CFE> boot -tftp -elf 192.168.0.9:/vmlinux-initramfs.elf
   ```
   Or as a one-liner: `ifconfig eth0 -addr=192.168.0.10;boot -tftp -elf 192.168.0.9:/vmlinux-initramfs.elf`
 * The linux kernel isn't expecting any any bcm* device to be big endian. I crudely hacked openwrt (and the linux kernel) to use openwrt here: ToDo
 * To see any boot messages you have to enable early printk
 * The SSB bus isn't working. See the debug output here: ToDo
 
## Boot Initialization of the Stock Firmware
Netgear provides sources of the firmware here. I've placed them into a github repository: https://github.com/fvollmer/GS108Tv2-ecos-2.0. The mentioned SSB bus problem could be due to an initialization problem. So here a rough overview of the boot and initialization process:

ToDo

## Miscellaneous Stuff:
 * You can just edit the kernel files in the build directory and do a `make target/linux/install` to avoid recompiling everything. This way only the kernel is rebuild and a new image is created.

Related Links
 * https://openwrt.org/docs/guide-developer/add.new.device
 * https://openwrt.org/docs/guide-developer/hw.hacking.first.steps
 * https://openwrt.org/docs/guide-developer/adding_new_device
 * https://openwrt.org/docs/techref/hardware/soc/soc.broadcom.bcm47xx
 * https://openwrt.org/docs/techref/bootloader/cfe
 * https://kb.netgear.com/2649/NETGEAR-Open-Source-Code-for-Programmers-GPL
 * https://bcm-v4.sipsolutions.net/
