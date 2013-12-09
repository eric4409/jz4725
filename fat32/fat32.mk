SOURCES += $(wildcard $(FAT32DIR)/*.c)
CFLAGS	+= -I$(FAT32DIR)
VPATH   +=  $(FAT32DIR)

