# 
# 
# Author: Seeger Chin
# e-mail: seeger.chin@gmail.com
# 
# Copyright (C) 2006 Ingenic Semiconductor Inc.
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
# 
# 

# select which module to compile, set value to 1 for the module to compile, remain blank for the module not to compile
# -----------------------------
#JZ4740_PAV = 1
#UCOS	= 1
WDT	= 1
#
#TOUCHTYPE :
#           0. No Touch driver
#           1. AK4182
#           2. JZ4740SADC
#
TOUCHTYPE  = 2

#LCDTYPE :
#           0. No LCD Driver
#           1. Samsung tft 480 * 272
#           2. FOXCONN 8-bit serial
#           3. slcd
#           4. AUO_A030FL01_V1
#LCDTYPE    = 1
LCDTYPE    = 0

#SLCDTYPE :
#           0. No SLCD Driver
#           1. LGDP4551
SLCDTYPE    = 0

#RTCTYPE :
#           0. No RTC Driver
#           1. jz4740RTC
RTCTYPE    = 1

#CODECTYPE :
#           0. No Codec Driver
#           1. jz4740 Codec
#	    2.play wav from memory
CODECTYPE = 1
WAVDECODE = 2

#KeyCTYPE :
#           0. No Key Driver
#           1. zk key
#	    2. zk key with 74hc164
#KEYTYPE = 1
KEYTYPE = 2

#TouchPanelTYPE :
#           0. No Tpanel Driver
#           1. jz4740 Tpanel
TPANELTYPE = 0

MMC    = 0
#MMCTYPE :
#           1. SD card
#           2. Micro -SD card
MMCTYPE    = 0

USBDISK	= 1

DEVTYPE = 1
#       1 h1plus
#       2 H10 or S50

#MACHINE_ID :
#       1: H1
#       2: K18
MACHINE_ID = 2

DRAM_SIZE = 8
#DRAM_SIZE = 16
CAMERA = 1
I2C = 1
LCM	= 1
NO_YAFFS1   = 1
YAFFS2   = 0
YAFFS1   = 1
FAT32	 = 1
FIRMWARE = 1
#-------------------
LWIP   = 0
UCGUI  = 0
UCFS   = 0
JPEG   = 0
UDC    = 1
MP3    = 0
UDCCPU = 0
GUIDEMO = 0
DM = 0
FM = 0
FMTYPE =0
USE_MIDWARE = 0
NAND = 1
# ------------------------------

CC	:= mipsel-linux-gcc
AR	:= mipsel-linux-ar rcsv
LD	:= mipsel-linux-ld
OBJCOPY	:= mipsel-linux-objcopy
NM	:= mipsel-linux-nm
OBJDUMP	:= mipsel-linux-objdump

CFLAGS	:= -mips32 -O2 -mno-abicalls -fno-builtin \
	   -fno-exceptions -ffunction-sections \
	   -fomit-frame-pointer -Wall
CFLAGS +=      -DCONFIG_YAFFS_DIRECT -DCONFIG_YAFFS_SHORT_NAMES_IN_RAM \
		-DCONFIG_YAFFS_PROVIDE_DEFS -DCONFIG_YAFFS_YAFFS2\
		-DCONFIG_YAFFSFS_PROVIDE_VALUES -DNO_Y_INLINE\
		 -DDRAM_SIZE=$(DRAM_SIZE) -DDEVTYPE=$(DEVTYPE) \
		 -DMACHINE_ID=$(MACHINE_ID)
#CFLAGS +=      -DCONFIG_YAFFS_DIRECT -DCONFIG_YAFFS_SHORT_NAMES_IN_RAM -DCONFIG_YAFFS_YAFFS2 \
		-DCONFIG_YAFFS_NO_YAFFS1 -DCONFIG_YAFFS_PROVIDE_DEFS \
		-DCONFIG_YAFFSFS_PROVIDE_VALUES -DNO_Y_INLINE
#	   -fomit-frame-pointer -msoft-float -G 0 -Werror

LIBC	:= ./firmware/jz_7131/libc.a #-lc
LIBS	:= -lstdc++ $(LIBC) -lm -lgcc

TOP	:= ..
COREDIR	 := $(TOP)/jz4740/core
OSDIR	 := $(TOP)/src
ARCHDIR	 := $(TOP)/mips
SOCDIR	 := $(TOP)/jz4740
MMCDIR	 := $(TOP)/mmc
USBDIR	 := $(TOP)/ch376
UCFSDIR  := $(TOP)/ucfs
LWIPDIR  := $(TOP)/lwip/src
UCGUIDIR := $(TOP)/ucgui
CIMDIR	 := $(TOP)/jz4740/drv/camera
CORE     := $(TOP)/ucgui/Core
TOUCHDIR := $(TOP)/jz4740/drv/touch
UDCDIR   := $(TOP)/jz4740/drv/udc
#UDCDIR   := $(TOP)/jz4740/udc_new
LCDDIR   := $(TOP)/jz4740/drv/lcd
SLCDDIR  := $(TOP)/jz4740/drv/slcd
LCMDIR	 := $(TOP)/jz4740/drv/lcm
RTCDIR   := $(TOP)/jz4740/drv/rtc
CODECDIR := $(TOP)/jz4740/drv/codec
WAVDIR   := $(TOP)/audio/wave
WAVMEMDIR   := $(TOP)/audio/wave_mem
KEYDIR   := $(TOP)/jz4740/drv/key
TPANELDIR	:= $(TOP)/jz4740/drv/tpanel
I2CDIR	 := $(TOP)/jz4740/drv/i2c
MP3DIR   := $(TOP)/madplay/libmad-0.15.1b
ID3DIR   := $(TOP)/madplay/libid3tag-0.15.1b
MADDIR   := $(TOP)/madplay/madplay-0.15.2b
FMDIR	 := $(TOP)/jz4740/drv/fm

NANDDIR  := $(TOP)/jz4740/nand
NAND512DIR  := $(TOP)/jz4740/nand512
UDCCPUDIR := $(TOP)/jz4740/udc_new
MIDWAREDIR := $(TOP)/midware
YAFFSDIR := $(TOP)/yaffs
FAT32DIR := $(TOP)/fat32
FIRMWAREDIR:= $(TOP)/jz4740/firmware

LIBDIR	:= $(TOP)/LIBS
ifeq ($(GUIDEMO),1)
LIBS += $(wildcard $(LIBDIR)/*.a)
endif
#CFLAGS  += -DJZ4740_PAV=$(JZ4740_PAV)
#CFLAGS  += -DUCOS=$(UCOS)
CFLAGS  += -DWDT=$(WDT)
CFLAGS  += -DZEM300
#CFLAGS  += -DJZRTC

#CFLAGS += -DCAMERA=$(CAMERA) \
          -DUCGUI=$(UCGUI)   \
          -DMMC=$(MMC)       \
          -DTOUCH=$(TOUCHTYPE)\
	  -DMMCTYPE=$(MMCTYPE)
LINKXN := link_$(DRAM_SIZE)M.xn

include  $(COREDIR)/core.mk

ifeq ($(UDCCPU),1)
include  $(UDCCPUDIR)/udc.mk
endif

ifeq ($(NAND),2)
include  $(NANDDIR)/nand.mk
endif

ifeq ($(NAND512),1)
include  $(NAND512DIR)/nand.mk
endif

ifneq ($(KEYTYPE),0)
include $(KEYDIR)/key.mk
endif

ifneq ($(TPANELTYPE),0)
include $(TPANELDIR)/tpanel.mk
endif

ifeq ($(WAVDECODE),1)
include $(WAVDIR)/wave.mk
endif

ifeq ($(WAVDECODE),2)
include $(WAVMEMDIR)/wave.mk
CFLAGS += -DPLAYSOUNDMEM
endif

ifneq ($(CODECTYPE),0)
include $(CODECDIR)/codec.mk
endif

ifneq ($(RTCTYPE),0)
include  $(RTCDIR)/rtc.mk
endif

ifneq ($(LCDTYPE),0)
include $(LCDDIR)/lcd.mk
endif

ifneq ($(SLCDTYPE),0)
include $(SLCDDIR)/slcd.mk
endif

ifeq ($(LCM),1)
include $(LCMDIR)/lcm.mk
endif

ifneq ($(TOUCHTYPE),0)
include $(TOUCHDIR)/touch.mk
endif

ifeq ($(MMC),1)
include $(MMCDIR)/mmc.mk
endif

ifeq ($(MACHINE_ID),2)
include $(FAT32DIR)/fat32.mk
include $(USBDIR)/ch376.mk
endif

ifeq ($(UCFS),1)
include $(UCFSDIR)/ucfs.mk
endif

ifeq ($(LWIP),1)
include $(LWIPDIR)/lwip.mk
endif

ifeq ($(UCGUI),1)
include $(UCGUIDIR)/ucgui.mk
endif

ifeq ($(USE_MIDWARE),1)
include $(MIDWAREDIR)/midware.mk
CFLAGS	+= -DUSE_MIDWARE=$(USE_MIDWARE)
endif

ifeq ($(CAMERA),1)
include $(CIMDIR)/camera.mk
endif

ifeq ($(YAFFS1),1)
SOURCES += $(wildcard $(YAFFSDIR)/yaffs1/*.c)
SOURCES += $(wildcard $(YAFFSDIR)/mem/*.c)
SOURCES += $(wildcard $(YAFFSDIR)/nand_flash/*.c)
CFLAGS += -I$(YAFFSDIR)/yaffs1
CFLAGS += -I$(YAFFSDIR)/mem
CFLAGS += -I$(YAFFSDIR)/nand_flash
VPATH  += $(YAFFSDIR)/yaffs1
VPATH  += $(YAFFSDIR)/mem
VPATH  += $(YAFFSDIR)/nand_flash
CFLAGS += -DYAFFS1
include  $(NAND512DIR)/nand.mk
endif

ifeq ($(YAFFS2),1)
SOURCES += $(wildcard $(YAFFSDIR)/yaffs2/*.c)
SOURCES += $(wildcard $(YAFFSDIR)/mem/*.c)
SOURCES += $(wildcard $(YAFFSDIR)/nand_flash/*.c)
CFLAGS += -I$(YAFFSDIR)/yaffs2
CFLAGS += -I$(YAFFSDIR)/mem
CFLAGS += -I$(YAFFSDIR)/nand_flash
VPATH  += $(YAFFSDIR)/yaffs2
VPATH  += $(YAFFSDIR)/mem
VPATH  += $(YAFFSDIR)/nand_flash
ifeq ($(NO_YAFFS1),1)
CFLAGS += -DCONFIG_YAFFS_NO_YAFFS1
include  $(NANDDIR)/nand.mk
else
include  $(NAND512DIR)/nand.mk
endif
endif

ifeq ($(FIRMWARE),1)
include $(FIRMWAREDIR)/firmware.mk
endif

ifeq ($(JPEG),1)
SOURCES	+= $(wildcard $(UCGUIDIR)/JPEG/*.c)
CFLAGS += -I$(UCGUIDIR)/JPEG
VPATH  += $(UCGUIDIR)/JPEG
endif

ifeq ($(UDC),1)
CFLAGS += -DUDC
include $(UDCDIR)/udc.mk
endif

ifeq ($(MP3),1)
include $(wildcard $(MP3DIR)/mp3.mk)
endif

ifeq ($(I2C),1)
include $(I2CDIR)/i2c.mk
endif

ifeq ($(FM),1)
include $(FMDIR)/fm.mk
endif

OBJS	:= $(addsuffix .o , $(basename $(notdir $(SOURCES))))
HEADO	:= $(addsuffix .o , $(basename $(notdir $(HEADS))))

APP	:= ucos.elf

all:	$(APP)
	$(OBJCOPY) -O binary $(APP) ucos.bin
	$(OBJDUMP) -d $(APP) > ucos.dump
	$(NM) $(APP) | sort > ucos.sym
	$(OBJDUMP) -h $(APP) > ucos.map

#	tftp 192.168.1.44 -c put ucos.bin
	cp -f ucos.bin   /E/tools/h1移交测试_8M

$(APP):	$(HEADO) $(OBJS) $(EXTLIBS) $(LINKXN) makefile
	$(CC) --static -nostdlib -T $(LINKXN) -o $@ $(HEADO) $(OBJS) $(EXTLIBS) $(FIRMWAREDIR)/jz_7131/mipsel-linux-libzkfp.ucos.a $(LIBS) $(DRVOBJ) $(LIBC) $(LIBS) -lgcc -lm $(FIRMWAREDIR)/jz_7131/libzkfp.a

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<
.cpp.o:
	$(CC) $(CFLAGS) -fno-rtti -fvtable-gc -o $@ -c $<
.S.o:
	$(CC) $(CFLAGS) -D_ASSEMBLER_ -D__ASSEMBLY__ -o $@ -c $<

clean:
	rm -fr *.o $(APP) $(OBJS) $(OTHER) *.sym *.map *.dump *.bin *.lib

