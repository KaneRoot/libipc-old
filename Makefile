PACKAGE = 'perfect-os-junk'
VERSION = '0.0.1'

PREFIX := /usr/local
BINDIR := $(PREFIX)/bin
LIBDIR := $(PREFIX)/lib
SHAREDIR := $(PREFIX)/share
INCLUDEDIR := $(PREFIX)/include

CC := cc
AR := ar
RANLIB := ranlib
CFLAGS := -O2 -Wall -Wextra -Wshadow -ansi -pedantic -std=c99
LDFLAGS := 

Q := @

all: libposj init-connection open-read-close-fifo open-write-close-fifo service-test

libposj: libposj.so libposj.a
	@:
libposj.install: libposj.so.install libposj.a.install

libposj.clean: libposj.so.clean libposj.a.clean

libposj.uninstall: libposj.so.uninstall libposj.a.uninstall

init-connection: init-connection.o lib/communication.o libposj.a
	@echo '[01;32m  [LD]    [01;37minit-connection[00m'
	$(Q)$(CC) -o init-connection $(LDFLAGS) init-connection.o lib/communication.o libposj.a

init-connection.install: init-connection
	@echo '[01;31m  [IN]    [01;37m$(BINDIR)/init-connection[00m'
	$(Q)mkdir -p '$(DESTDIR)$(BINDIR)'
	$(Q)install -m0755 init-connection $(DESTDIR)$(BINDIR)/init-connection

init-connection.clean:  init-connection.o.clean lib/communication.o.clean
	@echo '[01;37m  [RM]    [01;37minit-connection[00m'
	$(Q)rm -f init-connection

init-connection.uninstall:
	@echo '[01;37m  [RM]    [01;37m$(BINDIR)/init-connection[00m'
	$(Q)rm -f '$(DESTDIR)$(BINDIR)/init-connection'

open-read-close-fifo: open-read-close-fifo.o lib/communication.o libposj.a
	@echo '[01;32m  [LD]    [01;37mopen-read-close-fifo[00m'
	$(Q)$(CC) -o open-read-close-fifo $(LDFLAGS) open-read-close-fifo.o lib/communication.o libposj.a

open-read-close-fifo.install: open-read-close-fifo
	@echo '[01;31m  [IN]    [01;37m$(BINDIR)/open-read-close-fifo[00m'
	$(Q)mkdir -p '$(DESTDIR)$(BINDIR)'
	$(Q)install -m0755 open-read-close-fifo $(DESTDIR)$(BINDIR)/open-read-close-fifo

open-read-close-fifo.clean:  open-read-close-fifo.o.clean lib/communication.o.clean
	@echo '[01;37m  [RM]    [01;37mopen-read-close-fifo[00m'
	$(Q)rm -f open-read-close-fifo

open-read-close-fifo.uninstall:
	@echo '[01;37m  [RM]    [01;37m$(BINDIR)/open-read-close-fifo[00m'
	$(Q)rm -f '$(DESTDIR)$(BINDIR)/open-read-close-fifo'

open-write-close-fifo: open-write-close-fifo.o lib/communication.o libposj.a
	@echo '[01;32m  [LD]    [01;37mopen-write-close-fifo[00m'
	$(Q)$(CC) -o open-write-close-fifo $(LDFLAGS) open-write-close-fifo.o lib/communication.o libposj.a

open-write-close-fifo.install: open-write-close-fifo
	@echo '[01;31m  [IN]    [01;37m$(BINDIR)/open-write-close-fifo[00m'
	$(Q)mkdir -p '$(DESTDIR)$(BINDIR)'
	$(Q)install -m0755 open-write-close-fifo $(DESTDIR)$(BINDIR)/open-write-close-fifo

open-write-close-fifo.clean:  open-write-close-fifo.o.clean lib/communication.o.clean
	@echo '[01;37m  [RM]    [01;37mopen-write-close-fifo[00m'
	$(Q)rm -f open-write-close-fifo

open-write-close-fifo.uninstall:
	@echo '[01;37m  [RM]    [01;37m$(BINDIR)/open-write-close-fifo[00m'
	$(Q)rm -f '$(DESTDIR)$(BINDIR)/open-write-close-fifo'

service-test: service-test.o lib/communication.o libposj.a
	@echo '[01;32m  [LD]    [01;37mservice-test[00m'
	$(Q)$(CC) -o service-test $(LDFLAGS) service-test.o lib/communication.o libposj.a

service-test.install: service-test
	@echo '[01;31m  [IN]    [01;37m$(BINDIR)/service-test[00m'
	$(Q)mkdir -p '$(DESTDIR)$(BINDIR)'
	$(Q)install -m0755 service-test $(DESTDIR)$(BINDIR)/service-test

service-test.clean:  service-test.o.clean lib/communication.o.clean
	@echo '[01;37m  [RM]    [01;37mservice-test[00m'
	$(Q)rm -f service-test

service-test.uninstall:
	@echo '[01;37m  [RM]    [01;37m$(BINDIR)/service-test[00m'
	$(Q)rm -f '$(DESTDIR)$(BINDIR)/service-test'

libposj.so: lib/communication.o 
	@echo '[01;32m  [LD]    [01;37mlibposj.so[00m'
	$(Q)$(CC) -o libposj.so -shared $(LDFLAGS) lib/communication.o 

libposj.so.install: libposj.so
	@echo '[01;31m  [IN]    [01;37m$(LIBDIR)/libposj.so.0.0.1[00m'
	$(Q)mkdir -p '$(DESTDIR)$(LIBDIR)'
	$(Q)install -m0755 libposj.so $(DESTDIR)$(LIBDIR)/libposj.so.0.0.1
	@echo '[01;33m  [LN]    [01;37m$(LIBDIR)/libposj.so.0.0[00m'
	$(Q)ln -sf '$(LIBDIR)/libposj.so.0.0.1' '$(DESTDIR)/$(LIBDIR)/libposj.so.0.0'
	@echo '[01;33m  [LN]    [01;37m$(LIBDIR)/libposj.so.0[00m'
	$(Q)ln -sf '$(LIBDIR)/libposj.so.0.0.1' '$(DESTDIR)/$(LIBDIR)/libposj.so.0'
	@echo '[01;33m  [LN]    [01;37m$(LIBDIR)/libposj.so[00m'
	$(Q)ln -sf '$(LIBDIR)/libposj.so.0.0.1' '$(DESTDIR)/$(LIBDIR)/libposj.so'

libposj.so.clean:
	@echo '[01;37m  [RM]    [01;37mlibposj.so[00m'
	$(Q)rm -f libposj.so

libposj.so.uninstall:
	@echo '[01;37m  [RM]    [01;37m$(LIBDIR)/libposj.so.0.0.1[00m'
	$(Q)rm -f '$(DESTDIR)$(LIBDIR)/libposj.so.0.0.1'
	@echo '[01;37m  [RM]    [01;37m$(LIBDIR)/libposj.so.0.0[00m'
	$(Q)rm -f '$(DESTDIR)$(LIBDIR)/libposj.so.0.0'
	@echo '[01;37m  [RM]    [01;37m$(LIBDIR)/libposj.so.0[00m'
	$(Q)rm -f '$(DESTDIR)$(LIBDIR)/libposj.so.0'
	@echo '[01;37m  [RM]    [01;37m$(LIBDIR)/libposj.so[00m'
	$(Q)rm -f '$(DESTDIR)$(LIBDIR)/libposj.so'

libposj.a: lib/communication.o 
	@echo '[01;32m  [LD]    [01;37mlibposj.a[00m'
	$(Q)$(AR) rc 'libposj.a' lib/communication.o

libposj.a.install: libposj.a
	@echo '[01;31m  [IN]    [01;37m$(LIBDIR)/libposj.a[00m'
	$(Q)mkdir -p '$(DESTDIR)$(LIBDIR)'
	$(Q)install -m0755 libposj.a $(DESTDIR)$(LIBDIR)/libposj.a

libposj.a.clean:
	@echo '[01;37m  [RM]    [01;37mlibposj.a[00m'
	$(Q)rm -f libposj.a

libposj.a.uninstall:
	@echo '[01;37m  [RM]    [01;37m$(LIBDIR)/libposj.a[00m'
	$(Q)rm -f '$(DESTDIR)$(LIBDIR)/libposj.a'

init-connection.o: init-connection.c ./lib/communication.h
	@echo '[01;34m  [CC]    [01;37minit-connection.o[00m'
	$(Q)$(CC) $(CFLAGS)  -c init-connection.c   -o init-connection.o

init-connection.o.install:

init-connection.o.clean:
	@echo '[01;37m  [RM]    [01;37minit-connection.o[00m'
	$(Q)rm -f init-connection.o

init-connection.o.uninstall:

lib/communication.o: lib/communication.c lib/communication.h
	@echo '[01;34m  [CC]    [01;37mlib/communication.o[00m'
	$(Q)$(CC) $(CFLAGS) -fPIC  -c lib/communication.c  -fPIC  -o lib/communication.o

lib/communication.o.install:

lib/communication.o.clean:
	@echo '[01;37m  [RM]    [01;37mlib/communication.o[00m'
	$(Q)rm -f lib/communication.o

lib/communication.o.uninstall:

open-read-close-fifo.o: open-read-close-fifo.c
	@echo '[01;34m  [CC]    [01;37mopen-read-close-fifo.o[00m'
	$(Q)$(CC) $(CFLAGS)  -c open-read-close-fifo.c   -o open-read-close-fifo.o

open-read-close-fifo.o.install:

open-read-close-fifo.o.clean:
	@echo '[01;37m  [RM]    [01;37mopen-read-close-fifo.o[00m'
	$(Q)rm -f open-read-close-fifo.o

open-read-close-fifo.o.uninstall:

open-write-close-fifo.o: open-write-close-fifo.c
	@echo '[01;34m  [CC]    [01;37mopen-write-close-fifo.o[00m'
	$(Q)$(CC) $(CFLAGS)  -c open-write-close-fifo.c   -o open-write-close-fifo.o

open-write-close-fifo.o.install:

open-write-close-fifo.o.clean:
	@echo '[01;37m  [RM]    [01;37mopen-write-close-fifo.o[00m'
	$(Q)rm -f open-write-close-fifo.o

open-write-close-fifo.o.uninstall:

service-test.o: service-test.c ./lib/communication.h
	@echo '[01;34m  [CC]    [01;37mservice-test.o[00m'
	$(Q)$(CC) $(CFLAGS)  -c service-test.c   -o service-test.o

service-test.o.install:

service-test.o.clean:
	@echo '[01;37m  [RM]    [01;37mservice-test.o[00m'
	$(Q)rm -f service-test.o

service-test.o.uninstall:

$(DESTDIR)$(PREFIX):
	@echo '[01;35m  [DIR]   [01;37m$(PREFIX)[00m'
	$(Q)mkdir -p $(DESTDIR)$(PREFIX)
$(DESTDIR)$(BINDIR):
	@echo '[01;35m  [DIR]   [01;37m$(BINDIR)[00m'
	$(Q)mkdir -p $(DESTDIR)$(BINDIR)
$(DESTDIR)$(LIBDIR):
	@echo '[01;35m  [DIR]   [01;37m$(LIBDIR)[00m'
	$(Q)mkdir -p $(DESTDIR)$(LIBDIR)
$(DESTDIR)$(SHAREDIR):
	@echo '[01;35m  [DIR]   [01;37m$(SHAREDIR)[00m'
	$(Q)mkdir -p $(DESTDIR)$(SHAREDIR)
$(DESTDIR)$(INCLUDEDIR):
	@echo '[01;35m  [DIR]   [01;37m$(INCLUDEDIR)[00m'
	$(Q)mkdir -p $(DESTDIR)$(INCLUDEDIR)
install: subdirs.install libposj.install init-connection.install open-read-close-fifo.install open-write-close-fifo.install service-test.install libposj.so.install libposj.a.install init-connection.o.install lib/communication.o.install open-read-close-fifo.o.install lib/communication.o.install open-write-close-fifo.o.install lib/communication.o.install service-test.o.install lib/communication.o.install lib/communication.o.install lib/communication.o.install
	@:

subdirs.install:

uninstall: subdirs.uninstall libposj.uninstall init-connection.uninstall open-read-close-fifo.uninstall open-write-close-fifo.uninstall service-test.uninstall libposj.so.uninstall libposj.a.uninstall init-connection.o.uninstall lib/communication.o.uninstall open-read-close-fifo.o.uninstall lib/communication.o.uninstall open-write-close-fifo.o.uninstall lib/communication.o.uninstall service-test.o.uninstall lib/communication.o.uninstall lib/communication.o.uninstall lib/communication.o.uninstall
	@:

subdirs.uninstall:

test: all subdirs subdirs.test
	@:

subdirs.test:

clean: libposj.clean init-connection.clean open-read-close-fifo.clean open-write-close-fifo.clean service-test.clean libposj.so.clean libposj.a.clean init-connection.o.clean lib/communication.o.clean open-read-close-fifo.o.clean lib/communication.o.clean open-write-close-fifo.o.clean lib/communication.o.clean service-test.o.clean lib/communication.o.clean lib/communication.o.clean lib/communication.o.clean

distclean: clean

dist: dist-gz dist-xz dist-bz2
	$(Q)rm -- $(PACKAGE)-$(VERSION)

distdir:
	$(Q)rm -rf -- $(PACKAGE)-$(VERSION)
	$(Q)ln -s -- . $(PACKAGE)-$(VERSION)

dist-gz: $(PACKAGE)-$(VERSION).tar.gz
$(PACKAGE)-$(VERSION).tar.gz: distdir
	@echo '[01;33m  [TAR]   [01;37m$(PACKAGE)-$(VERSION).tar.gz[00m'
	$(Q)tar czf $(PACKAGE)-$(VERSION).tar.gz \
		$(PACKAGE)-$(VERSION)/init-connection.c \
		$(PACKAGE)-$(VERSION)/service-test.c \
		$(PACKAGE)-$(VERSION)/open-write-close-fifo.c \
		$(PACKAGE)-$(VERSION)/open-read-close-fifo.c \
		$(PACKAGE)-$(VERSION)/lib/communication.c \
		$(PACKAGE)-$(VERSION)/libposj.a \
		$(PACKAGE)-$(VERSION)/lib/communication.h

dist-xz: $(PACKAGE)-$(VERSION).tar.xz
$(PACKAGE)-$(VERSION).tar.xz: distdir
	@echo '[01;33m  [TAR]   [01;37m$(PACKAGE)-$(VERSION).tar.xz[00m'
	$(Q)tar cJf $(PACKAGE)-$(VERSION).tar.xz \
		$(PACKAGE)-$(VERSION)/init-connection.c \
		$(PACKAGE)-$(VERSION)/service-test.c \
		$(PACKAGE)-$(VERSION)/open-write-close-fifo.c \
		$(PACKAGE)-$(VERSION)/open-read-close-fifo.c \
		$(PACKAGE)-$(VERSION)/lib/communication.c \
		$(PACKAGE)-$(VERSION)/libposj.a \
		$(PACKAGE)-$(VERSION)/lib/communication.h

dist-bz2: $(PACKAGE)-$(VERSION).tar.bz2
$(PACKAGE)-$(VERSION).tar.bz2: distdir
	@echo '[01;33m  [TAR]   [01;37m$(PACKAGE)-$(VERSION).tar.bz2[00m'
	$(Q)tar cjf $(PACKAGE)-$(VERSION).tar.bz2 \
		$(PACKAGE)-$(VERSION)/init-connection.c \
		$(PACKAGE)-$(VERSION)/service-test.c \
		$(PACKAGE)-$(VERSION)/open-write-close-fifo.c \
		$(PACKAGE)-$(VERSION)/open-read-close-fifo.c \
		$(PACKAGE)-$(VERSION)/lib/communication.c \
		$(PACKAGE)-$(VERSION)/libposj.a \
		$(PACKAGE)-$(VERSION)/lib/communication.h

help:
	@echo '[01;37m :: perfect-os-junk-0.0.1[00m'
	@echo ''
	@echo '[01;37mGeneric targets:[00m'
	@echo '[00m    - [01;32mhelp          [37mPrints this help message.[00m'
	@echo '[00m    - [01;32mall           [37mBuilds all targets.[00m'
	@echo '[00m    - [01;32mdist          [37mCreates tarballs of the files of the project.[00m'
	@echo '[00m    - [01;32minstall       [37mInstalls the project.[00m'
	@echo '[00m    - [01;32mclean         [37mRemoves compiled files.[00m'
	@echo '[00m    - [01;32muninstall     [37mDeinstalls the project.[00m'
	@echo ''
	@echo '[01;37mCLI-modifiable variables:[00m'
	@echo '    - [01;34mCC            [37m${CC}[00m'
	@echo '    - [01;34mCFLAGS        [37m${CFLAGS}[00m'
	@echo '    - [01;34mLDFLAGS       [37m${LDFLAGS}[00m'
	@echo '    - [01;34mDESTDIR       [37m${DESTDIR}[00m'
	@echo '    - [01;34mPREFIX        [37m${PREFIX}[00m'
	@echo '    - [01;34mBINDIR        [37m${BINDIR}[00m'
	@echo '    - [01;34mLIBDIR        [37m${LIBDIR}[00m'
	@echo '    - [01;34mSHAREDIR      [37m${SHAREDIR}[00m'
	@echo '    - [01;34mINCLUDEDIR    [37m${INCLUDEDIR}[00m'
	@echo ''
	@echo '[01;37mProject targets: [00m'
	@echo '    - [01;33mlibposj       [37mlibrary[00m'
	@echo '    - [01;33minit-connection[37mbinary[00m'
	@echo '    - [01;33mopen-read-close-fifo[37mbinary[00m'
	@echo '    - [01;33mopen-write-close-fifo[37mbinary[00m'
	@echo '    - [01;33mservice-test  [37mbinary[00m'
	@echo ''
	@echo '[01;37mMakefile options:[00m'
	@echo '    - gnu:          true'
	@echo '    - colors:       true'
	@echo ''
	@echo '[01;37mRebuild the Makefile with:[00m'
	@echo '    zsh ./build.zsh -c -g'
.PHONY: all subdirs clean distclean dist install uninstall help

