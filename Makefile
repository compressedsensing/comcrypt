TARGET_LIBFILES += -lm
CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"
CONTIKI = ../..

MAKE_NET = MAKE_NET_NULLNET

CONTIKI_PROJECT = comcrypt_end_to_end
all: $(CONTIKI_PROJECT)

PROJECT_SOURCEFILES += encrypt.c compression.c fixedpoint.c

include $(CONTIKI)/Makefile.include
