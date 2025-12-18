#
# Makefile:
#	Copyright (c) 2025 Karinca Robotics

DESTDIR?=/usr
PREFIX?=/local

ifneq ($V,1)
Q ?= @
endif

#DEBUG	= -g -O0
DEBUG	= -O2
CC	= gcc
INCLUDE	= -I$(DESTDIR)$(PREFIX)/include
CFLAGS	= $(DEBUG) -Wall -Wextra $(INCLUDE) -Winline -pipe

LDFLAGS	= -L$(DESTDIR)$(PREFIX)/lib
LIBS    = -lwiringPi -lwiringPiDev -lpthread -lrt -lm -lcrypt

# May not need to  alter anything below this line
###############################################################################

SRC	=	gpio_daemon.c

OBJ	=	$(SRC:.c=.o)

all:		gpio_daemon

version.h:	../VERSION
	$Q echo Need to run newVersion above.

gpio_daemon:	$(OBJ)
	$Q echo [Link]
	$Q $(CC) -o $@ $(OBJ) $(LDFLAGS) $(LIBS)

.c.o:
	$Q echo [Compile] $<
	$Q $(CC) -c $(CFLAGS) $(EXTRA_CFLAGS) $< -o $@

.PHONY:	clean
clean:
	$Q echo "[Clean]"
	$Q rm -f $(OBJ) gpio_daemon *~ core tags *.bak

.PHONY:	tags
tags:	$(SRC)
	$Q echo [ctags]
	$Q ctags $(SRC)

.PHONY:	install
install: gpio_daemon
	$Q echo "[Install]"
	$Q cp gpio_daemon		$(DESTDIR)$(PREFIX)/bin/
ifneq ($(WIRINGPI_SUID),0)
	$Q chown root:root	$(DESTDIR)$(PREFIX)/bin/gpio_daemon
	$Q chmod 4755		$(DESTDIR)$(PREFIX)/bin/gpio_daemon
endif
	$Q mkdir -p		$(DESTDIR)$(PREFIX)/share/man/man1

.PHONY:	install-deb
install-deb:	gpio_daemon
	$Q echo "[Install: deb]"
	$Q install -m 0755 -d							$(CURDIR)/../debian-template/wiringPi/usr/bin
	$Q install -m 0755 gpio_daemon							$(CURDIR)/../debian-template/wiringPi/usr/bin
	$Q install -m 0755 -d							$(CURDIR)/../debian-template/wiringPi/usr/share/man/man1

.PHONY:	uninstall
uninstall:
	$Q echo "[UnInstall]"
	$Q rm -f $(DESTDIR)$(PREFIX)/bin/gpio_daemon

.PHONY:	depend
depend:
	makedepend -Y $(SRC)

# DO NOT DELETE

gpio_daemon.o: ../version.h
