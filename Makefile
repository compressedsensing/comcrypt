TARGET_LIBFILES += -lm

CONTIKI_PROJECT = comcrypt
all: $(CONTIKI_PROJECT)

CONTIKI = ../..

PROJECT_SOURCEFILES += encrypt.c compression.c

include $(CONTIKI)/Makefile.include
