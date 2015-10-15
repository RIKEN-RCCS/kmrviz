prefix ?= /usr/local

CC := gcc

SRC_H = kmrviz.h
SRC_C = kmrviz.c
SRC_ALL = $(SRC_H) $(SRC_C) control.c

cflags_1 ?= -Wall -Wextra -g

CFLAGS += $(cflags_1)
CFLAGS += -Wl,--export-dynamic

ldflags_1 ?=

LDFLAGS += `pkg-config --cflags --libs gtk+-3.0`
LDFLAGS += -lm
LDFLAGS += $(ldflags_1)

all : compile
download :
compile : compile_done
install : install_done

exe := kmrviz

compile_done : $(exe)
	touch $@

$(exe) : $(SRC_ALL)
	$(CC) $(CFLAGS) $(SRC_C) $(LDFLAGS) -o $@

install_done: compile_done
	mkdir -p $(prefix)/bin
	ln -sf $(realpath .)/$(exe) $(prefix)/bin/
	touch $@

clean :
	rm -f $(exe)

distclean:
	rm -rf *_done

