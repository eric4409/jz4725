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
JZ4740_PAVO = 0
JZ4740_LYRA = 1
LOADCOMMONDATA = 0x80FFF200
#
#TOUCHTYPE :
#           0. No Touch driver
#           1. AK4182
#           2. JZ4740SADC
#
TOUCHTYPE  = 0

#LCDTYPE :
#           0. No LCD Driver
#           1. Samsung tft 480 * 272
LCDTYPE    = 0

#SLCDTYPE :
#           0. No SLCD Driver
#           1. LGDP4551
#	    2. SPFD5420A
SLCDTYPE    = 0

FRAME_BUFF = 0x80F38000
#RTCTYPE :
#           0. No RTC Driver
#           1. jz4740RTC
RTCTYPE    = 1

#CODECTYPE :
#           0. No Codec Driver
#           1. jz4740 Codec
CODECTYPE = 0
WAVDECODE = 0
#KeyCTYPE :
#           0. No Key Driver
#           1. jz4740 Key
KEYTYPE = 1

CAMERA = 0
LWIP   = 0
UCGUI  = 0
UCFS   = 0
MMC    = 0
JPEG   = 0
UDC    = 0
MP3    = 0
NAND   = 0
UDCCPU = 0
GUIDEMO = 0
DM = 0
I2C = 1
USE_MIDWARE = 0
# ------------------------------

CC	:= mipsel-linux-gcc
AR	:= mipsel-linux-ar rcsv
LD	:= mipsel-linux-ld
OBJCOPY	:= mipsel-linux-objcopy
NM	:= mipsel-linux-nm
OBJDUMP	:= mipsel-linux-objdump

CFLAGS	:= -mips32 -O2 -mno-abicalls -fno-pic -fno-builtin \
	   -fno-exceptions -ffunction-sections \
	   -fomit-frame-pointer -msoft-float -G 0

LIBS	:= -lstdc++ -lc -lm -lgcc

TOP	:= ../..
COREDIR	 := $(TOP)/jz4740/core
OSDIR	 := $(TOP)/src
ARCHDIR	 := $(TOP)/mips
SOCDIR	 := $(TOP)/jz4740
MMCDIR	 := $(TOP)/mmc
UCFSDIR  := $(TOP)/ucfs
LWIPDIR  := $(TOP)/lwip/src
UCGUIDIR := $(TOP)/ucgui
CORE     := $(TOP)/ucgui/Core
TOUCHDIR := $(TOP)/jz4740/drv/touch
UDCDIR   := $(TOP)/jz4740/udc_new
LCDDIR   := $(TOP)/jz4740/drv/lcd
RTCDIR   := $(TOP)/jz4740/drv/rtc
I2CDIR   := $(TOP)/jz4740/drv/i2c
CODECDIR := $(TOP)/jz4740/drv/codec
WAVDIR   := $(TOP)/audio/wave
KEYDIR   := $(TOP)/jz4740/drv/key
MP3DIR   := $(TOP)/madplay/libmad-0.15.1b
ID3DIR   := $(TOP)/madplay/libid3tag-0.15.1b
NANDDIR  := $(TOP)/jz4740/nand
UDCCPUDIR := $(TOP)/jz4740/udc
LOADDDIR := $(TOP)/mobile_tv/loader
LIBDIR	:= $(TOP)/LIBS
MIDWAREDIR := $(TOP)/midware
ifeq ($(GUIDEMO),1)
LIBS += $(wildcard $(LIBDIR)/*.a)
endif

ifeq ($(JZ4740_PAVO),1)
CFLAGS  += -DJZ4740_PAVO=$(JZ4740_PAVO)
endif
ifeq ($(JZ4740_LYRA),1)
CFLAGS += -DJZ4740_LYRA=$(JZ4740_LYRA)
LCDTYPE = 4
MMCTYPE = 2
endif

CFLAGS += -DCAMERA=$(CAMERA) \
          -DUCGUI=$(UCGUI)   \
          -DUCFS=$(UCFS)     \
          -DMMC=$(MMC)       \
          -DJPEG=$(JPEG)     \
          -DTOUCH=$(TOUCHTYPE)


SOURCES := $(SOCDIR)/cache.c $(SOCDIR)/clock.c $(SOCDIR)/init.c \
	   $(SOCDIR)/timer.c $(SOCDIR)/intc.c \
	   $(SOCDIR)/uart.c $(SOCDIR)/dma.c $(SOCDIR)/gpio.c \
	   $(SOCDIR)/dm.c $(SOCDIR)/pm.c $(SOCDIR)/excpt.c \
	   $(SOCDIR)/DrvMemMgr.c $(SOCDIR)/loadcommon.c

SOURCES	+= $(wildcard $(ARCHDIR)/*.c) \
	   $(OSDIR)/ucos_ii.c

SOURCES	+= $(wildcard $(ARCHDIR)/*.S)
SOURCES	+= $(wildcard $(LOADDDIR)/*.c)
SOURCES	+= $(NANDDIR)/jz_nand.c

CFLAGS  += -DJZ4740_PAV=$(JZ4740_PAV) -I$(LOADDDIR)
CFLAGS  +=-DFRAME_BUFF_ADDRESS=$(FRAME_BUFF)
CFLAGS  +=-DLOAD_DATA_ADDRESS=$(LOADCOMMONDATA)
CFLAGS	+= -I$(OSDIR) -I$(SOCDIR)/include -I$(ARCHDIR) -I$(SOCDIR)
CFLAGS	+= -I$(NANDDIR)

VPATH	:= $(ARCHDIR) $(SOCDIR) $(OSDIR) $(LOADDDIR) $(NANDDIR)

ifeq ($(UDCCPU),1)
include  $(UDCCPUDIR)/udc.mk
endif

ifeq ($(UCFS),1)
include $(UCFSDIR)/ucfs.mk
endif

ifeq ($(NAND),1)
include  $(NANDDIR)/nand.mk
endif


ifneq ($(KEYTYPE),0)
include $(KEYDIR)/key.mk
endif

ifeq ($(WAVDECODE),1)
include $(WAVDIR)/wave.mk
endif

ifneq ($(CODECTYPE),0)
include $(CODECDIR)/codec.mk
endif

ifneq ($(RTCTYPE),0)
include  $(RTCDIR)/rtc.mk
endif

ifeq ($(I2C),1)
include $(I2CDIR)/i2c.mk
endif

ifneq ($(LCDTYPE),0)
include $(LCDDIR)/lcd.mk
endif

ifneq ($(TOUCHTYPE),0)
include $(TOUCHDIR)/touch.mk
endif

ifeq ($(MMC),1)
include $(MMCDIR)/mmc.mk
endif

ifeq ($(LWIP),1)
include $(LWIPDIR)/lwip.mk
endif

ifeq ($(UCGUI),1)
include $(UCGUIDIR)/ucgui.mk
endif

ifeq ($(USE_MIDWARE),1)
include $(MIDWAREDIR)/midware.mk
CFLAGS  += -DUSE_MIDWARE=$(USE_MIDWARE)
endif

ifeq ($(CAMERA),1)
#SOURCES	+= $(wildcard $(SOCDIR)/camera/*.c) $(SOCDIR)/drv/i2c.c 
I2C = 1
SOURCES	+= $(wildcard $(SOCDIR)/camera/*.c)
CFLAGS += -I$(SOCDIR)/camera
VPATH  += $(SOCDIR)/camera
endif

ifeq ($(JPEG),1)
SOURCES	+= $(wildcard $(UCGUIDIR)/JPEG/*.c)
CFLAGS += -I$(UCGUIDIR)/JPEG
VPATH  += $(UCGUIDIR)/JPEG
endif

ifeq ($(UDC),1)
include $(UDCDIR)/udc.mk
endif

ifeq ($(MP3),1)
include $(wildcard $(MP3DIR)/mp3.mk)
endif

HEADS	+= $(SOCDIR)/head.S

OBJS	:= $(addsuffix .o , $(basename $(notdir $(SOURCES))))
HEADO	:= $(addsuffix .o , $(basename $(notdir $(HEADS))))


APP	:= loader.elf


all:	$(APP)
	$(OBJCOPY) -O binary $(APP) loader.bin
	$(OBJDUMP) -d $(APP) > loader.dump
	$(NM) $(APP) | sort > loader.sym
	$(OBJDUMP) -h $(APP) > loader.map
	cp loader.bin ../target/

$(APP):	$(HEADO) $(OBJS) $(EXTLIBS) link.xn makefile
	$(CC) -nostdlib -T link.xn -o $@ $(HEADO) $(OBJS) $(EXTLIBS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<
.cpp.o:
	$(CC) $(CFLAGS) -fno-rtti -fvtable-gc -o $@ -c $<
.S.o:
	$(CC) $(CFLAGS) -D_ASSEMBLER_ -D__ASSEMBLY__ -o $@ -c $<

clean:
	rm -fr *.o $(APP) $(OBJS) $(OTHER) *.sym *.map *.dump *.bin *.lib

ls:
	$(SOURCES)
