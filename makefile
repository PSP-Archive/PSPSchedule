TARGET = hello
OBJS = main.o  pg.o

CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = PSPSchedule

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak