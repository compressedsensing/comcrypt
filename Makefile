CONTIKI_PROJECT = comcrypt
all: $(CONTIKI_PROJECT)

CONTIKI = ../..

PROJECT_SOURCEFILES += encrypt.c

include $(CONTIKI)/Makefile.include
