TARGET_LIBFILES += -lm

CONTIKI_PROJECT = comcrypt_end_to_end
all: $(CONTIKI_PROJECT)

CONTIKI = ../..
CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

PROJECT_SOURCEFILES += encrypt.c compression.c fixedpoint.c

include $(CONTIKI)/Makefile.include
