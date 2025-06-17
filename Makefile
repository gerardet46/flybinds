# flybinds - dynamic menu one key to select
# See LICENSE file for copyright and license details.

include config.mk

SRC = drw.c flybinds.c util.c
OBJ = $(SRC:.c=.o)

all: options flybinds

options:
	@echo flybinds build options:
	@echo "CFLAGS   = $(CFLAGS)"
	@echo "LDFLAGS  = $(LDFLAGS)"
	@echo "CC       = $(CC)"

.c.o:
	$(CC) -c $(CFLAGS) $<

config.h:
	cp config.def.h $@

$(OBJ): config.h config.mk drw.h

flybinds: flybinds.o drw.o util.o
	$(CC) -o $@ flybinds.o drw.o util.o $(LDFLAGS)

clean:
	rm -f flybinds $(OBJ) flybinds-$(VERSION).tar.gz

dist: clean
	mkdir -p flybinds-$(VERSION)
	cp LICENSE Makefile README config.def.h config.mk flybinds.1\
		drw.h util.h $(SRC)\
		flybinds-$(VERSION)
	tar -cf flybinds-$(VERSION).tar flybinds-$(VERSION)
	gzip flybinds-$(VERSION).tar
	rm -rf flybinds-$(VERSION)

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f flybinds $(DESTDIR)$(PREFIX)/bin
	chmod 755 $(DESTDIR)$(PREFIX)/bin/flybinds
	mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	sed "s/VERSION/$(VERSION)/g" < flybinds.1 > $(DESTDIR)$(MANPREFIX)/man1/flybinds.1
	chmod 644 $(DESTDIR)$(MANPREFIX)/man1/flybinds.1

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/flybinds\
		$(DESTDIR)$(MANPREFIX)/man1/flybinds.1\

.PHONY: all options clean dist install uninstall
