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
CFLAGS := -O2 -Wall -Wextra -Wshadow -ansi -pedantic -std=c99 -D_XOPEN_SOURCE=500 -D_POSIX_C_SOURCE=199309L
LDFLAGS := 

Q := @

all: libposj pingpong/pingpong pubsub/pubsub pubsub/pubsub-test-send

libposj: libposj.so libposj.a
	@:
libposj.install: libposj.so.install libposj.a.install

libposj.clean: libposj.so.clean libposj.a.clean

libposj.uninstall: libposj.so.uninstall libposj.a.uninstall

pingpong/pingpong: pingpong/pingpong.o libposj.a
	@echo '[01;32m  [LD]    [01;37mpingpong/pingpong[00m'
	$(Q)$(CC) -o pingpong/pingpong $(LDFLAGS) pingpong/pingpong.o libposj.a -lpthread

pingpong/pingpong.install: pingpong/pingpong
	@echo '[01;31m  [IN]    [01;37m$(BINDIR)/pingpong[00m'
	$(Q)mkdir -p '$(DESTDIR)$(BINDIR)'
	$(Q)install -m0755 pingpong/pingpong $(DESTDIR)$(BINDIR)/pingpong

pingpong/pingpong.clean:  pingpong/pingpong.o.clean
	@echo '[01;37m  [RM]    [01;37mpingpong/pingpong[00m'
	$(Q)rm -f pingpong/pingpong

pingpong/pingpong.uninstall:
	@echo '[01;37m  [RM]    [01;37m$(BINDIR)/pingpong[00m'
	$(Q)rm -f '$(DESTDIR)$(BINDIR)/pingpong'

pubsub/pubsub: pubsub/pubsubd.o libposj.a
	@echo '[01;32m  [LD]    [01;37mpubsub/pubsub[00m'
	$(Q)$(CC) -o pubsub/pubsub $(LDFLAGS) pubsub/pubsubd.o libposj.a -lpthread

pubsub/pubsub.install: pubsub/pubsub
	@echo '[01;31m  [IN]    [01;37m$(BINDIR)/pubsub[00m'
	$(Q)mkdir -p '$(DESTDIR)$(BINDIR)'
	$(Q)install -m0755 pubsub/pubsub $(DESTDIR)$(BINDIR)/pubsub

pubsub/pubsub.clean:  pubsub/pubsubd.o.clean
	@echo '[01;37m  [RM]    [01;37mpubsub/pubsub[00m'
	$(Q)rm -f pubsub/pubsub

pubsub/pubsub.uninstall:
	@echo '[01;37m  [RM]    [01;37m$(BINDIR)/pubsub[00m'
	$(Q)rm -f '$(DESTDIR)$(BINDIR)/pubsub'

pubsub/pubsub-test-send: pubsub/pubsub-test-send.o libposj.a
	@echo '[01;32m  [LD]    [01;37mpubsub/pubsub-test-send[00m'
	$(Q)$(CC) -o pubsub/pubsub-test-send $(LDFLAGS) pubsub/pubsub-test-send.o libposj.a -lpthread

pubsub/pubsub-test-send.install: pubsub/pubsub-test-send
	@echo '[01;31m  [IN]    [01;37m$(BINDIR)/pubsub-test-send[00m'
	$(Q)mkdir -p '$(DESTDIR)$(BINDIR)'
	$(Q)install -m0755 pubsub/pubsub-test-send $(DESTDIR)$(BINDIR)/pubsub-test-send

pubsub/pubsub-test-send.clean:  pubsub/pubsub-test-send.o.clean
	@echo '[01;37m  [RM]    [01;37mpubsub/pubsub-test-send[00m'
	$(Q)rm -f pubsub/pubsub-test-send

pubsub/pubsub-test-send.uninstall:
	@echo '[01;37m  [RM]    [01;37m$(BINDIR)/pubsub-test-send[00m'
	$(Q)rm -f '$(DESTDIR)$(BINDIR)/pubsub-test-send'

libposj.so: lib/communication.o lib/process.o lib/pubsubd.o 
	@echo '[01;32m  [LD]    [01;37mlibposj.so[00m'
	$(Q)$(CC) -o libposj.so -shared $(LDFLAGS) lib/communication.o lib/process.o lib/pubsubd.o 

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

libposj.a: lib/communication.o lib/process.o lib/pubsubd.o 
	@echo '[01;32m  [LD]    [01;37mlibposj.a[00m'
	$(Q)$(AR) rc 'libposj.a' lib/communication.o lib/process.o lib/pubsubd.o

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

pingpong/pingpong.o: pingpong/pingpong.c pingpong/../lib/communication.h
	@echo '[01;34m  [CC]    [01;37mpingpong/pingpong.o[00m'
	$(Q)$(CC) $(CFLAGS)  -c pingpong/pingpong.c   -o pingpong/pingpong.o

pingpong/pingpong.o.install:

pingpong/pingpong.o.clean:
	@echo '[01;37m  [RM]    [01;37mpingpong/pingpong.o[00m'
	$(Q)rm -f pingpong/pingpong.o

pingpong/pingpong.o.uninstall:

pubsub/pubsubd.o: pubsub/pubsubd.c pubsub/../lib/pubsubd.h
	@echo '[01;34m  [CC]    [01;37mpubsub/pubsubd.o[00m'
	$(Q)$(CC) $(CFLAGS)  -c pubsub/pubsubd.c   -o pubsub/pubsubd.o

pubsub/pubsubd.o.install:

pubsub/pubsubd.o.clean:
	@echo '[01;37m  [RM]    [01;37mpubsub/pubsubd.o[00m'
	$(Q)rm -f pubsub/pubsubd.o

pubsub/pubsubd.o.uninstall:

pubsub/pubsub-test-send.o: pubsub/pubsub-test-send.c pubsub/../lib/pubsubd.h
	@echo '[01;34m  [CC]    [01;37mpubsub/pubsub-test-send.o[00m'
	$(Q)$(CC) $(CFLAGS)  -c pubsub/pubsub-test-send.c   -o pubsub/pubsub-test-send.o

pubsub/pubsub-test-send.o.install:

pubsub/pubsub-test-send.o.clean:
	@echo '[01;37m  [RM]    [01;37mpubsub/pubsub-test-send.o[00m'
	$(Q)rm -f pubsub/pubsub-test-send.o

pubsub/pubsub-test-send.o.uninstall:

lib/communication.o: lib/communication.c lib/communication.h
	@echo '[01;34m  [CC]    [01;37mlib/communication.o[00m'
	$(Q)$(CC) $(CFLAGS) -fPIC  -c lib/communication.c  -fPIC  -o lib/communication.o

lib/communication.o.install:

lib/communication.o.clean:
	@echo '[01;37m  [RM]    [01;37mlib/communication.o[00m'
	$(Q)rm -f lib/communication.o

lib/communication.o.uninstall:

lib/process.o: lib/process.c lib/process.h
	@echo '[01;34m  [CC]    [01;37mlib/process.o[00m'
	$(Q)$(CC) $(CFLAGS) -fPIC  -c lib/process.c  -fPIC  -o lib/process.o

lib/process.o.install:

lib/process.o.clean:
	@echo '[01;37m  [RM]    [01;37mlib/process.o[00m'
	$(Q)rm -f lib/process.o

lib/process.o.uninstall:

lib/pubsubd.o: lib/pubsubd.c lib/pubsubd.h
	@echo '[01;34m  [CC]    [01;37mlib/pubsubd.o[00m'
	$(Q)$(CC) $(CFLAGS) -fPIC  -c lib/pubsubd.c  -fPIC  -o lib/pubsubd.o

lib/pubsubd.o.install:

lib/pubsubd.o.clean:
	@echo '[01;37m  [RM]    [01;37mlib/pubsubd.o[00m'
	$(Q)rm -f lib/pubsubd.o

lib/pubsubd.o.uninstall:

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
install: subdirs.install libposj.install pingpong/pingpong.install pubsub/pubsub.install pubsub/pubsub-test-send.install libposj.so.install libposj.a.install pingpong/pingpong.o.install pubsub/pubsubd.o.install pubsub/pubsub-test-send.o.install lib/communication.o.install lib/process.o.install lib/pubsubd.o.install lib/communication.o.install lib/process.o.install lib/pubsubd.o.install
	@:

subdirs.install:

uninstall: subdirs.uninstall libposj.uninstall pingpong/pingpong.uninstall pubsub/pubsub.uninstall pubsub/pubsub-test-send.uninstall libposj.so.uninstall libposj.a.uninstall pingpong/pingpong.o.uninstall pubsub/pubsubd.o.uninstall pubsub/pubsub-test-send.o.uninstall lib/communication.o.uninstall lib/process.o.uninstall lib/pubsubd.o.uninstall lib/communication.o.uninstall lib/process.o.uninstall lib/pubsubd.o.uninstall
	@:

subdirs.uninstall:

test: all subdirs subdirs.test
	@:

subdirs.test:

clean: libposj.clean pingpong/pingpong.clean pubsub/pubsub.clean pubsub/pubsub-test-send.clean libposj.so.clean libposj.a.clean pingpong/pingpong.o.clean pubsub/pubsubd.o.clean pubsub/pubsub-test-send.o.clean lib/communication.o.clean lib/process.o.clean lib/pubsubd.o.clean lib/communication.o.clean lib/process.o.clean lib/pubsubd.o.clean

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
		$(PACKAGE)-$(VERSION)/lib/communication.c \
		$(PACKAGE)-$(VERSION)/lib/process.c \
		$(PACKAGE)-$(VERSION)/lib/pubsubd.c \
		$(PACKAGE)-$(VERSION)/pingpong/pingpong.c \
		$(PACKAGE)-$(VERSION)/pubsub/pubsub-test-send.c \
		$(PACKAGE)-$(VERSION)/pubsub/pubsubd.c \
		$(PACKAGE)-$(VERSION)/libposj.a \
		$(PACKAGE)-$(VERSION)/lib/communication.h \
		$(PACKAGE)-$(VERSION)/lib/process.h \
		$(PACKAGE)-$(VERSION)/lib/pubsubd.h

dist-xz: $(PACKAGE)-$(VERSION).tar.xz
$(PACKAGE)-$(VERSION).tar.xz: distdir
	@echo '[01;33m  [TAR]   [01;37m$(PACKAGE)-$(VERSION).tar.xz[00m'
	$(Q)tar cJf $(PACKAGE)-$(VERSION).tar.xz \
		$(PACKAGE)-$(VERSION)/lib/communication.c \
		$(PACKAGE)-$(VERSION)/lib/process.c \
		$(PACKAGE)-$(VERSION)/lib/pubsubd.c \
		$(PACKAGE)-$(VERSION)/pingpong/pingpong.c \
		$(PACKAGE)-$(VERSION)/pubsub/pubsub-test-send.c \
		$(PACKAGE)-$(VERSION)/pubsub/pubsubd.c \
		$(PACKAGE)-$(VERSION)/libposj.a \
		$(PACKAGE)-$(VERSION)/lib/communication.h \
		$(PACKAGE)-$(VERSION)/lib/process.h \
		$(PACKAGE)-$(VERSION)/lib/pubsubd.h

dist-bz2: $(PACKAGE)-$(VERSION).tar.bz2
$(PACKAGE)-$(VERSION).tar.bz2: distdir
	@echo '[01;33m  [TAR]   [01;37m$(PACKAGE)-$(VERSION).tar.bz2[00m'
	$(Q)tar cjf $(PACKAGE)-$(VERSION).tar.bz2 \
		$(PACKAGE)-$(VERSION)/lib/communication.c \
		$(PACKAGE)-$(VERSION)/lib/process.c \
		$(PACKAGE)-$(VERSION)/lib/pubsubd.c \
		$(PACKAGE)-$(VERSION)/pingpong/pingpong.c \
		$(PACKAGE)-$(VERSION)/pubsub/pubsub-test-send.c \
		$(PACKAGE)-$(VERSION)/pubsub/pubsubd.c \
		$(PACKAGE)-$(VERSION)/libposj.a \
		$(PACKAGE)-$(VERSION)/lib/communication.h \
		$(PACKAGE)-$(VERSION)/lib/process.h \
		$(PACKAGE)-$(VERSION)/lib/pubsubd.h

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
	@echo '    - [01;33mpingpong/pingpong[37mbinary[00m'
	@echo '    - [01;33mpubsub/pubsub [37mbinary[00m'
	@echo '    - [01;33mpubsub/pubsub-test-send[37mbinary[00m'
	@echo ''
	@echo '[01;37mMakefile options:[00m'
	@echo '    - gnu:          true'
	@echo '    - colors:       true'
	@echo ''
	@echo '[01;37mRebuild the Makefile with:[00m'
	@echo '    zsh ./build.zsh -c -g'
.PHONY: all subdirs clean distclean dist install uninstall help

