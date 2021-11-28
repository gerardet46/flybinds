# flybinds - dynamic menu one key to select
# See LICENSE file for copyright and license details.

include config.mk

SRC = drw.c flybinds.c stest.c util.c
OBJ = $(SRC:.c=.o)

all: options flybinds stest

options:
	@echo flybinds build options:
	@echo "CFLAGS   = $(CFLAGS)"
	@echo "LDFLAGS  = $(LDFLAGS)"
	@echo "CC       = $(CC)"

.c.o:
	$(CC) -c $(CFLAGS) $<

config.h:
	cp config.def.h $@

$(OBJ): arg.h config.h config.mk drw.h

flybinds: flybinds.o drw.o util.o
	$(CC) -o $@ flybinds.o drw.o util.o $(LDFLAGS)

stest: stest.o
	$(CC) -o $@ stest.o $(LDFLAGS)

clean:
	rm -f flybinds stest $(OBJ) flybinds-$(VERSION).tar.gz

dist: clean
	mkdir -p flybinds-$(VERSION)
	cp LICENSE Makefile README arg.h config.def.h config.mk flybinds.1\
		drw.h util.h stest.1 $(SRC)\
		flybinds-$(VERSION)
	tar -cf flybinds-$(VERSION).tar flybinds-$(VERSION)
	gzip flybinds-$(VERSION).tar
	rm -rf flybinds-$(VERSION)

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f flybinds stest $(DESTDIR)$(PREFIX)/bin
	chmod 755 $(DESTDIR)$(PREFIX)/bin/flybinds
	chmod 755 $(DESTDIR)$(PREFIX)/bin/stest
	mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	sed "s/VERSION/$(VERSION)/g" < flybinds.1 > $(DESTDIR)$(MANPREFIX)/man1/flybinds.1
	sed "s/VERSION/$(VERSION)/g" < stest.1 > $(DESTDIR)$(MANPREFIX)/man1/stest.1
	chmod 644 $(DESTDIR)$(MANPREFIX)/man1/flybinds.1
	chmod 644 $(DESTDIR)$(MANPREFIX)/man1/stest.1

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/flybinds\
		$(DESTDIR)$(PREFIX)/bin/stest\
		$(DESTDIR)$(MANPREFIX)/man1/flybinds.1\
		$(DESTDIR)$(MANPREFIX)/man1/stest.1

.PHONY: all options clean dist install uninstall
