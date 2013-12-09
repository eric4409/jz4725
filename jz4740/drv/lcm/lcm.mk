
SOURCES	+= $(wildcard $(LCMDIR)/*.c)
CFLAGS	+= -DLCM=$(LCM)
CFLAGS	+= -I$(LCMDIR)
VPATH   += $(LCMDIR)

