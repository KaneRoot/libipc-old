PACKAGE = 'ipc'
VERSION = '0.1.0'

PREFIX := /usr/local
BINDIR := $(PREFIX)/bin
LIBDIR := $(PREFIX)/lib
SHAREDIR := $(PREFIX)/share
INCLUDEDIR := $(PREFIX)/include
MANDIR := $(SHAREDIR)/man

CC := cc
AR := ar
RANLIB := ranlib
CFLAGS :=
LDFLAGS :=

Q := @

all: libipc src/ipc.h man/libipc.7
	@:

libipc: libipc.so libipc.a
	@:
libipc.install: libipc.so.install libipc.a.install

libipc.clean: libipc.so.clean libipc.a.clean

libipc.uninstall: libipc.so.uninstall libipc.a.uninstall

src/ipc.h.install: src/ipc.h
	@echo '  IN >    $(INCLUDEDIR)/ipc.h'
	$(Q)mkdir -p '$(DESTDIR)$(INCLUDEDIR)'
	$(Q)install -m0644 src/ipc.h $(DESTDIR)$(INCLUDEDIR)/ipc.h

src/ipc.h.clean: src/ipc.h
	$(Q):

src/ipc.h.uninstall:
	@echo '  RM >    $(INCLUDEDIR)/ipc.h'
	$(Q)rm -f '$(DESTDIR)$(INCLUDEDIR)/ipc.h'

man/libipc.7: man/libipc.7.md
	@echo '  MAN >   man/libipc.7'
	$(Q)pandoc -s --from markdown --to man 'man/libipc.7.md' -o 'man/libipc.7'


man/libipc.7.install: man/libipc.7
	@echo '  IN >    $(MANDIR)/man7/libipc.7'
	$(Q)mkdir -p '$(DESTDIR)$(MANDIR)/man7'
	$(Q)install -m0644 man/libipc.7 $(DESTDIR)$(MANDIR)/man7/libipc.7

man/libipc.7.clean:
	@echo '  RM >    man/libipc.7'
	$(Q)rm -f man/libipc.7

man/libipc.7.uninstall:
	@echo '  RM >    $(MANDIR)/man7/libipc.7'
	$(Q)rm -f '$(DESTDIR)$(MANDIR)/man7/libipc.7'

libipc.so: src/communication.o src/error.o src/logger.o src/message.o src/usocket.o src/utils.o 
	@echo '  LD >    libipc.so'
	$(Q)$(CC) -o libipc.so -shared $(LDFLAGS) src/communication.o src/error.o src/logger.o src/message.o src/usocket.o src/utils.o 

libipc.so.install: libipc.so
	@echo '  IN >    $(LIBDIR)/libipc.so.0.1.0'
	$(Q)mkdir -p '$(DESTDIR)$(LIBDIR)'
	$(Q)install -m0755 libipc.so $(DESTDIR)$(LIBDIR)/libipc.so.0.1.0
	@echo '  LN >    $(LIBDIR)/libipc.so.0.1'
	$(Q)ln -sf '$(LIBDIR)/libipc.so.0.1.0' '$(DESTDIR)/$(LIBDIR)/libipc.so.0.1'
	@echo '  LN >    $(LIBDIR)/libipc.so.0'
	$(Q)ln -sf '$(LIBDIR)/libipc.so.0.1.0' '$(DESTDIR)/$(LIBDIR)/libipc.so.0'
	@echo '  LN >    $(LIBDIR)/libipc.so'
	$(Q)ln -sf '$(LIBDIR)/libipc.so.0.1.0' '$(DESTDIR)/$(LIBDIR)/libipc.so'

libipc.so.clean:
	@echo '  RM >    libipc.so'
	$(Q)rm -f libipc.so

libipc.so.uninstall:
	@echo '  RM >    $(LIBDIR)/libipc.so.0.1.0'
	$(Q)rm -f '$(DESTDIR)$(LIBDIR)/libipc.so.0.1.0'
	@echo '  RM >    $(LIBDIR)/libipc.so.0.1'
	$(Q)rm -f '$(DESTDIR)$(LIBDIR)/libipc.so.0.1'
	@echo '  RM >    $(LIBDIR)/libipc.so.0'
	$(Q)rm -f '$(DESTDIR)$(LIBDIR)/libipc.so.0'
	@echo '  RM >    $(LIBDIR)/libipc.so'
	$(Q)rm -f '$(DESTDIR)$(LIBDIR)/libipc.so'

libipc.a: src/communication.o src/error.o src/logger.o src/message.o src/usocket.o src/utils.o 
	@echo '  LD >    libipc.a'
	$(Q)$(AR) rc 'libipc.a' src/communication.o src/error.o src/logger.o src/message.o src/usocket.o src/utils.o

libipc.a.install: libipc.a
	@echo '  IN >    $(LIBDIR)/libipc.a'
	$(Q)mkdir -p '$(DESTDIR)$(LIBDIR)'
	$(Q)install -m0755 libipc.a $(DESTDIR)$(LIBDIR)/libipc.a

libipc.a.clean:
	@echo '  RM >    libipc.a'
	$(Q)rm -f libipc.a

libipc.a.uninstall:
	@echo '  RM >    $(LIBDIR)/libipc.a'
	$(Q)rm -f '$(DESTDIR)$(LIBDIR)/libipc.a'

src/communication.o: src/communication.c src/ipc.h src/utils.h src/message.h
	@echo '  CC >    src/communication.o'
	$(Q)$(CC) $(CFLAGS) -fPIC  -c src/communication.c  -fPIC  -o src/communication.o

src/communication.o.install:

src/communication.o.clean:
	@echo '  RM >    src/communication.o'
	$(Q)rm -f src/communication.o

src/communication.o.uninstall:

src/error.o: src/error.c src/ipc.h
	@echo '  CC >    src/error.o'
	$(Q)$(CC) $(CFLAGS) -fPIC  -c src/error.c  -fPIC  -o src/error.o

src/error.o.install:

src/error.o.clean:
	@echo '  RM >    src/error.o'
	$(Q)rm -f src/error.o

src/error.o.uninstall:

src/logger.o: src/logger.c src/logger.h
	@echo '  CC >    src/logger.o'
	$(Q)$(CC) $(CFLAGS) -fPIC  -c src/logger.c  -fPIC  -o src/logger.o

src/logger.o.install:

src/logger.o.clean:
	@echo '  RM >    src/logger.o'
	$(Q)rm -f src/logger.o

src/logger.o.uninstall:

src/message.o: src/message.c src/message.h src/usocket.h
	@echo '  CC >    src/message.o'
	$(Q)$(CC) $(CFLAGS) -fPIC  -c src/message.c  -fPIC  -o src/message.o

src/message.o.install:

src/message.o.clean:
	@echo '  RM >    src/message.o'
	$(Q)rm -f src/message.o

src/message.o.uninstall:

src/usocket.o: src/usocket.c src/usocket.h src/utils.h
	@echo '  CC >    src/usocket.o'
	$(Q)$(CC) $(CFLAGS) -fPIC  -c src/usocket.c  -fPIC  -o src/usocket.o

src/usocket.o.install:

src/usocket.o.clean:
	@echo '  RM >    src/usocket.o'
	$(Q)rm -f src/usocket.o

src/usocket.o.uninstall:

src/utils.o: src/utils.c src/utils.h
	@echo '  CC >    src/utils.o'
	$(Q)$(CC) $(CFLAGS) -fPIC  -c src/utils.c  -fPIC  -o src/utils.o

src/utils.o.install:

src/utils.o.clean:
	@echo '  RM >    src/utils.o'
	$(Q)rm -f src/utils.o

src/utils.o.uninstall:

$(DESTDIR)$(PREFIX):
	@echo '  DIR >   $(PREFIX)'
	$(Q)mkdir -p $(DESTDIR)$(PREFIX)
$(DESTDIR)$(BINDIR):
	@echo '  DIR >   $(BINDIR)'
	$(Q)mkdir -p $(DESTDIR)$(BINDIR)
$(DESTDIR)$(LIBDIR):
	@echo '  DIR >   $(LIBDIR)'
	$(Q)mkdir -p $(DESTDIR)$(LIBDIR)
$(DESTDIR)$(SHAREDIR):
	@echo '  DIR >   $(SHAREDIR)'
	$(Q)mkdir -p $(DESTDIR)$(SHAREDIR)
$(DESTDIR)$(INCLUDEDIR):
	@echo '  DIR >   $(INCLUDEDIR)'
	$(Q)mkdir -p $(DESTDIR)$(INCLUDEDIR)
$(DESTDIR)$(MANDIR):
	@echo '  DIR >   $(MANDIR)'
	$(Q)mkdir -p $(DESTDIR)$(MANDIR)
install: subdirs.install libipc.install src/ipc.h.install man/libipc.7.install libipc.so.install libipc.a.install src/communication.o.install src/error.o.install src/logger.o.install src/message.o.install src/usocket.o.install src/utils.o.install src/communication.o.install src/error.o.install src/logger.o.install src/message.o.install src/usocket.o.install src/utils.o.install
	@:

subdirs.install:

uninstall: subdirs.uninstall libipc.uninstall src/ipc.h.uninstall man/libipc.7.uninstall libipc.so.uninstall libipc.a.uninstall src/communication.o.uninstall src/error.o.uninstall src/logger.o.uninstall src/message.o.uninstall src/usocket.o.uninstall src/utils.o.uninstall src/communication.o.uninstall src/error.o.uninstall src/logger.o.uninstall src/message.o.uninstall src/usocket.o.uninstall src/utils.o.uninstall
	@:

subdirs.uninstall:

test: all subdirs subdirs.test
	@:

subdirs.test:

clean: libipc.clean src/ipc.h.clean man/libipc.7.clean libipc.so.clean libipc.a.clean src/communication.o.clean src/error.o.clean src/logger.o.clean src/message.o.clean src/usocket.o.clean src/utils.o.clean src/communication.o.clean src/error.o.clean src/logger.o.clean src/message.o.clean src/usocket.o.clean src/utils.o.clean

distclean: clean

dist: dist-gz dist-xz dist-bz2
	$(Q)rm -- $(PACKAGE)-$(VERSION)

distdir:
	$(Q)rm -rf -- $(PACKAGE)-$(VERSION)
	$(Q)ln -s -- . $(PACKAGE)-$(VERSION)

dist-gz: $(PACKAGE)-$(VERSION).tar.gz
$(PACKAGE)-$(VERSION).tar.gz: distdir
	@echo '  TAR >   $(PACKAGE)-$(VERSION).tar.gz'
	$(Q)tar czf $(PACKAGE)-$(VERSION).tar.gz \
		$(PACKAGE)-$(VERSION)/Makefile \
		$(PACKAGE)-$(VERSION)/project.zsh \
		$(PACKAGE)-$(VERSION)/src/ipc.h \
		$(PACKAGE)-$(VERSION)/man/libipc.7.md \
		$(PACKAGE)-$(VERSION)/src/communication.c \
		$(PACKAGE)-$(VERSION)/src/error.c \
		$(PACKAGE)-$(VERSION)/src/logger.c \
		$(PACKAGE)-$(VERSION)/src/message.c \
		$(PACKAGE)-$(VERSION)/src/usocket.c \
		$(PACKAGE)-$(VERSION)/src/utils.c \
		$(PACKAGE)-$(VERSION)/src/communication.h \
		$(PACKAGE)-$(VERSION)/src/logger.h \
		$(PACKAGE)-$(VERSION)/src/message.h \
		$(PACKAGE)-$(VERSION)/src/usocket.h \
		$(PACKAGE)-$(VERSION)/src/utils.h

dist-xz: $(PACKAGE)-$(VERSION).tar.xz
$(PACKAGE)-$(VERSION).tar.xz: distdir
	@echo '  TAR >   $(PACKAGE)-$(VERSION).tar.xz'
	$(Q)tar cJf $(PACKAGE)-$(VERSION).tar.xz \
		$(PACKAGE)-$(VERSION)/Makefile \
		$(PACKAGE)-$(VERSION)/project.zsh \
		$(PACKAGE)-$(VERSION)/src/ipc.h \
		$(PACKAGE)-$(VERSION)/man/libipc.7.md \
		$(PACKAGE)-$(VERSION)/src/communication.c \
		$(PACKAGE)-$(VERSION)/src/error.c \
		$(PACKAGE)-$(VERSION)/src/logger.c \
		$(PACKAGE)-$(VERSION)/src/message.c \
		$(PACKAGE)-$(VERSION)/src/usocket.c \
		$(PACKAGE)-$(VERSION)/src/utils.c \
		$(PACKAGE)-$(VERSION)/src/communication.h \
		$(PACKAGE)-$(VERSION)/src/logger.h \
		$(PACKAGE)-$(VERSION)/src/message.h \
		$(PACKAGE)-$(VERSION)/src/usocket.h \
		$(PACKAGE)-$(VERSION)/src/utils.h

dist-bz2: $(PACKAGE)-$(VERSION).tar.bz2
$(PACKAGE)-$(VERSION).tar.bz2: distdir
	@echo '  TAR >   $(PACKAGE)-$(VERSION).tar.bz2'
	$(Q)tar cjf $(PACKAGE)-$(VERSION).tar.bz2 \
		$(PACKAGE)-$(VERSION)/Makefile \
		$(PACKAGE)-$(VERSION)/project.zsh \
		$(PACKAGE)-$(VERSION)/src/ipc.h \
		$(PACKAGE)-$(VERSION)/man/libipc.7.md \
		$(PACKAGE)-$(VERSION)/src/communication.c \
		$(PACKAGE)-$(VERSION)/src/error.c \
		$(PACKAGE)-$(VERSION)/src/logger.c \
		$(PACKAGE)-$(VERSION)/src/message.c \
		$(PACKAGE)-$(VERSION)/src/usocket.c \
		$(PACKAGE)-$(VERSION)/src/utils.c \
		$(PACKAGE)-$(VERSION)/src/communication.h \
		$(PACKAGE)-$(VERSION)/src/logger.h \
		$(PACKAGE)-$(VERSION)/src/message.h \
		$(PACKAGE)-$(VERSION)/src/usocket.h \
		$(PACKAGE)-$(VERSION)/src/utils.h

help:
	@echo ' :: ipc-0.1.0'
	@echo ''
	@echo 'Generic targets:'
	@echo '    - help           Prints this help message.'
	@echo '    - all            Builds all targets.'
	@echo '    - dist           Creates tarballs of the files of the project.'
	@echo '    - install        Installs the project.'
	@echo '    - clean          Removes compiled files.'
	@echo '    - uninstall      Deinstalls the project.'
	@echo ''
	@echo 'CLI-modifiable variables:'
	@echo '    - CC             ${CC}'
	@echo '    - CFLAGS         ${CFLAGS}'
	@echo '    - LDFLAGS        ${LDFLAGS}'
	@echo '    - DESTDIR        ${DESTDIR}'
	@echo '    - PREFIX         ${PREFIX}'
	@echo '    - BINDIR         ${BINDIR}'
	@echo '    - LIBDIR         ${LIBDIR}'
	@echo '    - SHAREDIR       ${SHAREDIR}'
	@echo '    - INCLUDEDIR     ${INCLUDEDIR}'
	@echo '    - MANDIR         ${MANDIR}'
	@echo ''
	@echo 'Project targets: '
	@echo '    - libipc         library'
	@echo '    - src/ipc.h      header'
	@echo '    - man/libipc.7   man'
	@echo ''
	@echo 'Makefile options:'
	@echo '    - gnu:           false'
	@echo '    - colors:        false'
	@echo ''
	@echo 'Rebuild the Makefile with:'
	@echo '    zsh ./build.zsh'
.PHONY: all subdirs clean distclean dist install uninstall help

