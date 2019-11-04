Netgear GS108Tv2 reverse engineering
=======================================

## Device Details
 * CPU: BCM53312 (BCM47xx/53xx MIPS Big Endian) ([product brief](53312s.pdf))
 * Flash: JS28F128P33T85 (128Mbit = 16MB)
 * RAM: NT5DS32M16BS (512Mbit = 64 MB)

## General Observations:
 * Stock firmware provides telnet access on port 60000. Login with username `admin` and your password
 * Heat sink is glued. Tremendous force is requiered to remove it. I was succesful by using a putty knife to pry it out. A sheet of paper was used to protect the pcb, but I still think this is a rather risky mehtod.
 * There is a 14 pin smd pin header. Most likely it is a JTAG port. I didn't try it out.
 * There is a 4 pin smd pin header for the serial port, but it is hidden under the heatsink. GND, RX, TX, VCC. Logic levels appear to be 3.3V. Use 9600 baud 8 bit, no parity and 1 stop bit for communication.
* Bootloader is CFE. You cancel boot by pressing ctrl+c and will get a promt.
* System is based on ecos
* [Boot log of the stock system](boot-log-stock)

## Regarding Booting OpenWRT:
I created a some hacks and patches that work around the some problems: https://github.com/fvollmer/GS108Tv2-openwrt. This is just  a very crude hack and definitly needs a complete rewrite. Nonetheless it allows to boot openwrt without crashing ([New boot log](boot-log-openwrt-hack)). I have yet to see what else is broken. It appears like the network isn't working, which is most likely due to the pci bus not working. There are some more (and better) patches needed for the ssb driver.

 * You can do a network boot from the cfe bootloader (setup an tftp server):
   ```
   CFE> ifconfig eth0 -addr=192.168.0.10
   CFE> boot -tftp -elf 192.168.0.9:/vmlinux-initramfs.elf
   ```
   Or as a one-liner: `ifconfig eth0 -addr=192.168.0.10;boot -tftp -elf 192.168.0.9:/vmlinux-initramfs.elf`
 * The linux kernel isn't expecting any any bcm* device to be big endian. I crudely hacked openwrt and the linux kernel
 * It may be useful to add earlycon to the boot parameters. This enables early printk and you might see something before the kernel crashes (not a problem anymore)
 * The linux kernel tries to scan the ssb bus. This won't work since all IDHIGH and IDLOW register are always 0. Further it tries to scan at invalid addresses. The debug output looks something like [this](boot-log-openwrt) (This log contains some additional debug statements and the ssb cores where limited to 4). The only thing that appears to be ok is the chip id. The original ecos operating systems appears to statically assign the cores.
 * The IRQ handling of the SSB driver is also broken for this device, the function `ssb_irqflag` tries to acces `SSB_TPSFLAG` and crashes the kernel.


## ecos

### Building the ecos Sources
The stock firmware is based on the gpl licensed ecos operating system. These sources are provided by netgear. After some minor modifications ([see commits at the github repository](https://github.com/fvollmer/GS108Tv2-ecos-2.0)) I was able to build the sources using an old toolchain (recent versions are broken). I updated the [build instruction](https://github.com/fvollmer/GS108Tv2-ecos-2.0/blob/master/README.raptor_netgear.txt) to make building easier.

### Reading the SSB Bus Registers from ecos
The device should be correctly initialized if we use the ecos source code. This means we can check if the ssb registers with an ecos applicaton. I build a [quick test](hello.c) and compiled it with the example. According to the [output](bootlog-ecos-ssb) the id high and low registers are 0. It looks like there simply isn't an id.

### Boot Initialization of the Stock Firmware
One idea for the mentioned SSB bus problem was that maybe there is some initialization wrong. Therefore I took a closer look at the boot an initialization process. This depends on the loaded packages. The configuration file should be [`mips_raptor_netgear.ecc`](https://github.com/fvollmer/GS108Tv2-ecos-2.0/blob/master/mips_raptor_netgear.ecc). The most important packages seem to be `HAL_MIPS`, `HAL_MIPS_BCM47xx` and `HAL_MIPS_BCM953710`. 

A rough overview of the boot and initialization process:
 * `_start`                        at [`packages/hal/mips/arch/v2_0/src/vectors.S`](https://github.com/fvollmer/GS108Tv2-ecos-2.0/blob/master/packages/hal/mips/arch/v2_0/src/vectors.S#L168)
	* `hal_cpu_init`                at [`packages/hal/mips/arch/v2_0/include/arch.inc`](https://github.com/fvollmer/GS108Tv2-ecos-2.0/blob/master/packages/hal/mips/arch/v2_0/include/arch.inc#L187)
	* `hal_diag_init`               at [`packages/hal/mips/bcm953710/v2_0/src/hal_diag.c`](https://github.com/fvollmer/GS108Tv2-ecos-2.0/blob/master/packages/hal/mips/bcm953710/v2_0/src/hal_diag.c#L88)
	* `hal_mmu_init`                at [`packages/hal/mips/arch/v2_0/include/arch.inc`](https://github.com/fvollmer/GS108Tv2-ecos-2.0/blob/master/packages/hal/mips/arch/v2_0/include/arch.inc)
	* `hal_fpu_init`                at [`packages/hal/mips/arch/v2_0/include/arch.inc`](https://github.com/fvollmer/GS108Tv2-ecos-2.0/blob/master/packages/hal/mips/arch/v2_0/include/arch.inc#L592)
	* `hal_memc_init`               at [`packages/hal/mips/bcm953710/v2_0/include/platform.inc`](https://github.com/fvollmer/GS108Tv2-ecos-2.0/blob/master/packages/hal/mips/bcm953710/v2_0/include/platform.inc#L200)
		* `board_draminit`           at [`packages/hal/mips/bcm953710/v2_0/src/sbsdram.S`](https://github.com/fvollmer/GS108Tv2-ecos-2.0/blob/master/packages/hal/mips/bcm953710/v2_0/src/sbsdram.S#L156)
	* `hal_cache_init`              at [`packages/hal/mips/bcm47xx/v2_0/include/variant.inc`](https://github.com/fvollmer/GS108Tv2-ecos-2.0/blob/master/packages/hal/mips/bcm47xx/v2_0/include/variant.inc#L124)
	* `hal_timer_init`              at [`packages/hal/mips/arch/v2_0/include/arch.inc`](https://github.com/fvollmer/GS108Tv2-ecos-2.0/blob/master/packages/hal/mips/arch/v2_0/include/arch.inc#L813)
	* `hal_variant_init`            at [`packages/hal/mips/bcm47xx/v2_0/src/var_misc.c`](https://github.com/fvollmer/GS108Tv2-ecos-2.0/blob/master/packages/hal/mips/bcm47xx/v2_0/src/var_misc.c)
	* `hal_platform_init`         at [`packages/hal/mips/bcm953710/v2_0/src/plf_misc.c`](https://github.com/fvollmer/GS108Tv2-ecos-2.0/blob/master/packages/hal/mips/bcm953710/v2_0/src/plf_misc.c#L106)
		* `sb_kattach` at [`packages/hal/mips/bcm953710/v2_0/src/sbutils.c`](https://github.com/fvollmer/GS108Tv2-ecos-2.0/blob/master/packages/hal/mips/bcm953710/v2_0/src/sbutils.c#L152)
			* `sb_doattach` at [`packages/hal/mips/bcm953710/v2_0/src/sbutils.c`](https://github.com/fvollmer/GS108Tv2-ecos-2.0/blob/master/packages/hal/mips/bcm953710/v2_0/src/sbutils.c#L167)
		* `sb_mips_init` at [`packages/hal/mips/bcm953710/v2_0/src/sbmips.c`](https://github.com/fvollmer/GS108Tv2-ecos-2.0/blob/master/packages/hal/mips/bcm953710/v2_0/src/sbmips.c#L385])

Especially interesting appear `board_draminit` and `hal_platform_init`. The ecos system appears to allocate the cores staticallly at `sb_doattach`.

## Miscellaneous Stuff:
 * You can just edit the kernel files in the build directory and do a `make target/linux/install` to avoid recompiling everything. This way only the kernel is rebuild and a new image is created.

## The Excetion handler
The default exception handler looks like this:
```
**Exception 32: EPC=8027A97C, Cause=0000001C (BusErrD  )
                RA=8027D8F0, VAddr=00000000

        0  ($00) = 00000000     AT ($01) = 10000000
        v0 ($02) = B8000000     v1 ($03) = 806421E4
        a0 ($04) = 8064223C     a1 ($05) = B8005F18
        a2 ($06) = 00000000     a3 ($07) = 000006E0
        t0 ($08) = 00000000     t1 ($09) = 00000000
        t2 ($10) = 00000007     t3 ($11) = 00000000
        t4 ($12) = 0000002C     t5 ($13) = 804B0000
        t6 ($14) = 00000000     t7 ($15) = 00000000
        s0 ($16) = 8064223C     s1 ($17) = 8064223C
        s2 ($18) = 806421E4     s3 ($19) = 00000006
        s4 ($20) = 00000001     s5 ($21) = 00000001
        s6 ($22) = 8064223C     s7 ($23) = 804785C0
        t8 ($24) = 00000002     t9 ($25) = 00000000
        k0 ($26) = 804A0000     k1 ($27) = 804B0000
        gp ($28) = 804A4000     sp ($29) = 804A5D38
        fp ($30) = 80642218     ra ($31) = 8027D8F0
```
To better undertand the meaning of the registers please see [MIPS calling convention](https://en.wikipedia.org/wiki/Calling_convention#MIPS). First four call arguments in  $a0-$a3, return argument(s) in $v0-$v1.

EPC is the error programm counter, and RA is the return address. Both can be used with gdb to get information about the crash:
```
gdb ./build_dir/target-mips_mips32_musl/linux-brcm47xx_generic/vmlinux.debug 
GNU gdb (Ubuntu 8.2.91.20190405-0ubuntu3) 8.2.91.20190405-git
Copyright (C) 2019 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.
Type "show copying" and "show warranty" for details.
This GDB was configured as "x86_64-linux-gnu".
Type "show configuration" for configuration details.
For bug reporting instructions, please see:
<http://www.gnu.org/software/gdb/bugs/>.
Find the GDB manual and other documentation resources online at:
    <http://www.gnu.org/software/gdb/documentation/>.

For help, type "help".
Type "apropos word" to search for commands related to "word"...
Reading symbols from ./build_dir/target-mips_mips32_musl/linux-brcm47xx_generic/vmlinux.debug...
(gdb) list *0x8027A97C
0x8027a97c is in ssb_host_soc_read32 (./arch/mips/include/asm/io.h:434).
429	__BUILD_MEMORY_PFX(, bwlq, type)					\
430	__BUILD_MEMORY_PFX(__mem_, bwlq, type)					\
431	
432	BUILDIO_MEM(b, u8)
433	BUILDIO_MEM(w, u16)
434	BUILDIO_MEM(l, u32)
435	BUILDIO_MEM(q, u64)
436	
437	#define __BUILD_IOPORT_PFX(bus, bwlq, type)				\
438		__BUILD_IOPORT_SINGLE(bus, bwlq, type, ,)			\
(gdb) list *0x8027D8F0
0x8027d8f0 is in ssb_sflash_cmd (./include/linux/ssb/ssb.h:599).
594	{
595		dev->ops->write16(dev, offset, value);
596	}
597	static inline void ssb_write32(struct ssb_device *dev, u16 offset, u32 value)
598	{
599		dev->ops->write32(dev, offset, value);
600	}
601	#ifdef CONFIG_SSB_BLOCKIO
602	static inline void ssb_block_read(struct ssb_device *dev, void *buffer,
603					  size_t count, u16 offset, u8 reg_width)
(gdb) 
```

## Related Devices
The GS700TR appears to be especially interesting device. The [source code release](https://www.downloads.netgear.com/files/GPL/GS7XXTR_V3.0.1_src.zip.zip) contains linux sources for the `bcm56218`. These sources are very similar to the ecos sources and the ssb core mapping appears to be the same. Needs further investigation.

## Special Thanks
Thanks for the useful advices at the #openwrt-devel IRC channel. Especially to KanjiMonster who helped me with informations regarding the MIPS architecture and the SSB bus. The GS700TR sources were als discovered by KanjiMonster. 

## Related Links
 * https://openwrt.org/docs/guide-developer/add.new.device
 * https://openwrt.org/docs/guide-developer/hw.hacking.first.steps
 * https://openwrt.org/docs/guide-developer/adding_new_device
 * https://openwrt.org/docs/techref/hardware/soc/soc.broadcom.bcm47xx
 * https://openwrt.org/docs/techref/bootloader/cfe
 * https://kb.netgear.com/2649/NETGEAR-Open-Source-Code-for-Programmers-GPL
 * https://bcm-v4.sipsolutions.net/
 * https://en.wikipedia.org/wiki/Calling_convention#MIPS
