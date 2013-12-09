
NAND_UDC_DISK=0
RAM_UDC_DISK = 1
SD_UDC_DISK = 0

SOURCES	+= $(wildcard $(UDCDIR)/udc_bus/*.c)
SOURCES	+= $(wildcard $(UDCDIR)/mass_storage/*.c)
SOURCES	+= $(wildcard $(UDCDIR)/udc_dma/*.c)
ifeq ($(RAM_UDC_DISK),1)
SOURCES	+= $(wildcard $(UDCDIR)/block/ramdisk/*.c)
CFLAGS	+= -DRAM_UDC_DISK=$(RAM_UDC_DISK)
CFLAGS	+= -I$(UDCDIR)/block/ramdisk
VPATH   += $(UDCDIR)/block/ramdisk
endif
ifeq ($(NAND_UDC_DISK),1)
SOURCES	+= $(wildcard $(UDCDIR)/block/nanddisk/*.c)
CFLAGS	+= -DNAND_UDC_DISK=$(NAND_UDC_DISK)
CFLAGS	+= -I$(UDCDIR)/block/nanddisk
VPATH   += $(UDCDIR)/block/nanddisk
endif
ifeq ($(SD_UDC_DISK),1)
SOURCES	+= $(wildcard $(UDCDIR)/block/sddisk/*.c)
CFLAGS	+= -DSD_UDC_DISK=$(SD_UDC_DISK)
CFLAGS	+= -I$(UDCDIR)/block/sddisk
VPATH   += $(UDCDIR)/block/sddisk
endif

CFLAGS	+= -DUDC=$(UDC)

CFLAGS	+= -I$(UDCDIR)/udc_bus
CFLAGS	+= -I$(UDCDIR)/udc_dma
CFLAGS	+= -I$(UDCDIR)/mass_storage

VPATH   += $(UDCDIR)/udc_bus
VPATH   += $(UDCDIR)/udc_dma
VPATH   += $(UDCDIR)/mass_storage
