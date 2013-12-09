SOURCES	+= $(NAND512DIR)/jz_nand.c \
#           $(NANDDIR)/lb_nand.c
					 
#DRVOBJ += $(NANDDIR)/ssfdc.o
#OEMREALSE += $(NANDDIR)/ssfdc.c
CFLAGS += -I$(NAND512DIR)
CFLAGS += -DNAND512=$(NAND512)
VPATH  += $(NAND512DIR)
